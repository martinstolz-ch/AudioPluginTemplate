/**
 * (c) martin stolz at noizplay.com
 */

#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"

namespace apt {

class PluginUI final
    : public AudioProcessorEditor {

public:
    explicit PluginUI (PluginProcessor& pro)
        : AudioProcessorEditor {&pro}, processorRef {pro} {

        setSize (400, 300);
        setTitle (cmakeVar::productName);
    }

    void paint (Graphics& g) override {
        g.fillAll (Colours::black);
        g.setColour (Colours::white);
        g.setFont (20.0f);
        g.drawFittedText (cmakeVar::buildVersion, getLocalBounds(), Justification::centredBottom, 1);
    }

    void resized() override {
    }

private:
    [[maybe_unused]] PluginProcessor& processorRef;
};

inline AudioProcessorEditor* PluginProcessor::createEditor() {
    return new PluginUI (*this);
}

}
