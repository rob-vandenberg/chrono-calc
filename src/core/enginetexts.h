#pragma once

#include "CalcManager/CalculatorResource.h"

#include <string>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/enginetexts.h 0.0.4

// --- Version history ----------------------------------------------------
// v0.0.4: Initial: the engine's texts (D-015)

namespace chrono_calc {

// The texts the engine shows (error messages, operator names in the expression line) and the number separators.
// Every text number the engine can look up has a text: the engine stores a lookup key that points to a temporary
// string when it looks up a number it was given no text for (CalcEngine.h, GetString(int)), so none may be missing.
// Microsoft's g_sids (EngineStrings.h) lists SIDS_C ("62") where SIDS_CLEAR ("1") is meant and SIDS_RSH ("10") where
// SIDS_RSH2 ("98") is meant, so "1" and "98" are never loaded whatever we give; neither is reachable through GetString(int).
class EngineTexts : public CalculationManager::IResourceProvider
{
public:
    std::wstring GetCEngineString(std::wstring_view id) override;
};

}  // namespace chrono_calc
