/**
 * (c) martin stolz at noizplay.com
 */

#pragma once

#include <JuceHeader.h>

#include "binaryAssets.h"
#include "cmakeVar.h"
#include "config.h"
#include "Sf2Engine.h"

namespace pep {

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
    void prepareToPlay (const double sampleRate, const int samplesPerBlock) override {

        // load sound font and select preset
        _sf2Engine.loadSf2Binary (binaryAssets::galaxy_sf2, binaryAssets::galaxy_sf2Size);
        _sf2Engine.selectPreset (0, 5);

        // init render buffer and audio engine
        _sf2Engine.setRenderBuffer (static_cast <size_t> (samplesPerBlock) * 4); // take 4x 4 enough space ;)
        _sf2Engine.setOutput    (config::audio::numChannelsExpected, sampleRate);
        _sf2Engine.setMaxVoices (config::synth::maxVoices);
        _sf2Engine.setTuning    (config::synth::defaultTuningHz);
    }

    void releaseResources() override {
        _sf2Engine.unloadSf2();
    }

    void processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override
    {
        ScopedNoDenormals noDenormals;

        const int totalNumSamples = buffer.getNumSamples();
        buffer.clear();

        int startSample = 0;

        for (const auto metadata : midiMessages)
        {
            const auto& msg     = metadata.getMessage();
            const int samplePos = metadata.samplePosition;

            // Länge bis zu diesem Event
            int numSamples = samplePos - startSample;
            if (numSamples > 0)
            {
                AudioSourceChannelInfo info (&buffer, startSample, numSamples);
                _sf2Engine.renderAudio (info);
                startSample += numSamples;
            }

            if (msg.isNoteOn())
                _sf2Engine.noteOn (0, msg.getNoteNumber(), msg.getVelocity());

            else if (msg.isNoteOff())
                _sf2Engine.noteOff (0, msg.getNoteNumber());

            else if (msg.isController())
            {
                const int controller = msg.getControllerNumber();
                const int value      = msg.getControllerValue();
                _sf2Engine.midiController(0, controller, value);
            }

            else if (msg.isPitchWheel())
            {
                int value14bit = msg.getPitchWheelValue(); // JUCE liefert 0–16383, Mitte=8192
                _sf2Engine.pitchWheel (0, value14bit);
            }
        }

        // Rest des Blocks rendern
        const int remaining = totalNumSamples - startSample;
        if (remaining > 0)
        {
            AudioSourceChannelInfo info (&buffer, startSample, remaining);
            _sf2Engine.renderAudio (info);
        }
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
    Sf2Engine _sf2Engine;
};

}
