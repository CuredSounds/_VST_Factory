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

## Phase 1 — Softsyn1 trial

Branch: [`try_vst_factory`](https://github.com/CuredSounds/_vst_softysn1/tree/try_vst_factory) on
[github.com/CuredSounds/_vst_softysn1](https://github.com/CuredSounds/_vst_softysn1)

- [x] Harness ported (`AGENTS.md`, `tools/run_all_gates.sh`, CI workflow)
- [x] Build + Catch2 pass locally on `try_vst_factory`
- [ ] Fix convergent RT-safety findings (2026-06-29 reviews)
- [ ] Enable strict RT scan + `processBlock` allocation gate
- [ ] Preset smoke stable in CI (DawDreamer)
- [ ] Reconcile roadmap docs with verification artifacts

## Phase 2 — Plugin #2 scaffold

- [ ] CLI/template repo from sandbox fork
- [ ] Second plugin: effects processor
