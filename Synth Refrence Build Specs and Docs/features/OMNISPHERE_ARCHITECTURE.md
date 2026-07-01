# Omnisphere-style Multi (8 Parts) + A/B/C/D Layers

**Target parity:** Spectrasonics Omnisphere 2 hierarchy (see [`Untitled 11.png`](Untitled%2011.png)).

**Status:** **Phase 3 complete (Sprint 11+)** — 8-part multi with per-part editor scope, FX/ARP, layer signal path, `.softmulti` save/load, layer tabs UI.

---

## Implemented (2026-06-27)

| Component | Path | Notes |
|-----------|------|-------|
| `PartParameterMatrix` | `Source/DSP/Multi/PartParameterMatrix.h` | 4× `LayerParameterBlock` + shared `VoiceParameterMatrix patch` |
| `LayerParamBridge` | `Source/DSP/Multi/LayerParamBridge.h` | `applyLayerToPatchMatrix`, `syncPart0LayersFromPatch` |
| `PartPatchBank` | `Source/DSP/Multi/PartPatchBank.h` | Per-part APVTS + FX + ARP snapshots; stash/restore on part switch |
| `MultiPresetManager` | `Source/Core/MultiPresetManager.*` | `.softmulti` save/load (8 parts + master gain + voice tier) |
| `MultiEngine` | `Source/DSP/Multi/MultiEngine.h` | 8× synth + per-part FX/ARP in render path |
| `VoiceQualityTier` | `Source/DSP/Multi/VoiceQualityTier.h` | Eco 4 / Mid 8 / High 16 / Ultra 32 voices **per part** |
| `SynthVoice` 4-layer | `Source/SynthVoice.cpp` | Per-layer filters when signal path ≠ shared; shared filter on mix |
| APVTS | `PluginProcessor::createParameterLayout` | `P{n}_*`, `P{n}_LAY{x}_*`, `P{n}_SIGPATH`, `MULTI_*`, `VOICE_QUALITY_TIER` |
| UI | `PartSelectorBar`, `MultiMixerPanel`, `LayerTabsPanel` | Part 1–8, multi mixer, A/B/C/D + signal path + layer FLT |

**Part 1 compat:** Legacy `OSC_TYPE`, `LAYER_A/B/C/D_*` still drive Part 0 via `syncPart0LayersFromPatch` when editing Part 1. Parts 2–8 use stashed `P{n}_LAY{x}_*` via `PartPatchBank`.

**Deferred (optional):** ORB/soundsource browser, 8 stereo outs, full 8×16 mod matrix APVTS duplication (bank uses ValueTree per part).

---

## Hierarchy (what we're building)

```
MultiPatch  ("default multi" — saved/loaded as .softmulti or APVTS child tree)
│
├── MultiMixer     part level/pan/mute/solo, key splits, MIDI channel per part
│
├── Part 1 … Part 8   (each = independent synth patch)
│   ├── PartSettings   voices, octave, gain, scale, signal path, latch/trigger
│   ├── Layer A        full osc + (optional) filter/env slice
│   ├── Layer B
│   ├── Layer C
│   ├── Layer D
│   ├── Part FX        insert/send (or route to shared 4-lane rack — TBD)
│   └── Part ARP       per-part arp (optional phase 3)
│
└── Shared resources   wavetable blobs, sample zones (reference-counted per part)
```

**Today:** 8 Parts render in parallel via `MultiEngine`; each Part voice runs **4 identical layer chains** with per-layer or shared filter routing. Per-part FX/ARP and editor focus via `PartPatchBank` + `switchActivePart`.

**Target (optional):** ORB browser, 8 stereo outs, dedicated mod matrix per part in APVTS.

---

## Phased delivery

### Phase 1 — Data model + Part 1 deep layers (Sprint 8) ✅

| Task | Outcome |
|------|---------|
| `MultiPatchState` / `SynthPartState` / `PartLayerState` | Typed state in `Source/DSP/Multi/` |
| APVTS prefix scheme `P{n}_LAY{x}_*` | Parameter IDs for 8×4 layers (Part 1 aliases current params) |
| Upgrade Layer A | Primary osc = Layer A via `syncPart0LayersFromPatch` |
| Upgrade B/C/D | Full osc parity via `renderLayerAudio` loop |
| Part bar + multi mixer UI | `PartSelectorBar`, `MultiMixerPanel` |
| Voice tiers | Eco/Mid/High/Ultra per part |

**Exit criteria:** ✅ One part sounds as before; B/C/D can use any osc type; code ready for Part 2–8.

### Phase 2 — 8 Parts audible (Sprint 9–10) ✅

