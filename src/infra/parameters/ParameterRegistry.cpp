/**
  ParameterRegistry.cpp
  ---------------------
  Single source of truth for parameter metadata.

  Web-dev analogy:
  - Think of this like a JSON schema for your plugin's state.
  - UI, audio, and presets all read from this list so they stay in sync.
*/
#include "ParameterRegistry.h"
#include <cstring>

namespace params
{
namespace
{
const std::vector<ParamSpec> kParams = {
  {
    "gain",
    "Gain",
    0.0f,
    2.0f,
    0.001f,
    0.5f,
    1.0f,
    "x",
    0.02f
  },
  {
    "trim",
    "Output Trim",
    -12.0f,
    12.0f,
    0.01f,
    1.0f,
    0.0f,
    "dB",
    0.02f
  }
};
}

const std::vector<ParamSpec>& getAll()
{
  return kParams;
}

juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
  params.reserve(kParams.size());

  for (const auto& spec : kParams)
  {
    auto range = juce::NormalisableRange<float>(spec.min, spec.max, spec.step, spec.skew);
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
      spec.id,
      spec.name,
      range,
      spec.defaultValue,
      spec.unit
    ));
  }

  return { params.begin(), params.end() };
}

const ParamSpec* find(const char* id)
{
  for (const auto& spec : kParams)
  {
    if (std::strcmp(spec.id, id) == 0)
      return &spec;
  }
  return nullptr;
}
}
