VST Boilerplate Implementation Plan
===================================

Goal
----
Build a JUCE-based VST/AU/AAX boilerplate that is solo-dev friendly, real-time
safe, and organized by Kernel/Infra/Domain boundaries.

Non-Negotiables (RT Safety)
---------------------------
- Audio thread: no allocation, no logging, no file I/O, no locks.
- UI thread: rendering and interaction only.
- Parameters: single source of truth using atomic access.

Architecture Summary
--------------------
- Processor (audio thread) is the brain.
- Editor (UI thread) is the face.
- Parameters bridge both threads using APVTS wrappers and atomics.

Folder Layout
-------------
abe-vst-boilerplate/
├── assets/                 # SVGs, PNGs, fonts, IR samples
├── cmake/                  # Build logic (CPM.cmake, static analysis, deploy)
├── docs/                   # API reference + AI integration guide
├── scripts/                # Signing/notarization scripts (macOS)
│
├── src/
│   ├── kernel/             # Hard real-time core
│   │   ├── dsp/            # Filters, osc, gain, envelopes
│   │   ├── types/          # Buffer types, sample rate, tempo
│   │   └── constants.h
│   │
│   ├── infra/              # Adapters + platform IO
│   │   ├── parameters/     # APVTS wrappers
│   │   ├── state/          # Presets and state serialization
│   │   └── ai/             # ONNX/RTNeural adapters
│   │
│   ├── modules/            # Domain logic
│   │   ├── engine/         # Processor orchestration
│   │   └── features/       # Modular DSP blocks
│   │
│   └── ui/                 # Front-end
│       ├── components/     # Knobs, sliders, meters, visuals
│       ├── styles/         # LookAndFeel + theming
│       └── layouts/        # Resizable layout system
│
├── tests/                  # Offline DSP rendering + benchmarks
├── CMakeLists.txt
└── VERSION

Phase 1: Project Skeleton
-------------------------
- Create base JUCE project and CMake entry point.
- Lay out folders and stub modules (kernel/infra/modules/ui).
- Add versioning and basic CI lint/build config.

Phase 2: Core Audio Engine
--------------------------
- Implement processor skeleton with strict RT rules.
- Add kernel DSP primitives (gain, filter, envelope).
- Define shared types (buffer, sample rate, tempo).

Phase 3: Parameter System
-------------------------
- Build APVTS wrappers in infra/parameters.
- Add parameter registry + defaults + automation mapping.
- Create thread-safe value access helpers for audio thread.

Phase 4: UI Framework
---------------------
- Implement custom LookAndFeel and global styles.
- Add ColorPalette struct and font/icon binary data.
- Create reusable UI components and layout helpers.

Phase 5: AI Integration (Optional)
----------------------------------
- Provide ONNX Runtime or RTNeural adapter layer.
- Add ModelLoader and ProcessBlock with buffer <-> tensor
  conversion that never blocks the audio thread.
- Document inference constraints and recommended model sizes.

Phase 6: State + Presets
------------------------
- Implement state serialization (XML or JSON).
- Add preset save/load and safe migration hooks.

Phase 7: Tooling + Deployment
-----------------------------
- Add benchmarking harness for CPU profiling.
- Add lock-free queue for audio -> UI data transfer.
- Write macOS notarization script for AU/VST3.

Implementation Notes
--------------------
- Prefer composition over inheritance for DSP blocks.
- Keep DSP math readable and localized.
- Use offline tests for deterministic DSP validation.
