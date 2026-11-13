/* Grid A* router core for gen_pcb.py / route.py (called through ctypes).
 *
 * occ[l*H*W + y*W + x]:  0 free, -1 blocked, >0 net id of the copper there.
 * A cell is usable by net n on layer l when no cell within radius R
 * (precomputed disk offsets) holds copper of another net or a blockage.
 * Layer "plane" (index plane_layer) is the GND plane: only net plane_net may
 * enter it, every cell of it is a target, and nothing is stamped there.
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct { float f; int32_t idx; } node_t;

static node_t *heap; static int hn, hcap;
static void hpush(float f, int32_t idx) {
    if (hn == hcap) { hcap = hcap ? hcap * 2 : 1 << 16; heap = realloc(heap, hcap * sizeof(node_t)); }
    int i = hn++;
    while (i > 0) { int p = (i - 1) / 2; if (heap[p].f <= f) break; heap[i] = heap[p]; i = p; }
    heap[i].f = f; heap[i].idx = idx;
}
static node_t hpop(void) {
    node_t top = heap[0], last = heap[--hn];
    int i = 0;
    for (;;) {
        int c = 2 * i + 1; if (c >= hn) break;
        if (c + 1 < hn && heap[c + 1].f < heap[c].f) c++;
        if (heap[c].f >= last.f) break;
        heap[i] = heap[c]; i = c;
    }
    heap[i] = last; return top;
}

/* Rip-up support: cells of other nets' tracks may be crossed at a cost when
 * cross_cost > 0. Pads (hard[] set), blockages (-1) and the hard net never can. */
static const uint8_t *g_hard; static int g_hard_net; static float g_cross; static const float *g_hist;

/* 1: clear, 0: blocked, 2: clear only by ripping another net's track. */
static int disk_ok(const int32_t *occ, int W, int H, int l, int x, int y, int net,
                   const int16_t *dx, const int16_t *dy, int nd) {
    size_t base = (size_t)l * W * H;
    const int32_t *g = occ + base;
    int soft = 0;
    for (int k = 0; k < nd; k++) {
        int xx = x + dx[k], yy = y + dy[k];
        if (xx < 0 || yy < 0 || xx >= W || yy >= H) return 0;
        int32_t o = g[yy * W + xx];
        if (o != 0 && o != net) {
            if (g_cross <= 0 || o < 0 || o == g_hard_net || g_hard[base + yy * W + xx]) return 0;
            soft = 1;
        }
    }
    return soft ? 2 : 1;
}

void set_rip(const uint8_t *hard, int hard_net, float cross_cost) {
    g_hard = hard; g_hard_net = hard_net; g_cross = cross_cost;
}
/* PathFinder-style history: extra cost per cell that keeps being fought over. */
void set_hist(const float *hist) { g_hist = hist; }

/* Returns path length (cells) written to out (as l*H*W+y*W+x, target first),
 * 0 if no path. src/tgt are flat cell indices. */