| Task | Outcome |
|------|---------|
| `MultiEngine` | ✅ 8× `juce::Synthesiser` |
| Part selector UI | ✅ Top bar `1…8` + `MULTI` |
| Multi mixer UI | ✅ Part level/pan/mute/solo + master gain |
| MIDI routing | ✅ `PartSynthSound` channel + key range |
| Multi preset save/load | ✅ `.softmulti` via `MultiPresetManager` |

**Exit criteria:** ✅ Stack 8 different patches; solo/mute parts; key split; multi save/load.

### Phase 3 — Omnisphere depth (Sprint 11+) ✅

| Task | Outcome |
|------|---------|
| Per-layer signal path | ✅ Shared / per-layer / serial (`P{n}_SIGPATH`, layer `FLT_*`) |
| Per-part FX + ARP | ✅ `partFxRacks[8]`, `partArpeggiators[8]` in render + `PartPatchBank` |
| Part-focused editor | ✅ `switchActivePart` stash/restore APVTS + FX + ARP |
| Layer tabs UI | ✅ `LayerTabsPanel` (A/B/C/D + signal path + layer filter) |
| Multi preset | ✅ `MultiPresetManager` + UI in `MultiMixerPanel` |
| Part-level mod matrix | 🟡 Active-part matrix via APVTS; inactive parts in ValueTree bank |
| ORB / soundsource browser | ⬜ Deferred |
| Multi audio outputs | ⬜ Deferred |

---

## Parameter naming (APVTS)

```
MULTI_NAME
PART{n}_ON          PART{n}_LEVEL   PART{n}_PAN   PART{n}_MUTE   PART{n}_SOLO
PART{n}_MIDI_CH     PART{n}_KEY_LO   PART{n}_KEY_HI
PART{n}_VOICES      PART{n}_OCTAVE   PART{n}_GAIN
PART{n}_LAY{A|B|C|D}_ON   _OSC   _SEMI   _WT_POS   _LEVEL   _PAN   _MUTE   _SOLO
```

`n` = 0..7 (UI shows 1..8). Part 0 layer params **alias** existing `LAYER_B_*` / `OSC_TYPE` where possible for preset backward compatibility.

---

## DSP integration sketch

```cpp
// processBlock (future)
multiEngine.processBlock (buffer, midi, apvts, sharedAssets);

// Each SynthPart owns:
//   - VoiceParameterMatrix slice (or embedded copy)
//   - 4× LayerVoice path inside SynthVoice OR 4× mini-voice render
//   - Part-local arp state
```

**CPU note:** 8 full parts × 16 voices worst-case = 128 voices. Ship with **eco defaults** (e.g. 8 voices/part, 4 active parts) and telemetry warning.

---

## UI map (Omnisphere → Bio-Synth)

| Omnisphere | Bio-Synth (target) |
|------------|-------------------|
| `1–8` + `MULTI` | New top bar on `TabbedSynthUI` |
| `MAIN` | ENGINE → Core + global part strip |
| `A B C D` | ENGINE → Layers (4 tabs, not just mixer strips) |
| `FX` | ENGINE → FX Rack (scoped to part) |
| `ARP` | PERFORM (scoped to part) |
| `PATCHES` | PATCHES (part patch vs multi patch filter) |
| `PART LEVEL` M/S | Layers panel + Multi mixer |

Reference screenshot: [`features/Untitled 11.png`](Untitled%2011.png)

---

## Migration from current LayerStack

| Current | Multi target |
|---------|--------------|
| `LayerStack.h` B/C/D only | All 4 layers A–D in `PartLayerState` |
| `OscillatorModule` = “main” | Layer A engine; B/C/D upgraded to mini `OscillatorModule` or shared render path |
| Single `voiceMatrix` | `std::array<SynthPartState, 8>` with Part 0 mirroring legacy matrix |
| `LayersPanel` | Split: **Layer tabs A–D** + **Multi mixer** view |

---

## Open decisions

1. **8× Synthesiser vs pooled voices** — start with 8× synth (simpler, heavier CPU).
2. **Shared vs per-part wavetable/sample** — reference-counted assets in processor.
3. **One APVTS vs nested ValueTree per part** — nested tree scales better for multi save.
4. **GUI collaborator** — Part bar + MULTI view in `UI_UX_PLAN` Phase D.

---

## Related docs

- [`ROADMAP.md`](../ROADMAP.md) — Sprint 8+ flagship
- [`features/GAPS.md`](GAPS.md) — parity tracker
- [`features/features.md`](features.md) — Falcon / Omnisphere row
