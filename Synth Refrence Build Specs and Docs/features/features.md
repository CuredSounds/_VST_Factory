# Competitive feature targets

Status key: **✅** shipped · **🟡** partial / v1 · **❌** not started

Reference synths: VPS Avenger · Phase Plant · UVI Falcon

---

## 1. VPS Avenger

| Area | Target | Bio-Synth status |
|------|--------|------------------|
| **Synthesis** | VA, Wavetable, Granular, Sample, Drum, Resample | **✅** VA/WT/Granular/Sample · **✅** Drum (kit + step seq) · **✅** WT resample from sample |
| **Architecture** | Tab-based, up to 8 OSCs per patch | **✅** Tab hubs · **🟡** OSC A/B/C + SUB + layers B/C/D + unison×8 (not 8 fully independent columns) |
| **Samples** | Multisample, Granular, Import/Convert | **✅** Import · **✅** 4 independent zone buffers · **✅** Granular · **🟡** slice scan scaffold |
| **Wavetable** | Custom, FFT, Draw, Resample | **✅** Import · **✅** Draw/Smooth/Norm · **✅** FFT · **✅** Resample |
| **Modulation** | Drag matrix, 12 macros, zones, LFOs/ENV/seq | **✅** 16-slot drag matrix + curves · **✅** 12 macros · **✅** key zones · **✅** 4 LFOs + custom draw · **✅** 2× MSEG (4 seg) · **❌** step mod seq |
| **FX / Perform** | 4 racks, sends, drum kits, chorder, FFT morph | **✅** 4 parallel FX lanes · **✅** Chorder + scale quantize · **✅** Drum kit + seq · **❌** unique FX (Ruta Verb, etc.) |
| **Workflow** | Scalable GUI, tag librarian, batch convert | **✅** UI 75–200% · **✅** PATCHES browser + tags · **✅** batch TAG+ dialog · **✅** header preset bar |

---

## 2. Phase Plant

| Area | Target | Bio-Synth status |
|------|--------|------------------|
| **Synthesis** | Analog, WT, Granular, Sample, Noise | **✅** (noise via osc type) |
| **Architecture** | Fully modular, up to 32 modules | **❌** Fixed signal path (not free modular graph) |
| **Samples** | Granular, sample player, import | **✅** |
| **Wavetable** | 256-frame, import/edit, editor | **🟡** Import + editor (frame count varies) · **✅** editor panel |
| **Modulation** | Unlimited LFOs, audio-rate FM, poly mod | **🟡** 4 LFOs + custom draw · **🟡** 2-op FM on OSC hub · **❌** audio-rate poly mod bus |
| **FX / Perform** | Snapin lanes, poly FX, on-screen keyboard | **✅** 4 FX lanes · **❌** snapin ecosystem · **✅** PERFORM keyboard |
| **Workflow** | Visual patching, macros | **✅** Macros · **❌** visual patch cables |

---

## 3. UVI Falcon

| Area | Target | Bio-Synth status |
|------|--------|------------------|
| **Synthesis** | Analog, WT, FM, Additive, Granular, Sample, PM, Drum, … | **🟡** VA/WT/Granular/Sample · **✅** Karplus + Modal PM · **🟡** 2-op FM · **❌** additive/spectral/VOSIM |
| **Architecture** | Layered, unlimited OSCs, Multi/Part/Layer/Keygroup | **🟡** Layer A + B/C/D + mixer · **🔜** 8-Part Multi planned — [`OMNISPHERE_ARCHITECTURE.md`](OMNISPHERE_ARCHITECTURE.md) |
| **Samples** | 7+ sampling OSCs, slicing, advanced map | **🟡** 4 zones · **🟡** slice scan · **❌** slice editor / round-robin |
| **Wavetable** | Import audio/image, morph, editor | **✅** audio import · **❌** image import · **✅** morph + editor |
| **Modulation** | Multi-level, unlimited sources, LUA | **✅** 16 matrix slots + curves · **✅** LUA console · **❌** multi-level keygroup mod |
| **FX / Perform** | 100+ FX, event processors, MPE | **🟡** ~8 FX types × 4 lanes · **✅** dual filter · **✅** Arp · **🟡** MPE v1 |
| **Workflow** | Tree views, tags, custom banks | **✅** PATCHES + tags · **✅** 52+ factory presets |

---

## Summary scorecard

| Synth | Rough parity |
|-------|----------------|
| Avenger | **~72%** — engines, mod depth, drum kit, multisample zones, librarian |
| Phase Plant | **~42%** — WT editor + keyboard + custom LFO; gap: modular graph |
| Falcon | **~52%** — layers + mixer + LUA + PM + MSEG; gap: engine breadth + full MPE |

See **`ROADMAP.md`** (repo root) for sprint todos and **`features/GAPS.md`** for gap detail.
