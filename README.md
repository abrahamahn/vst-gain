# ProGain — JUCE VST/AU/Standalone Boilerplate

A small, real‑time‑safe JUCE plugin boilerplate focused on clean separation between audio, UI, and infrastructure. The default example is a simple gain/trim plugin with a peak meter and optional SQLite‑backed presets.

## Highlights
- JUCE pulled via CPM (no manual install)
- Plugin formats: VST3, AU, Standalone (AAX optional)
- Real‑time audio thread rules enforced by design
- Beginner‑friendly parameter registry (single source of truth)
- Optional presets stored in SQLite
- Optional integrations: Skia UI backend, GPU Audio SDK

## Requirements
- CMake 3.20+
- C++17 compiler
- Git (for CPM to fetch JUCE)

Optional dependencies (only if enabled):
- SQLite3 development package
- Skia SDK
- GPU Audio SDK
- AAX SDK (for AAX builds)

## Quick Start
Configure and build with CMake:

```bash
cmake -S . -B build
cmake --build build
```

### CMake Options
- `-DUSE_AAX=ON` — enable AAX (requires AAX SDK)
- `-DUSE_SKIA=ON -DSKIA_SDK_PATH=/path/to/skia` — enable Skia UI backend
- `-DUSE_GPU_AUDIO_SDK=ON -DGPU_AUDIO_SDK_PATH=/path/to/sdk` — enable GPU Audio SDK
- `-DUSE_SQLITE=OFF` — disable SQLite presets (enabled by default)

Notes:
- If `USE_SQLITE=ON` but SQLite3 is not found, presets are disabled automatically at configure time.

## Project Structure
```
assets/                 # SVGs, PNGs, fonts, IR samples
cmake/                  # Build logic (CPM, helpers)
docs/                   # Guides: setup, parameters, presets, stack
scripts/                # Deployment/signing scripts (macOS)
src/
  infra/                # Adapters + platform IO (parameters, presets)
  kernel/               # Real‑time DSP core (planned)
  modules/              # Domain logic (planned)
  ui/                   # UI components (planned)
tests/                  # Offline DSP rendering + benchmarks
```

## Parameter System
Parameters live in `src/infra/parameters/ParameterRegistry.*` and are the single source of truth for:
- processor access (`getRawParameterValue()`)
- UI bindings (APVTS attachments)
- presets

See `docs/parameter-system.md` for a beginner‑friendly walkthrough.

## Presets (SQLite)
When `USE_SQLITE=ON`, presets are stored in a single SQLite database file under the user application data directory:
`AbeAudio/ProGain/presets.db`.

See `docs/presets.md` for the schema and API.

## Real‑Time Safety Rules
- No allocation, logging, file I/O, or locks on the audio thread.
- UI work stays on the UI thread.
- Parameters are accessed atomically.

## Documentation
- `docs/setup.md` — build options and setup
- `docs/parameter-system.md` — parameter registry guide
- `docs/presets.md` — SQLite preset storage
- `docs/stack.md` — tech stack overview

## Version
Current version: `0.1.0`

## License
License has not been specified yet.
