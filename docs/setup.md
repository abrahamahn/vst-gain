Setup
=====

CMake options
-------------
- `-DUSE_VST2=ON -DVST2_SDK_PATH=/path/to/VST2_SDK`
- `-DUSE_AAX=ON` (requires AAX SDK)
- `-DUSE_SKIA=ON -DSKIA_SDK_PATH=/path/to/skia`
- `-DUSE_GPU_AUDIO_SDK=ON -DGPU_AUDIO_SDK_PATH=/path/to/sdk`
- `-DUSE_SQLITE=OFF` (disable SQLite presets)

Example configure
-----------------
```
cmake -S . -B build \
  -DUSE_VST2=ON -DVST2_SDK_PATH=/opt/VST2_SDK \
  -DUSE_SKIA=ON -DSKIA_SDK_PATH=/opt/skia \
  -DUSE_GPU_AUDIO_SDK=ON -DGPU_AUDIO_SDK_PATH=/opt/gpu_audio_sdk
```
