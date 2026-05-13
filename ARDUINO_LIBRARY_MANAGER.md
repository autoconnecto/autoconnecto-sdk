# Arduino Library Manager — compliance and submission

This repository (`autoconnecto-sdk`) is laid out as a **standalone Arduino library**: `library.properties` is at the **repository root**, with `src/`, `examples/`, and `LICENSE`, per the [Arduino Library Specification](https://arduino.github.io/arduino-cli/latest/library-specification/) and [Library Manager FAQ](https://github.com/arduino/library-registry/blob/main/FAQ.md).

The Autoconnecto **workspace** meta-repo lists `sdk/` as a submodule pointing here; Library Manager indexes **this** GitHub repo only, not the parent monorepo.

## Requirements (checklist before you submit)

- [x] `library.properties` at repo root, with `name`, `version`, `author`, `maintainer`, `sentence`, `paragraph`, `category`, `url`, `architectures`, `includes`, `depends`, `license`.
- [x] Source under `src/` (1.5 format).
- [x] Examples under `examples/` (lowercase), each sketch folder name matches its `.ino` name.
- [x] `LICENSE` at repo root (MIT).
- [x] `keywords.txt` at repo root (optional but recommended).
- [ ] **Public** GitHub repository.
- [ ] **No** `.development` file in any tagged release (Arduino indexer rejects it). Listed in `.gitignore` for local use only.
- [ ] **No** symlinks in the repo.
- [ ] **No** `.exe` files.
- [ ] **Git submodules** must not be required for the library to build (Library Manager does not ship submodule contents).
- [ ] **Git tag** (or GitHub Release) on a commit that satisfies all rules; bump `version` in `library.properties` for every indexed release and ensure it is **unique** per tag.

## Local verification

Install [Arduino Lint](https://github.com/arduino/arduino-lint) and run from this directory:

```bash
arduino-lint --library-manager submit --compliance strict
```

Or rely on the GitHub Action in `.github/workflows/arduino-lint.yml` after you push.

## Submitting to the index

1. Make the repo public.
2. Ensure `version` in `library.properties` matches the release you are tagging.
3. Create an annotated tag (or GitHub Release), e.g. `v1.0.1`, and push it.
4. Open a pull request on **[arduino/library-registry](https://github.com/arduino/library-registry)** following [their README](https://github.com/arduino/library-registry#readme) (add your repo URL to the registration list).
5. Watch the [indexer logs](https://github.com/arduino/library-registry/blob/main/FAQ.md#can-i-check-on-library-releases-being-added-to-library-manager) if a release does not appear within ~1 hour.

## After acceptance

Users install via **Sketch → Include Library → Manage Libraries…** and search for `AutoconnectoSDK`. New versions: bump `library.properties` `version`, tag, push; the indexer picks up new tags automatically.
