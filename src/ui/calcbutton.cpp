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
#include "core/layout.h"
#include "ui/fonts.h"
#include "ui/look.h"

#include <string_view>

// --- Version ------------------------------------------------------------
// src/ui/calcbutton.cpp 0.3.8

// --- Version history ----------------------------------------------------
// v0.3.8: The flat face of the hexadecimal digits A to F
// v0.2.7: The faces of Windows 7 (digit, function, memory, toggle), the greyed look, the label under Inv
// v0.1.6: Initial: a calculator button in our own drawing, close to the Windows 7 Calculator (D-005, D-018)

namespace chrono_calc {

namespace {

constexpr double cornerRadius = 3.0;
// How long a button looks pressed when its key is typed
constexpr int flashMilliseconds = 100;

QString fromView(std::string_view text)
{
    return QString::fromUtf8(text.data(), static_cast<qsizetype>(text.size()));
}

struct Face
{
    Colour border;
    Colour top;
    Colour middle;
    Colour bottom;
};

Face restingFace(ButtonFace face, bool on)
{
    switch (face) {
    case ButtonFace::Digit:
        return {.border = colours::buttonBorder, .top = colours::buttonTop, .middle = colours::buttonMiddle, .bottom = colours::buttonBottom};
    case ButtonFace::Memory:
        return {.border = colours::buttonBorder, .top = colours::memoryTop, .middle = colours::memoryFace, .bottom = colours::memoryBottom};
    case ButtonFace::Toggle:
        if (on) {
            return {.border = colours::toggleOnBorder,
                    .top = colours::toggleOnTop,
                    .middle = colours::toggleOnMiddle,
                    .bottom = colours::toggleOnBottom};
        }
        break;
    case ButtonFace::Flat:
        return {.border = colours::buttonBorder, .top = colours::flatFace, .middle = colours::flatFace, .bottom = colours::flatFace};
    case ButtonFace::Function:
        break;
    }
    return {.border = colours::buttonBorder, .top = colours::functionTop, .middle = colours::functionMiddle, .bottom = colours::functionBottom};
}

}  // namespace

CalcButton::CalcButton(const ButtonSpec& spec, QWidget* parent)
    : QAbstractButton(parent)
    , m_commandId(spec.commandId)
    , m_face(spec.face)
    , m_label(fromView(spec.label))
    , m_inverseLabel(fromView(spec.inverseLabel))
    , m_flashTimer(new QTimer(this))
{
    setText(m_label);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_Hover);
    setFont(fontWithInkHeight(font(), spec.labelInkHeight));
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

void CalcButton::showInverse(bool inverse)
{
    if (m_inverseLabel.isEmpty()) {
        return;
    }
    setText(inverse ? m_inverseLabel : m_label);
}

void CalcButton::showOn(bool on)
{
    if (on != m_on) {
        m_on = on;
        update();
    }
}

bool CalcButton::isOn() const
{
    return m_on;
}

void CalcButton::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const QRectF outer = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);

    if (!isEnabled()) {
        // Greyed: a light edge on the window's own colour, no relief
        painter.setPen(QPen(qtColour(colours::greyedBorder), 1.0));
        painter.setBrush(qtColour(colours::greyedFace));
        painter.drawRoundedRect(outer, cornerRadius, cornerRadius);
        painter.setPen(qtColour(colours::greyedText));
        painter.drawText(rect(), Qt::AlignCenter, text());
        return;
    }

    const bool pressed = looksPressed();
    Face face = restingFace(m_face, m_on);
    if (pressed) {
        face = {.border = colours::buttonPressedBorder,
                .top = colours::buttonPressedTop,
                .middle = colours::buttonPressedMiddle,
                .bottom = colours::buttonPressedBottom};
    } else if (underMouse()) {
        face = {.border = colours::buttonHoverBorder,
                .top = colours::buttonHoverTop,
                .middle = colours::buttonHoverMiddle,
                .bottom = colours::buttonHoverBottom};
    }

    QLinearGradient gradient(outer.topLeft(), outer.bottomLeft());
    if (m_face == ButtonFace::Memory && !pressed && !underMouse()) {
        // The memory buttons are flat: one light line at the top, then the face
        gradient.setColorAt(0.0, qtColour(face.top));
        gradient.setColorAt(0.08, qtColour(face.middle));
        gradient.setColorAt(0.9, qtColour(face.middle));
        gradient.setColorAt(1.0, qtColour(face.bottom));
    } else {
        gradient.setColorAt(0.0, qtColour(face.top));
        gradient.setColorAt(0.5, qtColour(face.middle));
        gradient.setColorAt(1.0, qtColour(face.bottom));
    }
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
