#include "core/version.h"

#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/version.cpp 0.9.14

// --- Version history ----------------------------------------------------
// v0.9.14: The repository's address and the licence's address
// v0.0.2: Initial: the project version as text

namespace chrono_calc {

std::string_view versionText()
{
    return CHRONO_CALC_VERSION;
}

std::string_view repositoryAddress()
{
    return "https://github.com/rob-vandenberg/chrono-calc";
}

std::string_view licenceAddress()
{
    return "https://www.gnu.org/licenses/agpl-3.0.html";
}

}  // namespace chrono_calc
