/**
  PluginEditor.cpp
  ----------------
  Implements the UI:
  - A rotary gain knob bound to the parameter system.
  - A simple vertical meter that reads a peak value from the processor.
*/
#include "PluginEditor.h"
#include "infra/state/PresetStore.h"
#include <string>

namespace
{
constexpr const char* kParamGainId = "gain";
constexpr const char* kParamTrimId = "trim";
}

class ProGainAudioProcessorEditor::MeterComponent : public juce::Component, private juce::Timer
{
public:
  explicit MeterComponent(ProGainAudioProcessor& proc)
    : processor(proc)
  {
    // Smoothing for the visual meter (not audio).
    meterSmoothed.reset(30.0, 0.15);
    meterSmoothed.setCurrentAndTargetValue(0.0f);
    startTimerHz(30);
  }

  void paint(juce::Graphics& g) override
  {
    // Draw background.
    auto bounds = getLocalBounds().toFloat();

    g.setColour(juce::Colours::black.withAlpha(0.7f));
    g.fillRoundedRectangle(bounds, 6.0f);

    // Draw filled meter.
    const float level = juce::jlimit(0.0f, 1.0f, currentLevel);
    auto fill = bounds;
    fill.removeFromTop(bounds.getHeight() * (1.0f - level));

    juce::Colour meterColour = juce::Colour::fromRGB(64, 196, 92);
    if (level > 0.85f)
      meterColour = juce::Colour::fromRGB(232, 98, 78);
    else if (level > 0.65f)
      meterColour = juce::Colour::fromRGB(232, 178, 62);

    g.setColour(meterColour);
    g.fillRoundedRectangle(fill, 6.0f);

    g.setColour(juce::Colours::white.withAlpha(0.15f));
    g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
  }

private:
  void timerCallback() override
  {
    // Poll the processor's atomic meter and animate smoothly.
    const float target = processor.getMeterLevel();
    meterSmoothed.setTargetValue(target);
    currentLevel = meterSmoothed.getNextValue();
    repaint();
  }

  ProGainAudioProcessor& processor;
  juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> meterSmoothed;
  float currentLevel { 0.0f };
};

ProGainAudioProcessorEditor::ProGainAudioProcessorEditor(ProGainAudioProcessor& p)
  : AudioProcessorEditor(&p), processor(p)
{
  // Basic layout size.
  setSize(520, 300);

  // Gain knob styling.
  gainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
  gainSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour::fromRGB(64, 196, 92));
  gainSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour::fromRGB(32, 40, 46));
  gainSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
  addAndMakeVisible(gainSlider);

  // Label below the knob.
  gainLabel.setText("Gain", juce::dontSendNotification);
  gainLabel.setJustificationType(juce::Justification::centred);
  gainLabel.setColour(juce::Label::textColourId, juce::Colours::white);
  addAndMakeVisible(gainLabel);

  // Wire the knob to the APVTS parameter.
  gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
    processor.getAPVTS(),
    kParamGainId,
    gainSlider
  );

  // Trim knob styling (mirrors gain).
  trimSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  trimSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
  trimSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour::fromRGB(232, 178, 62));
  trimSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour::fromRGB(32, 40, 46));
  trimSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
  addAndMakeVisible(trimSlider);

  trimLabel.setText("Output Trim", juce::dontSendNotification);
  trimLabel.setJustificationType(juce::Justification::centred);
  trimLabel.setColour(juce::Label::textColourId, juce::Colours::white);
  addAndMakeVisible(trimLabel);

  trimAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
    processor.getAPVTS(),
    kParamTrimId,
    trimSlider
  );

  // Meter component.
  meter = std::make_unique<MeterComponent>(processor);
  addAndMakeVisible(*meter);

  // Preset UI.
  presetName.setText("My Preset");
  presetName.setColour(juce::TextEditor::textColourId, juce::Colours::white);
  presetName.setColour(juce::TextEditor::backgroundColourId, juce::Colour::fromRGB(26, 30, 34));
  presetName.setColour(juce::TextEditor::outlineColourId, juce::Colour::fromRGB(48, 56, 64));
  addAndMakeVisible(presetName);

  addAndMakeVisible(savePresetButton);
  addAndMakeVisible(loadPresetButton);
  addAndMakeVisible(deletePresetButton);

  addAndMakeVisible(presetList);
  presetList.setJustificationType(juce::Justification::centredLeft);
  presetList.setColour(juce::ComboBox::backgroundColourId, juce::Colour::fromRGB(26, 30, 34));
  presetList.setColour(juce::ComboBox::textColourId, juce::Colours::white);

  auto refreshPresetList = [this]() {
    presetList.clear();
#if USE_SQLITE
    PresetStore store;
    auto dbFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                    .getChildFile("AbeAudio")
                    .getChildFile("ProGain")
                    .getChildFile("presets.db");
    dbFile.getParentDirectory().createDirectory();
    if (store.open(dbFile.getFullPathName().toStdString()))
    {
      const auto names = store.listPresets();
      int itemId = 1;
      for (const auto& name : names)
        presetList.addItem(name, itemId++);
    }
#endif
  };

  savePresetButton.onClick = [this, refreshPresetList]() {
#if USE_SQLITE
    PresetStore store;
    auto dbFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                    .getChildFile("AbeAudio")
                    .getChildFile("ProGain")
                    .getChildFile("presets.db");
    dbFile.getParentDirectory().createDirectory();
    if (!store.open(dbFile.getFullPathName().toStdString()))
      return;

    const auto name = presetName.getText().trim().toStdString();
    if (name.empty())
      return;

    const auto blob = processor.exportPresetBlob();
    store.savePreset(name, blob);
    refreshPresetList();
#endif
  };

  loadPresetButton.onClick = [this]() {
#if USE_SQLITE
    PresetStore store;
    auto dbFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                    .getChildFile("AbeAudio")
                    .getChildFile("ProGain")
                    .getChildFile("presets.db");
    if (!store.open(dbFile.getFullPathName().toStdString()))
      return;

    const auto name = presetList.getText().trim().toStdString();
    if (name.empty())
      return;

    std::string blob;
    if (store.loadPreset(name, blob))
      processor.importPresetBlob(blob);
#endif
  };

  deletePresetButton.onClick = [this, refreshPresetList]() {
#if USE_SQLITE
    PresetStore store;
    auto dbFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                    .getChildFile("AbeAudio")
                    .getChildFile("ProGain")
                    .getChildFile("presets.db");
    if (!store.open(dbFile.getFullPathName().toStdString()))
      return;

    const auto name = presetList.getText().trim().toStdString();
    if (name.empty())
      return;

    store.deletePreset(name);
    refreshPresetList();
#endif
  };

  refreshPresetList();
}

