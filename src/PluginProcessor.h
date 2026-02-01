#pragma once

#include <JuceHeader.h>

#include <string>

/**
  ProGainAudioProcessor
  ---------------------
  This is the "audio thread" brain of the plugin.

  Key ideas for beginners:
  - prepareToPlay() runs once before audio starts. Set up DSP here.
  - processBlock() runs for every audio buffer. Keep it real-time safe:
    no allocations, no locks, no file I/O, no logging.
  - Parameters are owned by APVTS (AudioProcessorValueTreeState) and are
    accessed on the audio thread via getRawParameterValue().
  - The meterLevel is a simple atomic that the UI reads.
*/
class ProGainAudioProcessor : public juce::AudioProcessor {
   public:
    ProGainAudioProcessor();
    ~ProGainAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    using APVTS = juce::AudioProcessorValueTreeState;
    APVTS& getAPVTS() { return apvts; }

    float getMeterLevel() const { return meterLevel.load(); }

    // Serialize current parameter state for saving presets.
    std::string exportPresetBlob();
    bool importPresetBlob(const std::string& blob);

    // All parameters are declared in one place so the UI + processor stay in
    // sync.
    static APVTS::ParameterLayout createParameterLayout();

   private:
    APVTS apvts;
    std::atomic<float> meterLevel{0.0f};
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> gainSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> trimSmoothed;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProGainAudioProcessor)
};
