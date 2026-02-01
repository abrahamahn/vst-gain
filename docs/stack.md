Audio Plugin Stack
==================

Layer | Technology | Why
---|---|---
Framework | JUCE 8 | Massive ecosystem and best-in-class DAW compatibility.
Build System | CMake + CPM | Zero-install dependency management and repeatable builds.
UI Engine | Skia | GPU-accelerated vector rendering that can hit 120fps.
Math | SIMD (xsimd) | 4–8× faster DSP with portable vectorized code.
AI / Heavy DSP | GPU Audio SDK | Offload expensive processing to the GPU.
Presets | SQLite | Fast, searchable, versionable preset storage.

Notes
-----
- Skia and GPU Audio SDK are optional and enabled via CMake flags.
- SQLite is enabled by default and can be disabled if you prefer a lighter setup.
