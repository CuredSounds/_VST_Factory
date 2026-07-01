# UI/UX Improvement Plan — Bio-Synth GUI

## Context

The user reviewed the current GUI and flagged three friction points after comparing to Serum/Nexus screenshots (in `features/serum*.png`, `features/nexus 1.png`):

1. **LFO custom drawing dominates the screen.** The modulation tab uses a mutually-exclusive 6-tab system (LFO 1–4, MSEG 1–2); only one is visible at a time, and the waveform view always claims 200 px / 50% of panel height — even when the user picked a preset shape (Sine, Saw, etc.) and doesn't need to draw anything. User intent: see 2–3 LFOs simultaneously at rest; expand to full-screen only when drawing a Custom shape.
2. **Knob sizes are wildly inconsistent across panels.** FX rack jams 10 knobs into a 100 px row (~110 px wide, 1.1:1 aspect → too squat); ADSR uses 240×88 (2.7:1 → too wide); LayersPanel mixer knobs are huge after the vertical-strip refactor. There is no `kStandardKnobSize` constant. `BioSynthUI::layoutKnobsInRow` distributes evenly with no size floor or ceiling.
3. **OSC layout feels off.** `OscColumnHub` *is* horizontal (3 columns side-by-side), but within each column the wavetable editor stacks above knobs — so each column reads as a tall vertical strip. Commercial synths (Serum reference: `serum osc.png`) put a single large WT display on the left with knobs in a horizontal row to the right of it, not stacked below each oscillator.

Goal: a focused UI/UX refactor that fixes these three issues without rewriting the panel architecture.

---

## Recommended approach (phased, smallest blast radius first)

### Phase A — Knob standardization (foundation; everything else depends on it)

**File to modify:** `Source/UI/BioSynthUI.h`

Add size constants and an updated layout helper:

```cpp
namespace BioSynthUI {
    constexpr int kKnobS = 48;   // FX rack, sample zones, dense rows
    constexpr int kKnobM = 64;   // OSC, modulation, performance — default
    constexpr int kKnobL = 88;   // ADSR, headline macros, filter env

    // New: layout that caps knob width to a target size + centres them in the row.
    // Falls back to even distribution only if row is too narrow for the target.
    void layoutKnobsInRowSized (juce::Rectangle<int> row,
                                juce::Slider** knobs, int count,
                                int targetKnobW, int topPad = 14);
}
```

**Why:** `layoutKnobsInRow` currently divides `rowWidth / count` with no ceiling, which is why ADSR (4 knobs in ~1100 px) gets 2.7:1 aspect and FX rack (10 knobs in 1100 px) gets 1.1:1. A sized variant fixes both.

**Panel updates (mechanical):**
- `FXRackPanel::resized` → use `kKnobS` (48) for the master FX row of 10. Will need to wrap to 2 rows of 5 in narrower windows.
- `OscColumnHub::resized` → use `kKnobM` (64) per column.
- `ModulationPanel::resized` (LFO knob row Rate/Depth/Phase) → use `kKnobM` (64).
- `SynthesisCorePanel::resized` (ADSR + Filter + FilterEnv rows) → use `kKnobL` (88).
- `LayersPanel::layoutMixerColumn` → cap each knob to `kKnobM` (currently knobs are full column width since the vertical strip refactor).
- `PerformPanel`, `SettingsPanel`, `OscOrganismsPanel`, `SampleMapPanel`, `DrumSequencerPanel` → `kKnobM`.

Keep the original `layoutKnobsInRow` for backward compatibility, but route all call sites through the sized variant.

---

### Phase B — LFO compact view + expand-to-draw editor

**Files to modify:**
- `Source/UI/Panels/ModulationPanel.cpp` and `.h` — replace the 6-tab system with a stacked compact list
- `Source/UI/WaveformDisplay.h` (no DSP changes; just confirm `LfoCustomDrawView` works at small sizes — it already scales)

