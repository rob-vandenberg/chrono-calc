#pragma once

#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/version.h 0.0.2

// --- Version history ----------------------------------------------------
// v0.0.2: Initial: the project version as text

namespace chrono_calc {

// The project version from CMakeLists.txt, for example "0.0.2"
std::string_view versionText();

}  // namespace chrono_calc
