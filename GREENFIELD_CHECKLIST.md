# Greenfield checklist — JUCE audio plugin

Use on **day 1** of a new instrument or effect project. Copy relevant sections into the new repo's `PROJECT_STANDARDS.md`.

---

## Repository skeleton

```text
Project_Name/
├── .github/workflows/       # build.yml at minimum
├── docs/
│   ├── README.md            # doc index
│   └── FOLDER_STRUCTURE.md
├── Source/
│   ├── Core/
│   │   ├── PluginProcessor.cpp/h
│   │   ├── PluginEditor.cpp/h
│   │   ├── Parameters.h       # ALL APVTS string IDs
│   │   └── PresetManager.cpp/h  # if needed
│   ├── DSP/
│   │   ├── Common/            # math, tables, utilities
│   │   ├── State/             # shared engine state
│   │   └── EffectModules/     # one folder per FX (if multi-FX)
│   ├── UI/
│   │   ├── LookAndFeel/
│   │   ├── Widgets/
│   │   └── Panels/
│   └── Script/                # optional automation host
├── Tests/
│   └── DSP_Tests/
├── Assets/                    # fonts, IRs, icons → BinaryData
├── tools/                     # generators, not repo root
├── Presets/Factory/           # bundled presets (optional)
├── CMakeLists.txt
├── README.md                  # links ROADMAP, CHANGELOG, PROJECT_STANDARDS
├── ROADMAP.md
├── CHANGELOG.md
├── PROJECT_STANDARDS.md
└── ARCHITECTURE.md
```

---

## Day-1 tasks

- [ ] Create folder tree above (empty dirs OK; add `.gitkeep` if needed)
- [ ] `CMakeLists.txt`: `target_include_directories(... PRIVATE Source)`
- [ ] `Parameters.h` with first param IDs before any UI attachments
- [ ] `ARCHITECTURE.md` § Core / DSP / UI with include-direction diagram
- [ ] `.gitignore`: `cmake-build-*`, `build/`, `*.vst3` in tree (except CI artifacts)
- [ ] First `CHANGELOG.md` entry: `[YYYY-MM-DD] Init: project scaffold.`
- [ ] `Tests/DSP_Tests/README.md` — even before tests exist

---

## Strict rules (enforce from commit 1)

1. **No new `.cpp` under `Source/` root** — only `Core/`, `DSP/`, `UI/`, `Script/`.
2. **DSP never includes UI or Editor headers.**
3. **All param IDs in `Core/Parameters.h`** — no raw `"ATTACK"` strings in new code.
4. **Each FX module = one folder** under `DSP/EffectModules/<Name>/`.
5. **CMake lists canonical paths** — not shim forwarders.

---

## Agent / AI standing instruction

Paste into every new feature session:

```text
Decoupled JUCE architecture:
- Pure DSP → Source/DSP/ (EffectModules/<Name>/ for FX)
- Wrapper/APVTS → Source/Core/ (Parameters.h for IDs)
- UI only → Source/UI/
Specify full relative paths for every new file. No new code at Source/ root.
Read ROADMAP.md + CHANGELOG.md before implementing.
```

---

## Softysn1 reference

This repo is the live reference implementation. See [`PROJECT_STANDARDS.md`](../PROJECT_STANDARDS.md) §1.1 and §7–§9 for the enforced rules and migration backlog.
