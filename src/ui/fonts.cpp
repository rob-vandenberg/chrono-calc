#include "ui/fonts.h"

#include <QFont>
#include <QFontDatabase>
#include <QFontMetricsF>

#include <algorithm>
#include <cmath>

// --- Version ------------------------------------------------------------
// src/ui/fonts.cpp 0.1.6

// --- Version history ----------------------------------------------------
// v0.1.6: Initial: fonts sized by the height of their digits, as measured in Windows 7

namespace chrono_calc {

namespace {

constexpr int measuringPixelSize = 100;
constexpr int smallestPixelSize = 6;
// A font that cannot be measured at the measuring size (a bitmap font) could give an absurd size: at most this many
// times the wanted height
constexpr double largestToInkHeight = 2.5;

}  // namespace

QFont fontWithInkHeight(QFont font, double inkHeight)
{
    font.setPixelSize(measuringPixelSize);
    const double digitHeight = QFontMetricsF(font).tightBoundingRect(QStringLiteral("0123456789")).height();
    const int pixelSize = digitHeight > 0.0 ? static_cast<int>(std::lround(inkHeight * measuringPixelSize / digitHeight))
                                            : static_cast<int>(std::lround(inkHeight * 1.4));
    const int largest = static_cast<int>(std::lround(inkHeight * largestToInkHeight));
    font.setPixelSize(std::clamp(pixelSize, smallestPixelSize, std::max(largest, smallestPixelSize)));
    return font;
}

QFont fixedWidthFont()
{
    return QFontDatabase::systemFont(QFontDatabase::FixedFont);
}

}  // namespace chrono_calc