**New layout (replaces current tab system):**

```
┌─ MOD ──────────────────────────────────────────────┐
│ ┌─ LFO 1 ──────────────────────────────────────┐  │
│ │ [shape ▼][trig ▼][dest ▼]  ⌇⌇⌇⌇⌇  rate dep ph[✎]│  
│ └──────────────────────────────────────────────┘  │
│ ┌─ LFO 2 ──────────────────────────────────────┐  │
│ │ ...                                           [✎]│
│ └──────────────────────────────────────────────┘  │
│ ┌─ LFO 3, LFO 4, MSEG 1, MSEG 2 (same compact)─┐  │
│ └──────────────────────────────────────────────┘  │
└────────────────────────────────────────────────────┘
```

- Compact row height: **~72 px** (small shape preview ~60×56 + combos + 3 mini knobs + edit button)
- All 6 rows stack vertically; the panel scrolls if needed (use `juce::Viewport` wrapping a content component)
- The `[✎]` (Edit) button is only enabled when shape == Custom (index 5)
- Clicking `[✎]` opens a modal `LfoDrawWindow` (`juce::DialogWindow` or `juce::CallOutBox`) hosting a full-size `LfoCustomDrawView` (~480×320) — this is where 16-point drawing happens

**New class needed:** `LfoCompactRow` (private nested class in `ModulationPanel`) — owns the compact controls per LFO/MSEG. Replaces the existing `LfoPage` and `MsegPage` private classes.

**Existing code to reuse:**
- `BioWaveform::LfoCustomDrawView` at `Source/UI/WaveformDisplay.h:319` — already handles both preview and edit modes; works fine at any size. Use a small instance in the compact row, a full instance in the modal.
- `LfoCustomTable.h::kLfoCustomPoints = 16` — no change needed.
- All APVTS bindings (`LFO*_SHAPE`, `LFO*_RATE`, `LFO*_W0..15`) — no change needed; the modal writes to the same parameters the compact preview reads.

**Tab buttons removed.** This is a structural simplification — the 28 px tab row goes away, freeing vertical space for 6 compact rows.

---

### Phase C — OSC layout: Serum-style flat horizontal (proposed; flag for confirmation)

**File to modify:** `Source/UI/Panels/OscColumnHub.cpp` and `.h`

Reference: `features/serum osc.png` — Serum keeps the WT display **wide and central** with knobs as a **horizontal row underneath**, not stacked per-oscillator.

Proposed structure:
```
┌─ OSC HUB ──────────────────────────────────────────┐
│ ┌──── WT Editor (~60% width, full height) ────┐ ┌─ OSC B (sync/ring) ─┐
│ │                                              │ │ shape combo         │
│ │     (large waveform display + editor)        │ │ ratio detune ring   │
│ │                                              │ └─────────────────────┘
│ │                                              │ ┌─ OSC C (FM) ────────┐
│ └──────────────────────────────────────────────┘ │ algo combo          │
│ ┌─ OSC A controls — horizontal knob row ──────┐ │ index ratio fb      │
│ │ type ▼  WT_POS  Warp  Unison  Spread  Det   │ └─────────────────────┘
│ └──────────────────────────────────────────────┘ ┌─ Sub ─┐ ┌─ Noise ─┐
│                                                   └───────┘ └─────────┘
└────────────────────────────────────────────────────┘
```

- WT editor + OSC A live on the left (large WT display + horizontal knob row beneath it)
- OSC B (sync/ring mod) and OSC C (FM) stack as compact panels on the right
- Sub + Noise stay in their existing narrow columns at the bottom right

**Why "flag for confirmation":** the current `OscColumnHub` is already horizontal (3 main columns side-by-side); the complaint reads more like "the WT editor and stacked knobs make each column feel vertical." The Serum-style restructure above resolves that, but it's a larger change than Phase A/B and you may prefer just standardising knob sizes within the existing column structure first.

