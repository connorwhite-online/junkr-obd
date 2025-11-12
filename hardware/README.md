# Hardware Projects Directory

This folder holds KiCad projects for each PCB in the three-board JNKR Gauge architecture.

| Sub-folder | Board | KiCad version |
|------------|-------|---------------|
| `intercept-board/` | ECU-Intercept inline tap | KiCad 7.0 |
| `dash-io-board/` | Dash I/O + Qualia carrier | KiCad 7.0 |
| `engine-bay-node/` | Engine-bay sensor node | KiCad 7.0 |

Each sub-folder will contain:
* `.kicad_pro` project file
* `schematics/` directory with `.kicad_sch`
* `pcb/` directory with `.kicad_pcb`
* `outputs/` for Gerbers/BOM/pos files

> **Note**: Commit binary KiCad files, but large fabrication ZIPs belong in Git-LFS or the release section, not the main repo.

