# VitePress pages for docs.autoconnecto.in

These Markdown files are **source content** for the public documentation site (`autoconnecto-docs` repo). Copy or merge them into the matching paths under `docs/` in that repository, then run the docs build/deploy workflow.

| File here | Target on docs site |
|-----------|---------------------|
| `about/whats-new.md` | `/about/whats-new` |
| `solutions/machine-fleet.md` | `/solutions/machine-fleet` |
| `solutions/generator-monitoring.md` | `/solutions/generator-monitoring` |
| `solutions/edge-gateways.md` | `/solutions/edge-gateways` |
| `developer/ota-firmware.md` | `/developer/ota-firmware` |

Update `docs/.vitepress/config.ts` sidebar/nav to link these pages after copying.

SDK contracts referenced from this repo:

- [MACHINE_RUNTIME.md](../../MACHINE_RUNTIME.md)
- [MACHINE_RUNTIME_HARDWARE_BOM.md](../../MACHINE_RUNTIME_HARDWARE_BOM.md)
- [CONNECTIVITY.md](../../CONNECTIVITY.md)
- [OTA.md](../../OTA.md)