**Open question:**
- **C1:** Restructure to Serum-style flat horizontal (proposed above)?
- **C2:** Keep existing 3-column structure but apply Phase A knob sizing?
- **C3:** Review one of `features/Untitled*.png` design mockups first to match a specific vision?

---

## Files & functions touched

| File | Change |
|------|--------|
| `Source/UI/BioSynthUI.h` | Add `kKnobS/M/L` constants, add `layoutKnobsInRowSized` |
| `Source/UI/Panels/FXRackPanel.cpp` | Use `kKnobS`; consider 2-row layout for 10 knobs |
| `Source/UI/Panels/OscColumnHub.cpp` | Use `kKnobM` (Phase A); restructure layout (Phase C if approved) |
| `Source/UI/Panels/ModulationPanel.cpp/.h` | Replace tabs with compact stack; add `LfoCompactRow`; add `LfoDrawWindow` modal |
| `Source/UI/Panels/SynthesisCorePanel.cpp` | Use `kKnobL` |
| `Source/UI/Panels/LayersPanel.cpp` | Cap mixer knobs to `kKnobM` |
| `Source/UI/Panels/PerformPanel.cpp` | Use `kKnobM` |
| `Source/UI/Panels/SettingsPanel.cpp` | Use `kKnobM` |
| `Source/UI/Panels/OscOrganismsPanel.cpp` | Use `kKnobM` |
| `Source/UI/Panels/SampleMapPanel.cpp` | Use `kKnobS` (dense zone strips) |
| `Source/UI/Panels/DrumSequencerPanel.cpp` | Use `kKnobM` |

Existing utilities to reuse (no duplication):
- `BioWaveform::LfoCustomDrawView` at `Source/UI/WaveformDisplay.h:319` — supports both preview and edit modes, scales to any size
- `BioSynthUI::styleKnob`, `styleCombo`, `styleToggle` — keep using
- `BioSynthUI::SectionHeader` — keep using for compact row headers
- `juce::DialogWindow` / `juce::CallOutBox` — built-in JUCE modal patterns for the LFO draw editor

---

## Verification

After implementation:

1. **Build:** `cmake --build cmake-build-debug --target Softysn1_Standalone` (verify no compile errors)
2. **Launch standalone:** open `_BC_softsyn1.app` and visually inspect:
   - **MOD tab:** all 4 LFOs + 2 MSEGs visible simultaneously without scrolling at 1200×800; clicking ✎ on an LFO with shape=Custom opens a large draw window; preset-shape LFOs hide ✎
   - **ENGINE / OSC tab:** knobs visibly consistent in size; no extreme aspect ratios
   - **MIXER:** vertical strips with reasonably sized knobs
   - **FX:** master FX row knobs no longer cramped (either smaller or wrapped to 2 rows)
3. **Load factory presets** that exercise LFOs (e.g., `Aurora_LFO_Pad`, `MSEG_Swell_Pad`) — confirm shape preview renders correctly in compact rows
4. **Draw custom LFO:** open Edit modal, drag points, close → confirm changes persist in the compact preview (APVTS write path unchanged so this should "just work")
5. **Window resize:** drag the plugin window to 900×600 and 1600×1000 — confirm layouts adapt without overlap
6. **Per-effect FX preview** (recently shipped) still renders inside slots — visual sanity check

No new tests required (UI changes); existing factory preset DAW listening pass in Sprint 6 will catch any regressions.

---

## Open questions for plan approval

1. **OSC restructure (Phase C):** C1 / C2 / C3 above? Default if no answer: **C2** (keep columns, standardize knobs only — smallest change).
2. **LFO compact row layout:** stacked vertical list (proposed) vs. 2×3 grid? Default: **stacked list** (matches "see 2–3 LFOs" phrasing better than a grid).
3. **Scope:** ship all 3 phases at once, or land Phase A first, then B, then C? Default: **A → B → C in separate commits** for reviewability.
