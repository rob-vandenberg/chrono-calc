#include "core/version.h"

#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/version.cpp 0.0.2

// --- Version history ----------------------------------------------------
// v0.0.2: Initial: the project version as text

namespace chrono_calc {

std::string_view versionText()
{
    return CHRONO_CALC_VERSION;
}

}  // namespace chrono_calc
