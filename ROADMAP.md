# _VST_Factory Harness Roadmap

Tracks harness implementation in this repo. Softsyn1 stabilization is **deferred** until items
below are complete. Full ecosystem vision: `_VST_Factory_Claude_docs/06_ROADMAP.md`.

## Phase 0 — Harness (this repo)

- [x] Canonical doc set (`_VST_Factory_Claude_docs/` 00–07)
- [x] Root README, AGENTS.md, .gitignore
- [x] Sandbox Greenfield layout (Core/DSP/UI + Catch2)
- [x] Quality gate scripts (`tools/run_*.sh`)
- [x] Local git hooks (`.githooks/`)
- [x] GitHub Actions macOS CI
- [x] Agent skills (`.cursor/skills/`)
- [x] "Done gate" PR template + sandbox ROADMAP/CHANGELOG

## Phase 1 — Softsyn1 (deferred)

Blocked until Phase 0 CI is green for 1–2 weeks.

- [ ] Copy `01_AGENTS_TEMPLATE.md` → Softsyn1 repo as `AGENTS.md`
- [ ] Wire CI gates into Softsyn1
- [ ] Fix convergent RT-safety findings from 2026-06-29 reviews
- [ ] Reconcile conflicting roadmap documents with verification artifacts

## Phase 2 — Plugin #2 scaffold

- [ ] CLI/template repo from sandbox fork
- [ ] Second plugin: effects processor (cheaper than second synth)
