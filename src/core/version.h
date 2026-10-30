#pragma once

#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/version.h 0.9.14

// --- Version history ----------------------------------------------------
// v0.9.14: The repository's address and the licence's address (Help > View Help, the About box)
// v0.0.2: Initial: the project version as text

namespace chrono_calc {

// The project version from CMakeLists.txt, for example "0.0.2"
std::string_view versionText();

// The project's home on GitHub: Help > View Help opens it, the About box links to it
std::string_view repositoryAddress();

// The text of the licence (GNU Affero General Public License, version 3 or later), linked from the About box
std::string_view licenceAddress();

}  // namespace chrono_calc
