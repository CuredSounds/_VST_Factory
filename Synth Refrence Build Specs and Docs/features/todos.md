# QA listening log

**Backlog and sprint status:** see master **[`../ROADMAP.md`](../ROADMAP.md)**  
**Shipped changes:** see **[`../CHANGELOG.md`](../CHANGELOG.md)**

---

## Sprint 6 P1 — DAW preset pass (post-P0)

**Priority smoke tests** (tail + CPU regressions from P0 fixes):

| Preset | Check |
|--------|-------|
| `FX_Deep_Space`, `Void_Cathedral`, `FX_Grit_Echo`, `FX_Warm_Tape` | Reverb/delay tail audible ~2–8 s after note-off (no chop) |
| `Ladder_Drive_Bass`, `Unison_Saw_Aggro`, `Neural_Storm` | 16-note chord — CPU stable vs pre-P0 (no 16× distortion rack) |
| `Granular_Shimmer`, `Mycelium_Pad`, `Layer_Ether_Stack` | Layer/mod presets still sound correct |

- [ ] Listen to all **52+** factory presets in a DAW (VST3 or AU)
- [ ] Note clicks, silence, CPU spikes, mod not audible, wrong category
- [ ] Log issues in the table below

| Preset | Issue | Severity |
|--------|-------|----------|
| | | |

---

## Done checklist (reference — do not edit)

Sprints 1–5 completion list archived in [`../CHANGELOG.md`](../CHANGELOG.md) (2026-06-20 through 2026-06-27 entries).
