#pragma once

#include <cstdint>

// --- Version ------------------------------------------------------------
// src/core/view.h 0.2.7

// --- Version history ----------------------------------------------------
// v0.2.7: Initial: the views and the angle units

namespace chrono_calc {

// The views of the calculator (Programmer and Statistics come later)
enum class View : std::uint8_t
{
    Standard,
    Scientific,
};

// The angle unit of the Scientific view's trigonometry
enum class AngleUnit : std::uint8_t
{
    Degrees,
    Radians,
    Grads,
};

}  // namespace chrono_calc
