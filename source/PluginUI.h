/**
 * (c) martin stolz at noizplay.com
 */

#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"

namespace pep {

class PluginUI final
    : public AudioProcessorEditor {

public:
    explicit PluginUI (PluginProcessor& pro)
        : AudioProcessorEditor {&pro}, processorRef {pro} {

        setSize (400, 300);
        setTitle (cmakeVar::productName);

        addAndMakeVisible (_button);
        _button.setButtonText ("Note");
        _button.onClick = [&] {

        };
    }

    void paint (Graphics& g) override {
        g.fillAll (Colours::black);
        g.setColour (Colours::white);
        g.setFont (20.0f);
        g.drawFittedText (cmakeVar::buildVersion, getLocalBounds(), Justification::centredBottom, 1);
    }

    void resized() override {
        _button.setBounds (10,  10, getWidth() - 20,  30);
    }

private:
    PluginProcessor& processorRef;
    TextButton _button;
};

inline AudioProcessorEditor* PluginProcessor::createEditor() {
    return new PluginUI (*this);
}

}
