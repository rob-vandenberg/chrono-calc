#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <list>
#include <random>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

// --- Version ------------------------------------------------------------
// third_party/calculator/shim/pch.h 0.0.3

// --- Version history ----------------------------------------------------
// v0.0.3: Initial: Microsoft's CalcManager/pch.h without its Windows-only headers (intsafe.h, ppltasks.h, winerror.h)

// Chrono Calc's own file, not Microsoft's (D-014). The engine's NumberFormattingUtils.cpp includes "pch.h"; Microsoft's
// pch.h is not copied because it includes Windows-only headers, so this one is found instead. It lists the same standard
// headers as Microsoft's, which say themselves that the engine builds with or without a precompiled header.
