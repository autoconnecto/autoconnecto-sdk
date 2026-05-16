# Arduino Library Manager — compliance and submission

This repository (`autoconnecto-sdk`) is laid out as a **standalone Arduino library**: `library.properties` is at the **repository root**, with `src/`, `examples/`, and `LICENSE`, per the [Arduino Library Specification](https://arduino.github.io/arduino-cli/latest/library-specification/) and [Library Manager FAQ](https://github.com/arduino/library-registry/blob/main/FAQ.md).

The Autoconnecto **workspace** meta-repo lists `sdk/` as a submodule pointing here; Library Manager indexes **this** GitHub repo only, not the parent monorepo.

## Requirements (checklist before you submit)

- [x] `library.properties` at repo root, with `name`, `version`, `author`, `maintainer`, `sentence`, `paragraph`, `category`, `url`, `architectures`, `includes`, `depends`, `license`.
- [x] Source under `src/` (1.5 format).
- [x] Examples under `examples/` (lowercase), each sketch folder name matches its `.ino` name.
- [x] `LICENSE` at repo root (MIT).
- [x] `keywords.txt` at repo root (optional but recommended).
- [x] **Public** GitHub repository (`https://github.com/autoconnecto/autoconnecto-sdk`).
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

## Submitting to the index (do this once)

**Library URL to register (copy exactly):**

```text
https://github.com/autoconnecto/autoconnecto-sdk
```

### Steps (GitHub web — ~5 minutes)

1. **Fork** the registry: [github.com/arduino/library-registry/fork](https://github.com/arduino/library-registry/fork) → **Create fork** (under your `autoconnecto` account or personal account).

2. **Branch** on your fork: **main** → **Branches** → **New branch** → name e.g. `add-autoconnecto-sdk` → create from `arduino/library-registry` / `main`.

3. **Edit** `repositories.txt` on that branch: open the file → pencil **Edit** → add this line anywhere (one line only):

   ```text
   https://github.com/autoconnecto/autoconnecto-sdk
   ```

   Commit directly to `add-autoconnecto-sdk`.

4. **Pull request**: on your fork, **Contribute** → **Open pull request** → base `arduino/library-registry:main` ← compare your branch.

   **Title:** `Add AutoconnectoSDK library`

   **Body:**

   ```text
   - [x] My submission is for a library I maintain
   - [x] I have read the [submission requirements](https://github.com/arduino/library-registry/blob/main/FAQ.md#submission-requirements)
   - [x] The library repository is public and contains library.properties at the repo root
   - [x] I have run / passed arduino-lint (Library Manager submit, strict) on the library repo

   Library URL: https://github.com/autoconnecto/autoconnecto-sdk
   ```

5. **Watch the PR** — bot comments in a few minutes. Fix any **errors** it reports (warnings are OK).

6. When the PR is **merged**, Library Manager usually lists the library within **~24 hours**. Latest indexed release comes from your **git tags** (e.g. `v1.0.5`).

### If the bot rejects the library (not the PR)

1. Fix the issue in `autoconnecto-sdk`.
2. Bump `version` in `library.properties`.
3. Tag and push (e.g. `git tag v1.0.6 && git push origin v1.0.6`).
4. Comment on the registry PR: `@ArduinoBot please recheck`.

### Prerequisites already done on our side

- Tags on GitHub: `v1.0.0`, `v1.0.3`, `v1.0.5` (indexer uses tagged releases).
- `library.properties` at repo root; `LICENSE`, `keywords.txt`, examples layout.
- CI: `.github/workflows/arduino-lint.yml` on the SDK repo.

### After merge

Users: **Sketch → Include Library → Manage Libraries…** → search **AutoconnectoSDK** → Install (also installs **ArduinoJson**).

## After acceptance

Users install via **Sketch → Include Library → Manage Libraries…** and search for `AutoconnectoSDK`. New versions: bump `library.properties` `version`, tag, push; the indexer picks up new tags automatically.