int astar(const int32_t *occ, int W, int H, int L, int net,
          const int32_t *src, int nsrc, const uint8_t *tgt,
          const int16_t *tdx, const int16_t *tdy, int ntd,      /* track disk */
          const int16_t *vdx, const int16_t *vdy, int nvd,      /* via disk */
          float via_cost, const float *layer_cost, const uint8_t *layer_ok,
          int plane_layer, int plane_net,
          int bx0, int by0, int bx1, int by1,                    /* search window */
          int hx, int hy,                                        /* heuristic point (-1: none) */
          int32_t *out, int maxout)
{
    size_t N = (size_t)W * H * L;
    float *g = malloc(N * sizeof(float));
    int32_t *par = malloc(N * sizeof(int32_t));
    uint8_t *closed = calloc(N, 1);
    for (size_t i = 0; i < N; i++) g[i] = 1e30f;
    hn = 0;
    size_t WH = (size_t)W * H;
    for (int i = 0; i < nsrc; i++) { g[src[i]] = 0; par[src[i]] = -1; hpush(0, src[i]); }
    static const int DX[8] = {1, -1, 0, 0, 1, 1, -1, -1};
    static const int DY[8] = {0, 0, 1, -1, 1, -1, 1, -1};
    static const float DC[8] = {1, 1, 1, 1, 1.41421f, 1.41421f, 1.41421f, 1.41421f};
    static const uint8_t zero1 = 0;
    if (!g_hard) { g_hard = &zero1; g_cross = 0; }
    int32_t found = -1;
    long expanded = 0;
    while (hn > 0) {
        node_t nd = hpop();
        int32_t idx = nd.idx;
        if (closed[idx]) continue;
        closed[idx] = 1;
        if (tgt[idx]) { found = idx; break; }
        if (++expanded > 2500000) break;
        int l = idx / WH; int rem = idx % WH; int y = rem / W, x = rem % W;
        float gc = g[idx];
        if (l == plane_layer) continue;
        for (int k = 0; k < 8; k++) {
            int xx = x + DX[k], yy = y + DY[k];
            if (xx < bx0 || yy < by0 || xx > bx1 || yy > by1) continue;
            int32_t ni = l * WH + yy * W + xx;
            if (closed[ni]) continue;
            int ok = tgt[ni] ? 1 : disk_ok(occ, W, H, l, xx, yy, net, tdx, tdy, ntd);
            if (!ok) continue;
            if (k >= 4) {  /* diagonal: both orthogonal neighbours must be clear too */
                if (!disk_ok(occ, W, H, l, x + DX[k], y, net, tdx, tdy, ntd) &&
                    !disk_ok(occ, W, H, l, x, y + DY[k], net, tdx, tdy, ntd)) continue;
            }
            float ng = gc + DC[k] * layer_cost[l] + (ok == 2 ? g_cross : 0) + (g_hist ? g_hist[ni] : 0);
            if (ng < g[ni]) {
                g[ni] = ng; par[ni] = idx;
                float h = 0;
                if (hx >= 0) { float ax = fabsf(xx - hx), ay = fabsf(yy - hy);
                    h = (ax > ay ? ax + 0.41421f * ay : ay + 0.41421f * ax); }
                hpush(ng + h, ni);
            }
        }
        /* via to the other layers */
        int vok = -1;
        for (int l2 = 0; l2 < L; l2++) {
            if (l2 == l || !layer_ok[l2]) continue;
            int32_t ni = l2 * WH + y * W + x;
            if (closed[ni]) continue;
            if (l2 == plane_layer && net != plane_net) continue;
            if (vok < 0) {
                vok = 1;
                for (int l3 = 0; l3 < L && vok; l3++) {
                    if (l3 == plane_layer) continue;
                    int r = disk_ok(occ, W, H, l3, x, y, net, vdx, vdy, nvd);
                    if (!r) vok = 0; else if (r == 2) vok = 2;
                }
            }
            if (!vok) break;
            float ng = gc + via_cost + (vok == 2 ? g_cross : 0);
            if (ng < g[ni]) { g[ni] = ng; par[ni] = idx; hpush(ng, ni); }
        }
    }
    int n = 0;
    if (found >= 0) {
        for (int32_t c = found; c >= 0 && n < maxout; c = par[c]) out[n++] = c;
    }
    free(g); free(par); free(closed);
    return n;
}

/* Stamp a disk of net id at each listed cell on its layer (or all layers when all=1). */
void stamp(int32_t *occ, int W, int H, int L, int net, const int32_t *cells, int n,
           const int16_t *dx, const int16_t *dy, int nd, int all, int skip_layer)
{
    size_t WH = (size_t)W * H;
    for (int i = 0; i < n; i++) {
        int l = cells[i] / WH; int rem = cells[i] % WH; int y = rem / W, x = rem % W;
        for (int ll = (all ? 0 : l); ll < (all ? L : l + 1); ll++) {
            if (ll == skip_layer) continue;
            for (int k = 0; k < nd; k++) {
                int xx = x + dx[k], yy = y + dy[k];
                if (xx < 0 || yy < 0 || xx >= W || yy >= H) continue;
                int32_t *o = occ + ll * WH + yy * W + xx;
                if (*o == 0) *o = net;
            }
        }
    }
}
