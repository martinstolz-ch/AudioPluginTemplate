/**
 * (c) martin stolz @ noizplay
 */

#pragma once

#include <JuceHeader.h>

// switch off the warning level for tsf.h which is written in plain old C
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"

#define TSF_IMPLEMENTATION
#include "tsf.h"

// restore warning level
#pragma clang diagnostic pop

#include "config.h"

namespace pep {

class Sf2Engine final {
public:
    Sf2Engine() = default;

    ~Sf2Engine() {
        unloadSf2();
    }

    bool loadSf2Binary (const char* binaryData, const int binarySize) {
        unloadSf2();

        _sf2 = tsf_load_memory (binaryData, binarySize);

        jassert (_sf2 != nullptr);
        return true;
    }

    void unloadSf2() {
        if (_sf2) {
            tsf_close (_sf2);
            _sf2 = nullptr;
        }
    }

    void setOutput (const int numChannels, const double sampleRate) {
        _sampleRate  = sampleRate;
        _numChannels = numChannels;
        _updateOutput();
    }

    void setRenderBuffer (const size_t numSamples) {
        _renderBuffer.resize (numSamples);
    }

    void setMaxVoices (const int maxVoices) const {
        jassert (_sf2 != nullptr);

        if (! tsf_set_max_voices (_sf2, maxVoices))
            jassertfalse; // could not set max voices
    }

    void setTuning (const float tuningHz) const {
        jassert (_sf2 != nullptr);

        const float semitonesOffset {12.0f * std::log2(tuningHz / 440.0f)};
        for (int ch{0}; ch < 16; ++ch) {
            tsf_channel_set_tuning (_sf2, ch, semitonesOffset);
        }
    }

    void noteOn (const int midiChannel, const int noteNumber, const int velocity) const {
        jassert (_sf2 != nullptr);
        tsf_channel_note_on (_sf2, midiChannel, noteNumber, static_cast <float> (velocity) * _inv127);
    }

    void noteOff (const int midiChannel, const int noteNumber) const {
        jassert (_sf2 != nullptr);
        tsf_channel_note_off (_sf2, midiChannel, noteNumber);
    }

    void midiController (const int midiChannel, const int controlNumber, const int controlValue) const {
        jassert (_sf2 != nullptr);

        switch (controlNumber) {
            case 7:   // Volume MSB
            case 39:  // Volume LSB
            case 10:  // Pan MSB
            case 42:  // Pan LSB
            case 11:  // Expression MSB
            case 43:  // Expression LSB
            case 64:  // Sustain Pedal
            case 100: // RPN MSB
            case 101: // RPN LSB
            case 120: // All Sounds Off
            case 121: // Reset All Controllers
            case 123: // All Notes Off
                tsf_channel_midi_control (_sf2, midiChannel,controlNumber, controlValue);
                break;

            default:
                DBG(controlNumber << " not handled");
                break;
        }
    }

    void pitchWheel (const int midiChannel, const int pitchWheelValue) const {
        jassert (_sf2 != nullptr);
        tsf_channel_set_pitchwheel (_sf2, midiChannel, pitchWheelValue);
    }

    // all notes off with fade out
    void allNotesOff() const {
        jassert (_sf2 != nullptr);
        tsf_note_off_all (_sf2);
    }

    // all sounds cut off
    void allSoundsOff() const {
        jassert (_sf2 != nullptr);
        for (int ch{0}; ch < 16; ++ch)
            tsf_channel_sounds_off_all (_sf2, ch);
    }

    void renderAudio (const AudioSourceChannelInfo& outBuffer) noexcept {
        jassert (_sf2 != nullptr);

        const auto numSamples        {outBuffer.numSamples};
        const auto outputNumChannels {outBuffer.buffer->getNumChannels()};
        auto* renderData          {_renderBuffer.data()};

        jassert (
            static_cast <size_t> (_numChannels * numSamples) <= _renderBuffer.size()
        ); // size of render buffer ok?

        // adjust tsf's number of channels (hopefully never...)
        if (outputNumChannels != _numChannels) {
            _numChannels = outputNumChannels;
            _updateOutput();
        }

        // render audio
        tsf_render_float (_sf2, renderData, numSamples, 0);

        // copy audio data to the out buffer
        for (int ch = 0; ch < outputNumChannels; ++ch) {
            auto* out {outBuffer.buffer->getWritePointer (ch, outBuffer.startSample)};

            // use the last rendered channel for possible surplus out channels
            const int srcCh {jmin (ch, _numChannels - 1)};

            const float* src {renderData + srcCh * numSamples};
            FloatVectorOperations::copy (out, src, numSamples);
        }
    }

    [[nodiscard]] int activeVoiceCount() const {
        jassert (_sf2 != nullptr);
        return tsf_active_voice_count (_sf2);
    }

    void selectPreset(const int midiChannel, const int presetIndex) const {
        jassert (_sf2 != nullptr);
        jassert (presetIndex >= 0 && presetIndex < tsf_get_presetcount (_sf2)); // preset index out of range?

        tsf_channel_set_presetindex (_sf2, midiChannel, presetIndex);
        DBG("SF2Player: Loaded preset " << tsf_get_presetname(_sf2, presetIndex) << " on channel " + String{midiChannel});
    }

private:
    void _updateOutput() const {
        jassert (_sf2 != nullptr);
        jassert (_sampleRate > 0.0); // sample rate yet undefined?
        tsf_set_output (
            _sf2,
            _numChannels == 1 ? TSF_MONO : TSF_STEREO_UNWEAVED,
            static_cast <int> (_sampleRate),
            _globalGain
        );
    }

    tsf* _sf2 {nullptr};

    TSFOutputMode _outputMode  {TSF_STEREO_UNWEAVED};

    int    _numChannels {config::audio::numChannelsExpected};
    double _sampleRate  {0.0};
    float  _globalGain  {config::audio::globalGain};

    std::vector <float> _renderBuffer;

    static constexpr float _inv127 {1 / 127.0f};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sf2Engine)
};

}
