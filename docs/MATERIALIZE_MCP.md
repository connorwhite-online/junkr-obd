# Putting a Build on Materialize with the MCP Connector

How the stage 1 tap box got onto [Materialize](https://www.materialize.cc)
(project "KZJ95 ECU Tap, Stage 1", private draft) on 2026-11-11, so you
can repeat it for stage 2 and 3 or for a new revision. Claude did these
steps through the Materialize MCP connector, and they map one-to-one onto
its tools.

Materialize hosts the project page (files, BOM, build guide, wiring,
photos) and gets parts printed through its print partner CraftCloud.
**Nothing is ordered without you:** an order is a draft until you approve
and pay at the emailed link, because this connection has no spending
policy (`materialize_whoami` → `approval: confirm_by_email`).

## The steps

| # | Step | Tool | Notes |
| --- | --- | --- | --- |
| 0 | Check the account | `materialize_whoami` | Confirms the account, upload defaults (private, CC-BY) and how orders get approved. |
| 1 | Get a presigned upload URL per STL | `materialize_request_upload_url` (filename, sizeBytes) | Returns `uploadUrl` + `storageKey`. Valid for 1 h. |
| 2 | Upload the bytes | `curl -X PUT --data-binary @part.stl "<uploadUrl>"` | Expect HTTP 200. |
| 3 | Register each upload | `materialize_register_upload` (storageKey, format `stl`, fileUnit `mm`, metadata) | Set `name`, a description that names the **material**, `license: mit`, `visibility: private`. Returns `fileAssetId` (for quotes) and `fileId` (for projects). |
| 4 | Check printability | `materialize_check_printability` (fileAssetId) | Flags thin walls per process. See "What it found" below. |
| 5 | Shortlist and price materials | `materialize_list_materials` (group), `materialize_get_quote` (fileAssetId, materialIds[]) | Quote several materials in one call to compare. |
| 6 | Price the whole build as one order | `materialize_quote_bom` (items: fileAssetId + materialId + finish) | Gives the cheapest single vendor and the cheapest mix, with shipping and minimums counted once. |
| 7 | Create the project | `materialize_create_project` (name, description, buildGuide markdown, category `automotive`, tags, `repoUrl`, `fileIds`, `visibility: private`, `license: mit`) | Bundled files take the project's license, tags and category. |
| 8 | Add the BOM | `materialize_set_project_bom` (items: name, quantity, unit, notes, sourceUrl) | Replaces the whole list each time. |
| 9 | Add the wiring diagram | `materialize_request_circuit_upload_url` → PUT → `materialize_add_project_circuit_image` | KZW-211.png. |
| 10 | Add renders | Render each STL with OpenSCAD (`docs/build-stages/renders/`), then `materialize_request_photo_upload_url` → PUT → `materialize_add_file_photo` + `materialize_set_file_cover_photo` per file, and `materialize_add_project_photo` + `materialize_set_project_cover_photo` + `materialize_reorder_project_photos` for the gallery | Without these, the page shows almost nothing. Swap in real photos once it's built. |
| 10b | Images inside the build guide | `materialize_add_project_inline_image` (a photo storageKey) → paste the returned `![](...)` into the guide → `materialize_update_project` | Circuit storage keys are refused; use a photo upload. |
| 11 | Order (only when you say so) | `materialize_create_order` (one call per part, from the quote_bom lines) | Creates a draft. You get an email, review it, and pay at `confirmationUrl`. Check status with `materialize_get_order`. |
| 12 | Publish (optional) | `materialize_update_project` (`visibility: public`) | It's private until you flip this. |

## What it found for these parts

- **The interposer plugs and shrouds have 0.35 mm walls** between
  terminal cavities. That's the same as the OEM header and can't grow
  without moving the cavities. Printability check on the E8 plug: 35 % of
  sampled surface points are under the 0.8 mm MJF/SLS minimum, and 19 %
  under the 0.6 mm SLA minimum. So these four parts go in **Tough Resin
  (SLA)**, with MJF PA12 as the fallback. Some vendors may still refuse or
  "fix" them, so put "do not thicken walls" in the order notes.
- **The chambers and tap box** have 2 mm walls and are fine in anything.
  They go in **SLS Nylon PA12** so the whole order ships from one vendor.
  PETG on the home printer works too.
- **One-vendor quote, 2026-11-11, US shipping:** $157.86 total from
  Unionfab (China), arriving 18 to 23 November. That's ten parts at about
  $11 each, plus $42.99 shipping and a $3.35 fee. A US-only order costs
  more: the cheapest US vendors each made only one of the two materials,
  so you'd pay shipping twice.

## IDs (for scripting a re-quote or an order)

Project: `81f91cfb-5ecb-4c47-a87c-f1dd0743c65e`

| Part | Material | fileAssetId |
| --- | --- | --- |
| E6 female plug | Tough Resin | `21f0fbe9-20e1-4dc2-96df-3dcfb5e12ccf` |
| E6 male shroud | Tough Resin | `4bb6e8ca-8da9-45ce-a0e2-246acba2ef05` |
| E6 chamber bottom | SLS PA12 | `fdfe0576-84f4-4dd7-813a-e2a4a30a3cf6` |
| E6 chamber top | SLS PA12 | `c1cd591d-7329-4791-83e6-dcb30c88aa14` |
| E8 female plug | Tough Resin | `54412647-dbea-4585-8d1d-58c3c6ab19f3` |
| E8 male shroud | Tough Resin | `e16e4862-7b83-45a0-af35-e84f39a2dc38` |
| E8 chamber bottom | SLS PA12 | `ad19cf41-7238-4d38-9f97-497e82985747` |
| E8 chamber top | SLS PA12 | `a6542d20-cbc1-4071-bbcd-961bda71d59c` |
| Tap box base | SLS PA12 | `3f17e8ba-41b2-47a0-a033-97ca38c27542` |
| Tap box lid | SLS PA12 | `1c26baf2-f901-4f1d-97bb-7cebeb01e635` |

Material IDs: Tough Resin `97f6c587-73ef-51c0-bb6a-aedf666ce59a`,
SLS Nylon PA12 `03a417a3-d9c9-5717-ab7f-3966a8ad4555`,
HP MJF PA12 `67f126d6-a917-5f87-8d26-b87161656539`.

## When a part changes

1. Re-export the STL (`hardware/ecu_interposer/cad/export_all.sh`).
2. Repeat steps 1 to 4 for that file only.
3. Swap it in the project: `materialize_add_project_files` with the new
   fileId, `materialize_remove_project_file` with the old one.
4. Re-run step 6, because quotes expire after about 30 minutes.
