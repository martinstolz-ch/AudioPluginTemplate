/**
 * (c) martin stolz at noizplay.com
 */

#pragma once

#include <JuceHeader.h>

#define BOOLIFY_TRUE  true
#define BOOLIFY_FALSE false

namespace apt::cmakeVar {

static const String
    projectName  {"@PROJECT_NAME@"},
    productName  {"@ProductName@"},
    buildVersion {"@buildVersion@"},

    companyName {"@companyName@"},
    companyURL  {"@companyURL@"};

static constexpr bool
    isSynth            {BOOLIFY_@isSynth@},
    isMidiEffect       {BOOLIFY_@isMidiEffect@},
    needsMidiInput     {BOOLIFY_@needsMidiInput@},
    needsMidiOutput    {BOOLIFY_@needsMidiOutput@},
    wantsKeyboardFocus {BOOLIFY_@wantsKeyboardFocus@};

}

#undef BOOLIFY_TRUE
#undef BOOLIFY_FALSE
