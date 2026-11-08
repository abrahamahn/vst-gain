#include "MeterComponent.h"

MeterComponent::MeterComponent(ProGainAudioProcessor& proc)
  : processor(proc)
{
  // Smoothing for the visual meter (not audio).
  meterSmoothed.reset(30.0, 0.15);
  meterSmoothed.setCurrentAndTargetValue(0.0f);
  startTimerHz(30);
}

void MeterComponent::paint(juce::Graphics& g)
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

void MeterComponent::timerCallback()
{
  // Poll the processor's atomic meter and animate smoothly.
  const float target = processor.getMeterLevel();
  meterSmoothed.setTargetValue(target);
  currentLevel = meterSmoothed.getNextValue();
  repaint();
}
