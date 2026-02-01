/**
  PluginProcessor.cpp
  -------------------
  Implements the real-time audio path.

  Walkthrough:
  - The gain parameter is read atomically each block.
  - We smooth gain changes to avoid clicks.
  - We compute a peak meter and store it atomically for the UI.
  - We provide helpers to serialize/restore parameter state for presets.
*/
#include "PluginProcessor.h"

#include "PluginEditor.h"
#include "infra/parameters/ParameterRegistry.h"

namespace {
constexpr const char* kParamGainId = "gain";
constexpr const char* kParamTrimId = "trim";
}  // namespace

ProGainAudioProcessor::ProGainAudioProcessor()
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMS", createParameterLayout()) {}

ProGainAudioProcessor::~ProGainAudioProcessor() = default;

const juce::String ProGainAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool ProGainAudioProcessor::acceptsMidi() const { return false; }
bool ProGainAudioProcessor::producesMidi() const { return false; }
bool ProGainAudioProcessor::isMidiEffect() const { return false; }
double ProGainAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int ProGainAudioProcessor::getNumPrograms() { return 1; }
int ProGainAudioProcessor::getCurrentProgram() { return 0; }
void ProGainAudioProcessor::setCurrentProgram(int) {}
const juce::String ProGainAudioProcessor::getProgramName(int) { return {}; }
void ProGainAudioProcessor::changeProgramName(int, const juce::String&) {}

void ProGainAudioProcessor::prepareToPlay(double sampleRate, int) {
    // Smoothing time (seconds) for gain changes.
    const auto* spec = params::find(kParamGainId);
    gainSmoothed.reset(sampleRate, spec ? spec->smoothingSeconds : 0.02f);
    const auto* gainParam = apvts.getRawParameterValue(kParamGainId);
    gainSmoothed.setCurrentAndTargetValue(gainParam->load());

    const auto* trimSpec = params::find(kParamTrimId);
    trimSmoothed.reset(sampleRate,
                       trimSpec ? trimSpec->smoothingSeconds : 0.02f);
    const auto* trimParam = apvts.getRawParameterValue(kParamTrimId);
    trimSmoothed.setCurrentAndTargetValue(trimParam->load());

    meterLevel.store(0.0f);
}

void ProGainAudioProcessor::releaseResources() {}

bool ProGainAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts) const {
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void ProGainAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                         juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Atomically read the parameter once per block, then smooth per-sample.
    const auto* gainParam = apvts.getRawParameterValue(kParamGainId);
    gainSmoothed.setTargetValue(gainParam->load());
    const auto* trimParam = apvts.getRawParameterValue(kParamTrimId);
    trimSmoothed.setTargetValue(trimParam->load());

    float blockPeak = 0.0f;
    // Cache channel pointers once to avoid repeated lookups in the inner loop.
    std::vector<float*> channelData;
    channelData.reserve((size_t)numChannels);
    for (int ch = 0; ch < numChannels; ++ch)
        channelData.push_back(buffer.getWritePointer(ch));

    for (int sample = 0; sample < numSamples; ++sample) {
        const float g = gainSmoothed.getNextValue();
        const float trimDb = trimSmoothed.getNextValue();
        const float trim = juce::Decibels::decibelsToGain(trimDb);
        const float total = g * trim;
        for (int ch = 0; ch < numChannels; ++ch) {
            float* data = channelData[(size_t)ch];
            const float v = data[sample] * total;
            data[sample] = v;
            blockPeak = juce::jmax(blockPeak, std::abs(v));
        }
    }

    // Push peak meter value to UI thread (atomic).
    meterLevel.store(blockPeak);
}

bool ProGainAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* ProGainAudioProcessor::createEditor() {
    return new ProGainAudioProcessorEditor(*this);
}

void ProGainAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void ProGainAudioProcessor::setStateInformation(const void* data,
                                                int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xmlState(
        getXmlFromBinary(data, sizeInBytes));
    if (xmlState && xmlState->hasTagName(apvts.state.getType()))
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

ProGainAudioProcessor::APVTS::ParameterLayout
ProGainAudioProcessor::createParameterLayout() {
    return params::createLayout();
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new ProGainAudioProcessor();
}

std::string ProGainAudioProcessor::exportPresetBlob() {
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    if (!xml) return {};

    juce::MemoryBlock block;
    copyXmlToBinary(*xml, block);
    return std::string(static_cast<const char*>(block.getData()),
                       block.getSize());
}

bool ProGainAudioProcessor::importPresetBlob(const std::string& blob) {
    if (blob.empty()) return false;

    auto xml = getXmlFromBinary(blob.data(), (int)blob.size());
    if (!xml || !xml->hasTagName(apvts.state.getType())) return false;

    apvts.replaceState(juce::ValueTree::fromXml(*xml));
    return true;
}
