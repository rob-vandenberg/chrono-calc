#include "ui/displaypanel.h"

#include <QFont>
#include <QFontMetrics>
#include <QLinearGradient>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QRect>
#include <QRectF>
#include <QString>
#include <QWidget>

#include "core/colours.h"
#include "core/standardlayout.h"
#include "ui/fonts.h"
#include "ui/look.h"

#include <algorithm>

// --- Version ------------------------------------------------------------
// src/ui/displaypanel.cpp 0.1.6

// --- Version history ----------------------------------------------------
// v0.1.6: Initial: the display of the Standard view, close to the Windows 7 Calculator (D-005, D-018)

namespace chrono_calc {

namespace {

constexpr double cornerRadius = 2.0;
// The height of the figures in pixels, as in Windows 7 (tests/reference/windows7-standard.md part G)
constexpr double numberInkHeight = 19.0;
constexpr double expressionInkHeight = 10.0;
constexpr double memoryMarkInkHeight = 11.0;
constexpr int numberPixelSizeSmallest = 8;

// A part of the display, from the window's measures to the panel's own coordinates
QRect inPanel(Rectangle part)
{
    return {part.left - standard::displayFrame.left, part.top - standard::displayFrame.top, part.width, part.height};
}

QFont withPixelSize(QFont font, int pixelSize)
{
    font.setPixelSize(pixelSize);
    return font;
}

}  // namespace

// The display's figures are drawn in the computer's fixed-width font, as Windows 7 draws them in a fixed-width one;
// the fonts are worked out once
DisplayPanel::DisplayPanel(QWidget* parent)
    : QWidget(parent)
    , m_numberFont(fontWithInkHeight(fixedWidthFont(), numberInkHeight))
    , m_expressionFont(fontWithInkHeight(fixedWidthFont(), expressionInkHeight))
    , m_memoryMarkFont(fontWithInkHeight(fixedWidthFont(), memoryMarkInkHeight))
{
    setFocusPolicy(Qt::NoFocus);
}

void DisplayPanel::showValues(const QString& number, const QString& expression, bool memoryHolds)
{
    if (number == m_number && expression == m_expression && memoryHolds == m_memoryHolds) {
        return;
    }
    m_number = number;
    m_expression = expression;
    m_memoryHolds = memoryHolds;
    setToolTip(m_memoryHolds ? tr("M: a number is kept in memory") : QString());
    update();
}

QString DisplayPanel::number() const
{
    return m_number;
}

QString DisplayPanel::expression() const
{
    return m_expression;
}

bool DisplayPanel::memoryMarkShown() const
{
    return m_memoryHolds;
}

int DisplayPanel::numberPixelSize() const
{
    const int width = inPanel(standard::mainNumber).width();
    const int fullSize = m_numberFont.pixelSize();
    const int fullWidth = QFontMetrics(m_numberFont).horizontalAdvance(m_number);
    if (fullWidth <= width) {
        return fullSize;
    }
    // The width grows with the size: one measurement gives the size that fits, then a step down while it still does
    // not fit
    int pixelSize = std::max(numberPixelSizeSmallest, fullSize * width / fullWidth);
    while (pixelSize > numberPixelSizeSmallest
           && QFontMetrics(withPixelSize(m_numberFont, pixelSize)).horizontalAdvance(m_number) > width) {
        --pixelSize;
    }
    // Hinted fonts round each character's width, so the guess can be one size too small: take the largest that fits
    while (pixelSize + 1 < fullSize
           && QFontMetrics(withPixelSize(m_numberFont, pixelSize + 1)).horizontalAdvance(m_number) <= width) {
        ++pixelSize;
    }
    return pixelSize;
}

void DisplayPanel::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QRectF frame = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    QLinearGradient gradient(frame.topLeft(), frame.bottomLeft());
    gradient.setColorAt(0.0, qtColour(colours::displayTop));
    gradient.setColorAt(0.5, qtColour(colours::displayMiddle));
    gradient.setColorAt(1.0, qtColour(colours::displayBottom));
    painter.setPen(QPen(qtColour(colours::displayBorder), 1.0));
    painter.setBrush(gradient);
    painter.drawRoundedRect(frame, cornerRadius, cornerRadius);
    // A thin white line just inside the border
    painter.setPen(QPen(qtColour(colours::displayInnerLine), 1.0));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(frame.adjusted(1.0, 1.0, -1.0, -1.0), cornerRadius - 1.0, cornerRadius - 1.0);

    // The expression line: when it is longer than the line, its end stays in view (the newest part)
    const QRect expressionRect = inPanel(standard::expressionLine);
    painter.setFont(m_expressionFont);
    painter.setPen(qtColour(colours::displayExpression));
    painter.drawText(expressionRect, Qt::AlignRight | Qt::AlignVCenter,
                     QFontMetrics(m_expressionFont).elidedText(m_expression, Qt::ElideLeft, expressionRect.width()));

    if (m_memoryHolds) {
        painter.setFont(m_memoryMarkFont);
        painter.setPen(qtColour(colours::displayMemoryMark));
        painter.drawText(inPanel(standard::memoryMark), Qt::AlignCenter, QStringLiteral("M"));
    }

    painter.setFont(withPixelSize(m_numberFont, numberPixelSize()));
    painter.setPen(qtColour(colours::displayNumber));
    painter.drawText(inPanel(standard::mainNumber), Qt::AlignRight | Qt::AlignVCenter, m_number);
}

}  // namespace chrono_calc
