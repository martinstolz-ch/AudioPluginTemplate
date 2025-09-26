/**
 * (c) martin stolz at noizplay.com
 */

#include <JuceHeader.h>

#include "PluginProcessor.h"
#include "PluginUI.h"

AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new apt::PluginProcessor();
}
