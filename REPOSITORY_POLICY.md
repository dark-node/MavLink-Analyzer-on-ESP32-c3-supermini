# Repository policy

This folder **is** the public GitHub repository. It contains documentation,
API headers, a few reference `.cpp` files, and **stubs** for proprietary core
logic.

The owner maintains the **full, buildable firmware** in a separate private
project directory (not on GitHub).

## Goals

1. **Transparency** — visitors can see architecture and documentation.
2. **Protection** — bridge, parser, web UI, NVS, CRC table, and generated
   MAVLink C are **not** published in usable form.
3. **License** — browsing GitHub does **not** grant permission to use or
   redistribute ([LICENSE](LICENSE)).

See [docs/PUBLIC_CODE_MANIFEST.md](docs/PUBLIC_CODE_MANIFEST.md).

## What visitors may do

| Allowed | Not allowed |
|---------|-------------|
| Read docs and partial source | Use code or firmware |
| Star / watch / open Issues | Fork and republish as their project |
| Learn from headers and specs | Complete stubs and distribute |
| Link to this repository | Imply open-source license |

Written permission required for any use beyond reading ([CONTACT.md](CONTACT.md)).

## GitHub settings

- **Visibility:** Public  
- **License field:** None (use custom `LICENSE` file)  
- **Description:** `Proprietary ESP32-C3 MAVLink bridge. Docs + partial source.`

## Firmware binaries

Do not attach `.bin` files to Releases unless you intend to distribute firmware
under LICENSE terms. Binaries are not included in this tree by default.

## Owner workflow

When the private project changes, copy updates into this public tree manually:

- Refresh docs from the private project.
- Replace only the three published `.cpp` files if they changed.
- Regenerate stubs if module list changes.
- Never commit full `src/` implementation, `tools/`, `lib/mavlink/generated/`,
  or `crc_common_table.h`.
