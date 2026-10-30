#include "ui/calcbutton.h"

#include <QAbstractButton>
#include <QFont>
#include <QLinearGradient>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QRectF>
#include <QString>
#include <QTimer>
#include <QWidget>

#include "core/colours.h"
#include "ui/fonts.h"
#include "ui/look.h"

#include <string_view>

// --- Version ------------------------------------------------------------
// src/ui/calcbutton.cpp 0.1.6

// --- Version history ----------------------------------------------------
// v0.1.6: Initial: a calculator button in our own drawing, close to the Windows 7 Calculator (D-005, D-018)

namespace chrono_calc {

namespace {

constexpr double cornerRadius = 3.0;
// The height of a digit on the label, in pixels, as in Windows 7 (tests/reference/windows7-standard.md part G)
constexpr double labelInkHeight = 14.0;
// How long a button looks pressed when its key is typed
constexpr int flashMilliseconds = 100;

struct Face
{
    Colour border;
    Colour top;
    Colour middle;
    Colour bottom;
};

Face faceFor(bool pressed, bool hovered)
{
    if (pressed) {
        return {.border = colours::buttonPressedBorder,
                .top = colours::buttonPressedTop,
                .middle = colours::buttonPressedMiddle,
                .bottom = colours::buttonPressedBottom};
    }
    if (hovered) {
        return {.border = colours::buttonHoverBorder,
                .top = colours::buttonHoverTop,
                .middle = colours::buttonHoverMiddle,
                .bottom = colours::buttonHoverBottom};
    }
    return {.border = colours::buttonBorder,
            .top = colours::buttonTop,
            .middle = colours::buttonMiddle,
            .bottom = colours::buttonBottom};
}

}  // namespace

CalcButton::CalcButton(std::string_view commandId, const QString& label, QWidget* parent)
    : QAbstractButton(parent)
    , m_commandId(commandId)
    , m_flashTimer(new QTimer(this))
{
    setText(label);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_Hover);
    setFont(fontWithInkHeight(font(), labelInkHeight));
    m_flashTimer->setSingleShot(true);
    m_flashTimer->setInterval(flashMilliseconds);
    connect(m_flashTimer, &QTimer::timeout, this, [this] { update(); });
}

std::string_view CalcButton::commandId() const
{
    return m_commandId;
}

void CalcButton::flash()
{
    m_flashTimer->start();
    update();
}

bool CalcButton::looksPressed() const
{
    return isDown() || m_flashTimer->isActive();
}

void CalcButton::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const bool pressed = looksPressed();
    const Face face = faceFor(pressed, underMouse() && isEnabled());
    const QRectF outer = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);

    QLinearGradient gradient(outer.topLeft(), outer.bottomLeft());
    gradient.setColorAt(0.0, qtColour(face.top));
    gradient.setColorAt(0.5, qtColour(face.middle));
    gradient.setColorAt(1.0, qtColour(face.bottom));
    painter.setPen(QPen(qtColour(face.border), 1.0));
    painter.setBrush(gradient);
    painter.drawRoundedRect(outer, cornerRadius, cornerRadius);

    // A thin light line just inside the edge gives the face its slight relief; a pressed button is sunk, without it
    if (!pressed) {
        painter.setPen(QPen(qtColour(colours::buttonInnerLine), 1.0));
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(outer.adjusted(1.0, 1.0, -1.0, -1.0), cornerRadius - 1.0, cornerRadius - 1.0);
    }

    painter.setPen(qtColour(colours::buttonText));
    painter.drawText(rect(), Qt::AlignCenter, text());
}

}  // namespace chrono_calc
