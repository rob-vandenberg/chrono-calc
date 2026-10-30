// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.
// Ported from microsoft/calculator commit 79e1ac5, src/CalculatorUnitTests/EngineResourceProvider.h, for Chrono Calc.

#pragma once

#include "CalcManager/CalculatorResource.h"

#include <string>
#include <string_view>

// --- Version ------------------------------------------------------------
// tests/engine/engine_resource_provider.h 0.0.3

// --- Version history ----------------------------------------------------
// v0.0.3: Initial: the engine's texts for the ported tests, without the Windows resource loader

namespace chrono_calc::engine_tests {

// The engine's texts as the original tests see them: the en-US texts of CEngineStrings.resw and the en-US number
// separators. Windows read them from its resource loader; here they are a table in engine_resource_provider.cpp.
class EngineResourceProvider : public CalculationManager::IResourceProvider
{
public:
    std::wstring GetCEngineString(std::wstring_view id) override;
};

}  // namespace chrono_calc::engine_tests
