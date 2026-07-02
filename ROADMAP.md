# _VST_Factory Harness Roadmap

Tracks harness implementation in this repo. Full ecosystem vision: `_VST_Factory_Claude_docs/06_ROADMAP.md`.

## Phase 0 — Harness (this repo) — complete

- [x] Canonical doc set (`_VST_Factory_Claude_docs/` 00–07)
- [x] Root README, AGENTS.md, .gitignore
- [x] Sandbox Greenfield layout (Core/DSP/UI + Catch2)
- [x] Quality gate scripts (`tools/run_*.sh`)
- [x] Local git hooks (`.githooks/`)
- [x] GitHub Actions macOS CI (`.github/workflows/sandbox-ci.yml`)
- [x] Agent skills (`.cursor/skills/`)
- [x] "Done gate" PR template + sandbox ROADMAP/CHANGELOG
- [x] Antigravity docs archived → `old_files/` + `SUPERSEDED.md`
- [x] Case-study files redirect to sandbox (`PROJECT_STANDARDS.md`, `CHANGELOG.md`, `GREENFIELD_CHECKLIST.md`)

### Success criteria (harness locked)

- [x] `vst_factory_sandbox` matches Greenfield checklist steps 1–6
- [x] `git push` runs build + DSP tests via `.githooks/pre-push`
- [x] GitHub Actions runs full gate stack on macOS (`sandbox-ci.yml`)
- [x] Root `AGENTS.md` + sandbox `AGENTS.md` are active agent context
- [x] Documented path: fork sandbox → fill `AGENTS.md` from `01_AGENTS_TEMPLATE.md`

Verification: `vst_factory_sandbox/tools/run_all_gates.sh` (local); CI badge on README.

## Phase 1 — Softsyn1 trial — harness complete

Branch: [`try_vst_factory`](https://github.com/CuredSounds/_vst_softysn1/tree/try_vst_factory) on
[github.com/CuredSounds/_vst_softysn1](https://github.com/CuredSounds/_vst_softysn1)

- [x] Harness ported (`AGENTS.md`, `tools/run_all_gates.sh`, CI workflow)
- [x] Build + Catch2 pass locally on `try_vst_factory`
- [x] Fix convergent RT-safety P0 findings (string param lookups, `std::async` removal, warm re-open double-free)
- [x] Enable strict RT scan + `processBlock` allocation gate in CI
- [x] Preset smoke stable in CI (in-process Catch2; DawDreamer optional)
- [x] Reconcile roadmap docs with verification artifacts (this file + Softsyn1 `docs/VST_FACTORY_HARNESS.md`)

Verification artifacts:

- RT scan enforced: `SOFTYSN1_RT_SCAN_ENFORCE=true` in `.github/workflows/softysn1-ci.yml`
- Allocation + preset smoke: `Tests/DSP_Tests/ProcessorIntegration_test.cpp` (`[rt][allocation]`, `[preset][smoke]`)
- pluginval strictness 8: CI run [28618778910](https://github.com/CuredSounds/_vst_softysn1/actions/runs/28618778910) (quality-gates ✓)
- Warm re-open fix: [PR #5](https://github.com/CuredSounds/_vst_softysn1/pull/5) (`PartSynthSound::Ptr` ownership)

Softsyn1 stabilization backlog (not harness blockers — see `06_ROADMAP.md` Phase 1):

- [ ] QA listening pass for factory presets
- [ ] Lua script host sandbox hardening
- [ ] CLAP dependency pinned to commit hash

## Phase 2 — Plugin #2 scaffold — in progress

- [x] CLI scaffold from sandbox fork — `tools/scaffold_plugin.py`
- [x] Sandbox plugin-level `processBlock` allocation test — `vst_factory_sandbox/Tests/DSP_Tests/ProcessorIntegration_test.cpp`
- [x] Second plugin: effects processor — [CuredSounds/_vst_tape_saturator](https://github.com/CuredSounds/_vst_tape_saturator)
- [x] GitHub CI for `_vst_tape_saturator` — `.github/workflows/tape-saturator-ci.yml`
- [ ] CLAP format in sandbox (optional, pinned commit)

```bash
python3 tools/scaffold_plugin.py --name "My Effect" --out ../my_effect
```
