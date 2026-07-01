# Architecture: Softysn1 Hybrid Synthesizer

## 1. Overview
Softysn1 is designed as a multi-engine hybrid synthesizer, combining Virtual Analog, Wavetable, Granular, and Sample-based synthesis. The goal is to provide a flexible, high-performance audio engine with a deeply routable modulation system.

## 2. Core Components

### 2.1 Audio Engine (The Core)
- **Voice Manager**: Handles voice allocation, polyphony (up to 64-128 voices), and voice lifecycle (Note On/Off, Release).
- **Oscillator Layers**: 
  - Each voice can contain multiple "Layers".
  - Layer types: Virtual Analog (VA), Wavetable, Granular, Sampler.
- **Filter Section**: Multi-mode filters (Ladder, SVF, Formant) per voice and per layer.
- **FX Rack**: Global and per-layer effects (Reverb, Delay, Chorus, Distortion, etc.).

### 2.2 Modulation System
- **Modulation Matrix**: A flexible many-to-many routing system.
- **Modulators**:
    - Envelopes (ADSR, MSEG).
    - LFOs (Syncable, customizable waveforms).
    - Step Sequencers.
    - MIDI CC / Velocity / Aftertouch.

### 2.3 User Interface (UI)
- High-fidelity vector graphics (likely using JUCE's Graphics class or an OpenGL/Vulkan backend).
- Real-time visualizations (Spectrogram, Oscilloscope, Wavetable 3D view).
- Drag-and-drop modulation assignment.

### 2.4 Data Management
- **Preset System**: JSON or XML-based preset storage.
- **Sample/Wavetable Management**: Efficient loading and streaming of audio assets.

## 3. Technical Stack
- **Language**: C++20
- **Framework**: JUCE (industry standard for VST3/AU/AAX)
- **DSP Libraries**: Custom DSP + JUCE DSP module.
- **Build System**: CMake

## 4. Source layout (Core / DSP / UI)

The codebase separates **wrapper**, **engine**, and **UI** so DSP can be unit-tested and reused outside JUCE.

| Layer | Path | Responsibility |
|-------|------|----------------|
| **Wrapper** | `Source/Core/` | `AudioProcessor`, APVTS layout, `PresetManager`, `Parameters.h` |
| **Engine** | `Source/DSP/` | Voice, oscillators, filters, mod matrix, `EffectModules/` |
| **UI** | `Source/UI/` | Look-and-feel, panels, widgets — no audio processing |
| **Tests** | `Tests/DSP_Tests/` | Headless tests linking only `Source/DSP/` |

### Multi-FX modules

Each effect inherits from `EffectModule` and lives in its own folder under `Source/DSP/EffectModules/`:

- `Compressor/`, `Delay/`, `Reverb/`, `Chorus/`, `EQ/`, `Distortion/` (saturation)

`EffectModuleFactory` in `Source/DSP/EffectModule.cpp` registers types for the modular FX rack.

### Parameter IDs

All APVTS string IDs are centralized in `Source/Core/Parameters.h` (`Softysn1::Params` namespace) to prevent automation mismatches from typos.

### Shim headers

Legacy includes at `Source/PluginProcessor.h`, `Source/DSP/CompressorModule.h`, etc. forward to canonical paths during migration. New code should include from `Core/` or `EffectModules/` directly.

See also: [`docs/FOLDER_STRUCTURE.md`](docs/FOLDER_STRUCTURE.md), [`PROJECT_STANDARDS.md`](PROJECT_STANDARDS.md) §1.1.