ProGainAudioProcessorEditor::~ProGainAudioProcessorEditor() = default;

void ProGainAudioProcessorEditor::paint(juce::Graphics& g)
{
  // Subtle gradient background and a frame.
  juce::ColourGradient bg(
    juce::Colour::fromRGB(22, 26, 29),
    0.0f, 0.0f,
    juce::Colour::fromRGB(36, 42, 48),
    0.0f, (float) getHeight(),
    false
  );
  g.setGradientFill(bg);
  g.fillAll();

  g.setColour(juce::Colours::white.withAlpha(0.15f));
  g.drawRoundedRectangle(getLocalBounds().toFloat().reduced(10.0f), 16.0f, 1.0f);
}

void ProGainAudioProcessorEditor::resized()
{
  // Lay out the meter on the right and the knob on the left.
  auto bounds = getLocalBounds().reduced(24);

  auto meterArea = bounds.removeFromRight(60);
  if (meter)
    meter->setBounds(meterArea);

  auto topArea = bounds.removeFromTop(200);
  auto knobArea = topArea.withTrimmedTop(20);

  auto gainArea = knobArea.removeFromLeft(200);
  gainSlider.setBounds(gainArea.removeFromTop(160).withSizeKeepingCentre(160, 160));
  gainLabel.setBounds(gainSlider.getX(), gainSlider.getBottom(), gainSlider.getWidth(), 20);

  auto trimArea = knobArea.removeFromLeft(200);
  trimSlider.setBounds(trimArea.removeFromTop(160).withSizeKeepingCentre(160, 160));
  trimLabel.setBounds(trimSlider.getX(), trimSlider.getBottom(), trimSlider.getWidth(), 20);

  auto presetArea = bounds;
  presetArea.removeFromTop(10);
  presetName.setBounds(presetArea.removeFromTop(28));
  presetArea.removeFromTop(8);
  presetList.setBounds(presetArea.removeFromTop(28));
  presetArea.removeFromTop(8);
  auto buttonRow = presetArea.removeFromTop(28);
  savePresetButton.setBounds(buttonRow.removeFromLeft(100));
  buttonRow.removeFromLeft(8);
  loadPresetButton.setBounds(buttonRow.removeFromLeft(100));
  buttonRow.removeFromLeft(8);
  deletePresetButton.setBounds(buttonRow.removeFromLeft(100));
}
