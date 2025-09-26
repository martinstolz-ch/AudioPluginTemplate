/**
 * (c) martin stolz at noizplay.com
 */

#pragma once

#include <JuceHeader.h>

// #include "binaryAssets.h"
#include "cmakeVar.h"
#include "config.h"

namespace apt {

class PluginProcessor final
    : public AudioProcessor {

public:
    PluginProcessor()
        : AudioProcessor (BusesProperties()
              .withOutput ("Output", juce::AudioChannelSet::stereo(), true)) {}

    ~PluginProcessor() override = default;

    // mono or stereo is allowed
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override {
        if (const auto outChannelSet {layouts.getMainOutputChannelSet()};
            outChannelSet == AudioChannelSet::stereo() || outChannelSet == AudioChannelSet::mono())
            return true;

        return false;
    }

    // DSP stuff
    void prepareToPlay (const double /* sampleRate */, const int /* samplesPerBlock */) override {
    }

    void releaseResources() override {
    }

    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& /* midiMessages */) override
    {
        ScopedNoDenormals noDenormals;

        buffer.clear();
    }

    // system stuff
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const String getName() const override { return cmakeVar::productName; }
    bool acceptsMidi() const override { return cmakeVar::needsMidiInput; }
    bool producesMidi() const override { return cmakeVar::needsMidiOutput; }
    bool isMidiEffect() const override { return cmakeVar::isMidiEffect; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const String getProgramName (int) override { return {}; }
    void changeProgramName (int, const String&) override {}

    void getStateInformation (MemoryBlock&) override {}
    void setStateInformation (const void*, int) override {}

private:
    
};

}
