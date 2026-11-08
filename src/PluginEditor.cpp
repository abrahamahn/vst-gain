/**
  PluginEditor.cpp
  ----------------
  Implements the UI:
  - Rotary gain and output trim knobs bound to the parameter system.
  - Vertical level meter component that reads peak values from the processor.
  - Preset controls backed by SQLite storage.
*/
#include "PluginEditor.h"
#include "infra/parameters/ParameterRegistry.h"
#include "infra/state/PresetStore.h"
#include <string>

namespace
{
#if USE_SQLITE
bool openDefaultPresetStore(PresetStore& store)
{
  auto dbFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                  .getChildFile("AbeAudio")
                  .getChildFile("ProGain")
                  .getChildFile("presets.db");
  dbFile.getParentDirectory().createDirectory();
  return store.open(dbFile.getFullPathName().toStdString());
}
#endif
}

ProGainAudioProcessorEditor::ProGainAudioProcessorEditor(ProGainAudioProcessor& p)
  : AudioProcessorEditor(&p), processor(p)
{
  setSize(520, 300);

  configureRotaryControl(
    gainSlider,
    gainLabel,
    "Gain",
    juce::Colour::fromRGB(64, 196, 92)
  );
  gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
    processor.getAPVTS(),
    params::kGainId,
    gainSlider
  );

  configureRotaryControl(
    trimSlider,
    trimLabel,
    "Output Trim",
    juce::Colour::fromRGB(232, 178, 62)
  );
  trimAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
    processor.getAPVTS(),
    params::kTrimId,
    trimSlider
  );

  meter = std::make_unique<MeterComponent>(processor);
  addAndMakeVisible(*meter);

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

  savePresetButton.onClick = [this]() {
#if USE_SQLITE
    PresetStore store;
    if (!openDefaultPresetStore(store))
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
    if (!openDefaultPresetStore(store))
      return;

    const auto name = presetList.getText().trim().toStdString();
    if (name.empty())
      return;

    std::string blob;
    if (store.loadPreset(name, blob))
      processor.importPresetBlob(blob);
#endif
  };

  deletePresetButton.onClick = [this]() {
#if USE_SQLITE
    PresetStore store;
    if (!openDefaultPresetStore(store))
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

void ProGainAudioProcessorEditor::configureRotaryControl(
  juce::Slider& slider,
  juce::Label& label,
  const juce::String& labelText,
  juce::Colour fillColour)
{
  slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
  slider.setColour(juce::Slider::rotarySliderFillColourId, fillColour);
  slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour::fromRGB(32, 40, 46));
  slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
  addAndMakeVisible(slider);

  label.setText(labelText, juce::dontSendNotification);
  label.setJustificationType(juce::Justification::centred);
  label.setColour(juce::Label::textColourId, juce::Colours::white);
  addAndMakeVisible(label);
}

void ProGainAudioProcessorEditor::refreshPresetList()
{
  presetList.clear();
#if USE_SQLITE
  PresetStore store;
  if (openDefaultPresetStore(store))
  {
    const auto names = store.listPresets();
    int itemId = 1;
    for (const auto& name : names)
      presetList.addItem(name, itemId++);
  }
#endif
}

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
