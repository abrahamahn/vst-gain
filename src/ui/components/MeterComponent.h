#pragma once

#include <JuceHeader.h>
#include "../../PluginProcessor.h"

/**
  MeterComponent
  --------------
  Vertical level meter component that polls the processor's atomic peak value
  on a 30 Hz UI timer and renders a smoothed level bar.
*/
class MeterComponent : public juce::Component, private juce::Timer
{
public:
  explicit MeterComponent(ProGainAudioProcessor& proc);
  ~MeterComponent() override = default;

  void paint(juce::Graphics& g) override;

private:
  void timerCallback() override;

  ProGainAudioProcessor& processor;
  juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> meterSmoothed;
  float currentLevel { 0.0f };

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MeterComponent)
};
