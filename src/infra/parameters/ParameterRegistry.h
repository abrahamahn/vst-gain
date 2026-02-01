#pragma once

#include <JuceHeader.h>
#include <string>
#include <vector>

/**
  ParameterRegistry
  -----------------
  Central place to define all parameters once.

  Web-dev analogy:
  - This is like a typed schema for your plugin's "config" object.
  - The UI, audio thread, and preset system all pull from this one source.
*/
namespace params
{
struct ParamSpec
{
  const char* id;
  const char* name;
  float min;
  float max;
  float step;
  float skew;
  float defaultValue;
  const char* unit;
  float smoothingSeconds; // UI/processor default smoothing time
};

const std::vector<ParamSpec>& getAll();

juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

const ParamSpec* find(const char* id);
}
