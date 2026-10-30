#pragma once

#include <cstdint>

// --- Version ------------------------------------------------------------
// src/core/view.h 0.3.8

// --- Version history ----------------------------------------------------
// v0.3.8: The Programmer view, its number bases and word sizes
// v0.2.7: Initial: the views and the angle units

namespace chrono_calc {

// The views of the calculator (Statistics comes later)
enum class View : std::uint8_t
{
    Standard,
    Scientific,
    Programmer,
};

// The angle unit of the Scientific view's trigonometry
enum class AngleUnit : std::uint8_t
{
    Degrees,
    Radians,
    Grads,
};

// The number base of the Programmer view
enum class NumberBase : std::uint8_t
{
    Hexadecimal,
    Decimal,
    Octal,
    Binary,
};

// The word size of the Programmer view: how many bits a number has (Qword 64, Dword 32, Word 16, Byte 8)
enum class WordSize : std::uint8_t
{
    Qword,
    Dword,
    Word,
    Byte,
};

}  // namespace chrono_calc
