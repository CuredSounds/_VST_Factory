# Commercial synth gap tracker

Reference: [`features/features.md`](features.md) · Master roadmap: [`ROADMAP.md`](../ROADMAP.md) · Updated 2026-06-27

## ✅ Shipped

| Feature | Reference | Bio-Synth |
|---------|-----------|-----------|
| VA / Wavetable / Granular / Sample engines | All | ✅ ENGINE |
| Chorder + scale quantize | Avenger | ✅ PERFORM |
| Physical modeling (Karplus / Modal) | Falcon | ✅ OSC types |
| Lua parameter automation | Falcon | ✅ SYSTEM → Lua |
| 12 performance macros | All | ✅ MOD → Macros |
| Arpeggiator | Avenger, Falcon | ✅ PERFORM |
| Multi-layer patches + layer mixer | Avenger, Falcon | ✅ A/B/C/D full osc + pan/mute/solo + layer tabs |
| 8-Part Omnisphere-style multi | Omnisphere | ✅ `MultiEngine` + `.softmulti` + per-part FX/ARP |
| LFO direct dest (×4) + WT Position mod | All | ✅ |
| LFO waveform previews + custom draw | Serum | ✅ MOD tabbed LFOs |
| Tabbed LFO 1–4 + MSEG 1–2 | Serum | ✅ MOD |
| Draggable ADSR + MSEG graphs | Serum | ✅ Core + MOD |
| UI scale 75–200% | Falcon | ✅ SYSTEM |
| GUI workspace presets (7) | — | ✅ SYSTEM |
| AU + VST3 + Standalone | — | ✅ |
| AAX (optional build) | Pro Tools | ✅ CMake flag |
| Drag mod matrix (16 slots + curves) | All | ✅ |
| 2× MSEG (4 seg each) | All | ✅ MSEG1 + MSEG2 |
| Dual filter serial/parallel | Spec | ✅ Core panel |
| 4× parallel FX lanes | All | ✅ |
| Wavetable import + editor | Serum | ✅ draw/FFT/resample |
| Columnar OSC hub UI | Avenger, Serum | 🟡 `OscColumnHub` |
| On-screen keyboard | Phase Plant | ✅ PERFORM |
| Drum kit + step sequencer | Avenger | ✅ KIT audio + MIDI fallback |
| Multisample (4 zone buffers) | Falcon | ✅ independent LOAD per zone |
| Batch preset metadata | Avenger | ✅ TAG+ dialog + CAT→ |
| Header preset navigation | Serum | ✅ ◀ ▶ SAVE ☆ |
| Output scope + CPU telemetry | — | ✅ |
| Unison ×8 | Avenger | ✅ |
| Modulation visual overlays | Serum | ✅ knob gold arcs |
| MPE v1 | Falcon | 🟡 toggle + CC74 timbre |
| 2-op FM on OSC hub | Phase Plant | 🟡 `FM_INDEX` / ratio (not 6-op) |
| Factory presets (52+) | — | ✅ KEYS/SEQ categories |

## ❌ Still missing (prioritized)

| Priority | Feature | Reference | Notes |
|----------|---------|-----------|-------|
| P3 | ORB / 8 stereo outs / per-part mod matrix APVTS | Omnisphere | Optional — bank uses ValueTree; see [`OMNISPHERE_ARCHITECTURE.md`](OMNISPHERE_ARCHITECTURE.md) |
| P1 | Factory preset DAW QA | — | 52+ presets — log in `features/todos.md`; see also `docs/issues.md` P0 |
| P0 | Tail length / APVTS literals / per-voice FX / RT alloc | — | `docs/issues.md` → ROADMAP P0 |
| P2 | Step mod as matrix source | All | ✅ `StepModEngine` + MOD panel |
| P2 | FX rack per-effect previews | Serum | No curve previews |
| P2 | 6-op FM / Additive / Spectral | Falcon, Spec | 2-op FM only |
| P2 | Comb / Formant filters · WT warp | Spec | Single SVF/Ladder + dual path |
| P2 | Slice editor / round-robin | Falcon | Scan scaffold only |
| P3 | Full MPE (slide, expression) | Falcon | v1: CC74 + per-channel bend |
| P3 | CLAP format | Spec | CMake flag; JUCE lacks CLAP in tree |
| P3 | Undo/Redo + A/B compare | Spec | Not started |
| P3 | Low CPU eco mode | Spec | Not started |
| P3 | Free modular graph | Phase Plant | Fixed architecture |

## Where to find UI

| Hub | Panel | What |
|-----|-------|------|
| Header | **Preset bar** | ◀ ▶ name SAVE ☆ |
| ENGINE | **OscColumnHub** | OSC A/B/C labels, WT editor, unison, FM/sub |
| ENGINE | **Core** | Draggable ADSR graphs, dual filter, filter env |
| ENGINE | **Layers** | Layer A–D mixer strips (pan/mute/solo) + B/C/D stack |
| ENGINE | **Drum Seq** | Step grid, KIT toggle, built-in drums |
| ENGINE | **Sample Map** | 4 zone LOAD + Vel Lo/Hi + keyboard overlay |
| MOD | **LFO 1–4 tabs** | Shape/trig/dest, custom draw, large preview |
| MOD | **MSEG 1–2 tabs** | Draggable segment graph + segment knobs |
| MOD | **Mod matrix** | 16 slots, drag palette, curve column |
| MOD | **Macros 1–12** | Performance knobs |
| MOD | **Perform** | Arp, chord, scale, keyboard |
| PATCHES | **Browser** | Tags, filters, TAG+ dialog |
| SYSTEM | **Lua console** | Script automation |
| SYSTEM | **Settings** | UI scale, GUI workspace, **MPE** toggle |

## Quick tests

1. **Layers:** ENGINE → Layers → solo B, pan C left, play chord
2. **Mod matrix:** MOD → slot 9, LFO1 → Cutoff, curve Expo
3. **Custom LFO:** LFO 1 tab → shape Custom → draw → route in matrix
4. **Dual filter:** Core → Dual ON, serial, lower F2 cutoff
5. **Drums:** DRUM ON + KIT ON, DAW transport, `Drum_Seq_Demo`
6. **Zones:** Sample Map Z1 vs Z2 different files, narrow Vel Lo/Hi
7. **Presets:** Header ◀ ▶ through `Aurora_LFO_Pad`, `Layer_Stack_Demo`
