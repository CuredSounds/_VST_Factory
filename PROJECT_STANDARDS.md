# Project Standard Operating Procedures (SOP)

Single source of truth for **Softysn1 / Bio-Synth** project structure, documentation, and development workflow. All contributors and AI assistants follow these standards.

---

## 1. Project initialization (Day 1)

Every project establishes immediately:

| Item | This repo |
|------|-----------|
| **Repository** | GitHub — `_vst_softysn1` |
| **Directory structure** | See [§1.1](#11-directory-structure-best-practice) below |
| **Roadmap** | [`ROADMAP.md`](ROADMAP.md) at repo root |
| **Change log** | [`CHANGELOG.md`](CHANGELOG.md) at repo root |

### 1.1 Directory structure (best practice)

Modular layout: **separation of concerns**, **centralized docs**, **feature-scoped source**. Any developer or AI should navigate from day one without guessing where things live.

**Generic template** (adapt names to stack):

```text
Project_Name/
├── .github/          # CI/CD workflows (build, test, release)
├── docs/             # Architecture, usage, doc index (master tracking at repo root)
├── infra/            # IaC / deploy (optional — not used in this JUCE repo)
├── src/              # Application source (modular by domain)
├── tests/            # Automated tests (mirror src layout)
├── tools/            # One-off scripts, generators, dev utilities
├── .gitignore
├── LICENSE
├── README.md         # Entry point → links master docs
├── ROADMAP.md        # Master roadmap (this repo: repo root)
└── CHANGELOG.md      # Master changelog (this repo: repo root)
```

**Softysn1 / Bio-Synth — actual layout:**

```text
_vst_softysn1/
├── .github/                  # CI/CD (add workflows here)
├── docs/                       # Doc index + extended architecture notes
├── features/                   # Reference only — parity, gaps, QA log (not master tracking)
│   ├── features.md           # Competitive matrix
│   ├── GAPS.md               # Gap tracker
│   ├── todos.md              # QA preset listening log
│   └── PROJECT_STANDARDS.md  # Redirect → ../PROJECT_STANDARDS.md
├── Source/                   # JUCE plugin source (= generic src/)
│   ├── Core/                 # Wrapper — JUCE AudioProcessor, APVTS, presets
│   │   ├── PluginProcessor.* # Main audio I/O + parameter layout
│   │   ├── PluginEditor.*    # Editor shell
│   │   ├── PresetManager.*   # Preset load/save
│   │   └── Parameters.h      # Central APVTS ID registry (no string literals)
│   ├── DSP/                  # Engine — pure audio (testable without UI)
│   │   ├── EffectModules/    # Modular FX (one folder per module)
│   │   │   ├── Compressor/
│   │   │   ├── Delay/
│   │   │   ├── Reverb/
│   │   │   ├── Chorus/
│   │   │   ├── EQ/
│   │   │   └── Distortion/   # SaturationModule
│   │   ├── Common/           # Math utils, windowing, fast-sine (shared)
│   │   ├── State/            # VoiceParameterMatrix, shared DSP state
│   │   └── …                 # Voice, oscillators, mod matrix, generators
│   ├── UI/                   # Custom UI (no audio processing)
│   │   ├── LookAndFeel/      # BioSynthLookAndFeel
│   │   ├── Widgets/          # Custom sliders, meters, buttons
│   │   └── Panels/           # Tab panels
│   ├── Script/               # Lua host
│   └── (shims)               # Legacy forwarders at old paths — prefer Core/DSP paths
├── Tests/
│   └── DSP_Tests/            # Unit tests for Source/DSP (Catch2/GoogleTest)
├── Presets/
│   └── Factory/              # Bundled .softpreset bank (CMake resources)
├── tools/                    # Dev utilities
│   └── build_factory_presets.py
├── Scripts/                  # Legacy helpers (prefer tools/ for new scripts)
├── Assets/                   # Fonts, images (BinaryData)
├── cmake-build-debug/        # Local build output (gitignored; JUCE in _deps/juce-src)
├── CMakeLists.txt            # Build + format targets (VST3, AU, Standalone)
├── README.md                 # Entry point
├── ROADMAP.md                # ★ Master roadmap
├── CHANGELOG.md              # ★ Master changelog
└── PROJECT_STANDARDS.md      # ★ This file
```

**Where to put new work**

| Adding… | Location |
|---------|----------|
| Pure DSP / voice / FX algorithm | `Source/DSP/` (FX modules → `Source/DSP/EffectModules/<Name>/`) |
| APVTS, processor, preset I/O | `Source/Core/` |
| Parameter ID constant | `Source/Core/Parameters.h` |
| UI panel or widget | `Source/UI/Panels/` or `Source/UI/Widgets/` |
| Look-and-feel / theme | `Source/UI/LookAndFeel/` |
| DSP unit test | `Tests/DSP_Tests/` (link only `Source/DSP/`) |
| Factory preset | Regenerate via `tools/build_factory_presets.py` → `Presets/Factory/` |
| Dev / batch script | `tools/` (not repo root) |
| Architecture or usage doc | `docs/` or root `ARCHITECTURE.md` |
| Sprint / shipped-work tracking | `ROADMAP.md` + `CHANGELOG.md` only |
| Competitive / gap notes | `features/` (reference, no duplicate backlog) |
| CI workflow | `.github/workflows/` |

**Decoupled architecture (Core / DSP / UI)**

1. **Engine** (`Source/DSP/`) — all pure audio logic. No `#include` of `PluginEditor`, panels, or APVTS from here.
2. **Wrapper** (`Source/Core/`) — JUCE `AudioProcessor`, parameter layout, preset manager, bridges engine ↔ host.
3. **UI** (`Source/UI/`) — components and styling only; reads/writes APVTS via attachments.

Legacy shim headers at old paths (e.g. `Source/PluginProcessor.h`) forward to `Core/` — new code should use canonical paths.

**AI / agent instruction**

> We use a decoupled architecture. All pure DSP logic must reside in `Source/DSP/`. All JUCE-wrapper and UI-interface code must reside in `Source/Core/` or `Source/UI/`. When you suggest new files, specify the path relative to these directories (e.g. `Source/DSP/EffectModules/Compressor/CompressorModule.h`).

**Principles**

1. **Standardized init** — repo + this tree + roadmap on day one.
2. **Centralized truth** — master tracking in root `ROADMAP.md` / `CHANGELOG.md`; `docs/` indexes everything else.
3. **Modular source** — `Source/DSP` vs `Source/UI` keeps AI/human context scoped per feature area.
4. **High-density logs** — one scannable `CHANGELOG.md`; no scattered status files.

Legacy / do not use for new work: `Synth_GUI/` (old UI experiments), duplicate roadmaps under `features/`.

---

## 2. Master documentation

**Do not scatter tracking notes.** Use these two files as primary context for humans and AI:

| File | Purpose |
|------|---------|
| **[`ROADMAP.md`](ROADMAP.md)** | Vision, milestones, sprint status, backlog |
| **[`CHANGELOG.md`](CHANGELOG.md)** | Dated, scannable history of shipped changes |

Supporting reference (not tracking):

| File | Purpose |
|------|---------|
| [`features/features.md`](features/features.md) | Competitive parity vs Avenger / Phase Plant / Falcon |
| [`features/GAPS.md`](features/GAPS.md) | Gap tracker detail |
| [`features/todos.md`](features/todos.md) | **QA listening log only** (preset pass) |
| [`features/UI_UX_PLAN.md`](features/UI_UX_PLAN.md) | Sprint 6 GUI refactor (parallel track) |
| [`docs/issues.md`](docs/issues.md) | Technical review — bugs tracked in ROADMAP P0 |
| [`ARCHITECTURE.md`](ARCHITECTURE.md) | Code architecture |
| [`tools/build_factory_presets.py`](tools/build_factory_presets.py) | Regenerate factory preset bank |

**AI instruction:** When context is needed, read `ROADMAP.md` and `CHANGELOG.md` first. Inject only the sections relevant to the current task.

---

## 3. Change log strategy

Keep entries **dense and scannable** — high-level summaries, not line-by-line diffs.

**Format:**

```text
[YYYY-MM-DD] [Feature/Fix Name]: Brief summary. (optional ticket/issue)
```

**CHANGELOG.md sections:**

```markdown
## [YYYY-MM-DD] Sprint N — short title
- **[Area]:** What changed and why it matters.
```

---

## 4. AI & dev communication workflow

- **Feature-based chat** — start a fresh session per major feature or bug, not one endless thread.
- **Context injection** — paste the relevant `ROADMAP.md` milestone/backlog block and recent `CHANGELOG.md` entries into the first message.
- **After shipping** — append one `CHANGELOG.md` entry; update checkboxes in `ROADMAP.md`.

---

## 5. Preset & build conventions

- Factory presets live in **`Presets/Factory/`** (bundled into VST3/AU/Standalone via CMake).
- Regenerate bank: `python3 tools/build_factory_presets.py`
- User presets: `~/Library/Application Support/Softysn1/Presets/` (macOS).

---

## 6. Doc maintenance rules

1. **One change, one changelog line** (group related edits under one sprint heading).
2. **Roadmap checkboxes** reflect reality — mark done only when code ships and builds.
3. **Satellite docs** (`features/*.md`) link back to master docs; do not duplicate backlog.
4. **Stale docs** get a one-line redirect, not a second copy of the roadmap.

---

## 7. Strict rules (Core / DSP / UI)

These are **non-negotiable** for new code. Violations block merge (or get fixed before the next sprint).

### Must

| Rule | Detail |
|------|--------|
| **DSP stays pure** | New audio algorithms go in `Source/DSP/`. No `#include` of `PluginEditor`, `*Panel*`, or UI headers from DSP. |
| **Wrapper stays thin** | `Source/Core/` owns APVTS, preset I/O, MIDI/host glue. No DSP math here — delegate to engine types. |
| **UI stays visual** | `Source/UI/` reads/writes parameters via attachments or processor callbacks. No per-sample audio in UI code. |
| **One param ID source** | New APVTS IDs are added to `Source/Core/Parameters.h` first, then referenced everywhere else. |
| **One module, one folder** | New FX modules: `Source/DSP/EffectModules/<Name>/<Name>Module.{h,cpp}` inheriting `EffectModule`. |
| **Canonical includes** | New files use full paths (`Core/…`, `DSP/…`, `UI/…`). Do not add new dependencies on shim headers. |
| **CMake honesty** | Every new `.cpp` is listed in `CMakeLists.txt` at its **canonical** path (not shim path). |
| **Tests mirror DSP** | New non-trivial DSP logic gets a corresponding test stub or case in `Tests/DSP_Tests/`. |

### Must not

| Rule | Detail |
|------|--------|
| **No root `Source/` dumps** | Do not add new `.cpp`/`.h` directly under `Source/` (except one-line shim forwarders during migration). |
| **No duplicate backlog** | Do not create second roadmaps or status files under `features/` or `docs/`. |
| **No build artifacts in git** | `cmake-build-*`, `build/`, generated IDE caches stay gitignored. |
| **No JUCE edits for features** | Feature work lives in `Source/`. JUCE is fetched by CMake (`8.0.12`); never patch `_deps/juce-src` for product features. |
| **No string-literal param IDs** | In new code, use `Softysn1::Params::*` from `Parameters.h`. |

### Include direction (allowed edges)

```text
UI  ──► Core  ──► DSP
         │
         └──► JUCE modules

DSP  ──► DSP/Common, DSP/State, juce_dsp (only)
DSP  ✗── UI, Core (except shared headers explicitly in DSP/State)
```

---

## 8. This repo — migration backlog

Architecture refactor is **partial**. Shims at old paths exist so the build stays green; **new work uses canonical paths only**.

| Still at legacy path | Target (when touched) |
|----------------------|------------------------|
| `Source/SynthVoice.*`, `Source/SynthSound.h` | `Source/DSP/Voice/` |
| `Source/Wavetable*.*`, `Source/Lfo.*` | `Source/DSP/Oscillators/` or `Source/DSP/Modulators/` |
| `Source/EffectSlotComponent.h`, `EffectLaneComponent.h`, `EffectParameterPanel.h` | `Source/UI/Widgets/` or `Source/UI/Panels/` |
| `Source/PerformanceTelemetry.*`, `TelemetryLog.*` | `Source/Core/` or `Source/DSP/State/` |
| `Source/DSP/*Module.h` shims (flat) | Already moved — delete shims when no includes remain |

Migrate opportunistically: when you edit a legacy file, move it and leave a one-line shim. Do not mass-rename in a single drive-by PR unless build + QA are scheduled.

---

## 9. Greenfield checklist (next JUCE plugin)

Copy this block on **day 1** of a new instrument project:

```text
[ ] ROADMAP.md + CHANGELOG.md + PROJECT_STANDARDS.md at repo root
[ ] Source/Core/     — PluginProcessor, PluginEditor, Parameters.h, PresetManager
[ ] Source/DSP/      — empty EffectModules/, Common/, State/
[ ] Source/UI/       — LookAndFeel/, Widgets/, Panels/
[ ] Tests/DSP_Tests/ — README + CMake option SOFTYSN1_BUILD_TESTS (optional)
[ ] CMakeLists.txt   — target_include_directories(PRIVATE Source)
[ ] docs/FOLDER_STRUCTURE.md — link from README
[ ] .gitignore       — cmake-build-*, build/, .idea user files
[ ] No feature .cpp under Source/ root (except JucePluginMain if required)
```

**Agent prompt for greenfield:**

> Decoupled JUCE architecture. DSP in `Source/DSP/`, wrapper in `Source/Core/`, UI in `Source/UI/`. Parameter IDs in `Core/Parameters.h`. FX modules in `DSP/EffectModules/<Name>/`. Specify full paths for every new file.

See also: [`docs/GREENFIELD_CHECKLIST.md`](docs/GREENFIELD_CHECKLIST.md).
