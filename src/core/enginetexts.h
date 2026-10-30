#pragma once

#include "CalcManager/CalculatorResource.h"

#include <string>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/core/enginetexts.h 0.5.12

// --- Version history ----------------------------------------------------
// v0.5.12: Digit grouping: the regional grouping pattern, used only while View > Digit grouping is on
// v0.1.5: The number separators are given by the caller (the computer's regional settings)
// v0.0.4: Initial: the engine's texts (D-015)

namespace chrono_calc {

// The number separators of the computer's regional settings
struct Separators
{
    wchar_t decimal = L'.';
    wchar_t thousands = L',';
    // How the digits are grouped, in the engine's form: "3;0" every three, "3;2;0" three and then every two (India)
    std::wstring grouping = L"3;0";
};

// The texts the engine shows (error messages, operator names in the expression line) and the number separators.
// Every text number the engine can look up has a text: the engine stores a lookup key that points to a temporary
// string when it looks up a number it was given no text for (CalcEngine.h, GetString(int)), so none may be missing.
// Microsoft's g_sids (EngineStrings.h) lists SIDS_C ("62") where SIDS_CLEAR ("1") is meant and SIDS_RSH ("10") where
// SIDS_RSH2 ("98") is meant, so "1" and "98" are never loaded whatever we give; neither is reachable through GetString(int).
class EngineTexts : public CalculationManager::IResourceProvider
{
public:
    explicit EngineTexts(Separators separators = {});
    std::wstring GetCEngineString(std::wstring_view id) override;
    void setSeparators(Separators separators);
    // View > Digit grouping: the engine groups the digits of decimal numbers only while it is on (Windows 7 starts off)
    void setDigitGrouping(bool grouping);

private:
    Separators m_separators;
    bool m_digitGrouping = false;
};

}  // namespace chrono_calc
