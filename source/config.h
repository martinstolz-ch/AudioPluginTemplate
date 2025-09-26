/**
 * martin stolz - (c) noizplay.com
 */

#pragma once

namespace pep::config {

 namespace audio {
    static constexpr int   numChannelsExpected {2};
    static constexpr float globalGain          {-5.0f};
}

namespace synth {
    static constexpr int   maxVoices       {64};
    static constexpr float defaultTuningHz {442.0f};
}

}
