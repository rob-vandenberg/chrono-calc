#include "core/unitconverter.h"

#include "core/unitconversion.h"

#include "Header Files/Rational.h"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

// --- Version ------------------------------------------------------------
// src/core/unitconverter.cpp 0.8.13

// --- Version history ----------------------------------------------------
// v0.8.13: Initial: the Unit conversion panel's state: the type, the two units, the From text and the To result (D-025)

namespace chrono_calc {

UnitConverter::UnitConverter(wchar_t decimal)
    : m_decimal(decimal)
{
}

void UnitConverter::chooseType(std::size_t type)
{
    if (type >= unitTypes().size()) {
        return;
    }
    m_type = type;
    m_from = 0;
    m_to = 0;
    m_input.clear();
}

void UnitConverter::chooseFrom(std::size_t unit)
{
    if (unit < unitTypes()[m_type].units.size()) {
        m_from = unit;
    }
}

void UnitConverter::chooseTo(std::size_t unit)
{
    if (unit < unitTypes()[m_type].units.size()) {
        m_to = unit;
    }
}

void UnitConverter::setInput(std::wstring_view text)
{
    m_input = accepted(text);
}

std::size_t UnitConverter::type() const
{
    return m_type;
}

std::size_t UnitConverter::from() const
{
    return m_from;
}

std::size_t UnitConverter::to() const
{
    return m_to;
}

const std::wstring& UnitConverter::input() const
{
    return m_input;
}

wchar_t UnitConverter::decimal() const
{
    return m_decimal;
}

std::wstring UnitConverter::accepted(std::wstring_view text) const
{
    return acceptedNumberText(text, m_decimal);
}

std::optional<CalcEngine::Rational> UnitConverter::inputValue() const
{
    return typedNumber(m_input, m_decimal);
}

const std::wstring& UnitConverter::result() const
{
    return m_result;
}

void UnitConverter::setResult(std::wstring result)
{
    m_result = std::move(result);
}

}  // namespace chrono_calc
