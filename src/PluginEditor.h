#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

/**
  ProGainAudioProcessorEditor
  ---------------------------
  This is the UI layer. It reads data from the processor and displays it.

  Key ideas:
  - UI runs on a separate thread. It must never touch audio buffers directly.
  - Parameters are connected with APVTS attachments.
  - The meter polls the processor's atomic meterLevel at ~30 FPS.
*/
class ProGainAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
  explicit ProGainAudioProcessorEditor(ProGainAudioProcessor&);
  ~ProGainAudioProcessorEditor() override;

  void paint(juce::Graphics&) override;
  void resized() override;

private:
  ProGainAudioProcessor& processor;

  juce::Slider gainSlider;
  juce::Label gainLabel;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gainAttachment;

  juce::Slider trimSlider;
  juce::Label trimLabel;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> trimAttachment;

  juce::TextEditor presetName;
  juce::TextButton savePresetButton { "Save" };
  juce::TextButton loadPresetButton { "Load" };
  juce::TextButton deletePresetButton { "Delete" };
  juce::ComboBox presetList;

  class MeterComponent;
  std::unique_ptr<MeterComponent> meter;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProGainAudioProcessorEditor)
};
