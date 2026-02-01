cmake_minimum_required(VERSION 3.20)

# CMakeLists.txt
# --------------
# Build entry point for the boilerplate. This file:
# - pulls JUCE via CPM (no manual install)
# - defines plugin formats (VST/VST3/AU/Standalone)
# - wires optional libraries (xsimd, SQLite, Skia, GPU SDK)

project(ProGain
  VERSION 0.1.0
  DESCRIPTION "JUCE VST/AU/Standalone boilerplate"
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(USE_AAX "Enable AAX build (requires AAX SDK)" OFF)
option(USE_SKIA "Enable Skia UI backend (requires Skia SDK)" OFF)
option(USE_GPU_AUDIO_SDK "Enable GPU Audio SDK (requires vendor SDK)" OFF)
option(USE_SQLITE "Enable SQLite preset storage" ON)
set(SKIA_SDK_PATH "" CACHE PATH "Path to Skia SDK (if USE_SKIA=ON)")
set(GPU_AUDIO_SDK_PATH "" CACHE PATH "Path to GPU Audio SDK (if USE_GPU_AUDIO_SDK=ON)")
set(JUCE_VERSION "8.0.0" CACHE STRING "JUCE version tag")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CPM)

CPMAddPackage(
  NAME JUCE
  GITHUB_REPOSITORY juce-framework/JUCE
  GIT_TAG ${JUCE_VERSION}
)

# SIMD math (header-only)
CPMAddPackage(
  NAME xsimd
  GITHUB_REPOSITORY xtensor-stack/xsimd
  GIT_TAG 12.1.1
)

# JUCE recommended globals
set(JUCE_ENABLE_MODULE_SOURCE_GROUPS ON)

set(PLUGIN_FORMATS VST3 AU Standalone)
if(USE_AAX)
  list(APPEND PLUGIN_FORMATS AAX)
endif()

juce_add_plugin(ProGain
  COMPANY_NAME "Abe Audio"
  COMPANY_WEBSITE "https://example.com"
  COMPANY_EMAIL "dev@example.com"
  BUNDLE_ID "com.abeaudio.progain"
  PLUGIN_MANUFACTURER_CODE AbeA
  PLUGIN_CODE PrGn
  FORMATS ${PLUGIN_FORMATS}
  PRODUCT_NAME "Pro Gain"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  BUNDLE_ID "com.abeaudio.progain"
)

juce_generate_juce_header(ProGain)

# Sources
set(SOURCES
  src/PluginProcessor.cpp
  src/PluginProcessor.h
  src/PluginEditor.cpp
  src/PluginEditor.h
  src/infra/parameters/ParameterRegistry.cpp
  src/infra/parameters/ParameterRegistry.h
  src/infra/state/PresetStore.cpp
  src/infra/state/PresetStore.h
)

target_sources(ProGain PRIVATE ${SOURCES})

set(USE_SQLITE_EFFECTIVE ${USE_SQLITE})

target_link_libraries(ProGain
  PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    xsimd
  PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
    juce::juce_recommended_lto_flags
)

if(USE_SQLITE)
  find_package(SQLite3 QUIET)
  if(NOT SQLite3_FOUND)
    message(WARNING "SQLite3 not found. Disabling SQLite presets for now.")
    set(USE_SQLITE_EFFECTIVE OFF)
  else()
    target_include_directories(ProGain PRIVATE ${SQLite3_INCLUDE_DIRS})
    target_link_libraries(ProGain PRIVATE ${SQLite3_LIBRARIES})
  endif()
endif()

target_compile_definitions(ProGain
  PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    $<$<BOOL:${USE_SQLITE_EFFECTIVE}>:USE_SQLITE=1>
    $<$<BOOL:${USE_SKIA}>:USE_SKIA=1>
    $<$<BOOL:${USE_GPU_AUDIO_SDK}>:USE_GPU_AUDIO_SDK=1>
)

if(USE_SKIA)
  if(NOT SKIA_SDK_PATH)
    message(FATAL_ERROR "USE_SKIA=ON requires SKIA_SDK_PATH to be set.")
  endif()
  target_include_directories(ProGain PRIVATE "${SKIA_SDK_PATH}/include")
  target_link_directories(ProGain PRIVATE "${SKIA_SDK_PATH}/lib")
  # Link your Skia libs here once finalized.
endif()

if(USE_GPU_AUDIO_SDK)
  if(NOT GPU_AUDIO_SDK_PATH)
    message(FATAL_ERROR "USE_GPU_AUDIO_SDK=ON requires GPU_AUDIO_SDK_PATH to be set.")
  endif()
  target_include_directories(ProGain PRIVATE "${GPU_AUDIO_SDK_PATH}/include")
  target_link_directories(ProGain PRIVATE "${GPU_AUDIO_SDK_PATH}/lib")
  # Link vendor GPU audio libs here once finalized.
endif()
