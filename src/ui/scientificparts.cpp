#include "ui/scientificparts.h"

#include <QAbstractButton>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QRect>
#include <QRectF>
#include <QString>
#include <QWidget>

#include "core/colours.h"
#include "ui/fonts.h"
#include "ui/look.h"

#include <string_view>

// --- Version ------------------------------------------------------------
// src/ui/scientificparts.cpp 0.2.7

// --- Version history ----------------------------------------------------
// v0.2.7: Initial: the Scientific view's angle units, their frame and the bracket counter (D-020)

namespace chrono_calc {

namespace {

constexpr double cornerRadius = 3.0;
// The labels are in the small font of the function buttons (Segoe UI 13 px in Windows 7)
constexpr double labelInkHeight = 9.0;
constexpr int circleDiameter = 13;
constexpr int dotDiameter = 5;
constexpr int circleToLabel = 5;

struct CellColours
{
    Colour border;
    Colour face;
};

// A flat cell with an edge, as the angle frame and the bracket counter
void paintFlatCell(QPainter& painter, const QRect& area, CellColours cell)
{
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(qtColour(cell.border), 1.0));
    painter.setBrush(qtColour(cell.face));
    painter.drawRoundedRect(QRectF(area).adjusted(0.5, 0.5, -0.5, -0.5), cornerRadius, cornerRadius);
}

}  // namespace

AngleFrame::AngleFrame(QWidget* parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::NoFocus);
}

void AngleFrame::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    paintFlatCell(painter, rect(), {.border = colours::angleFrameBorder, .face = colours::angleFrameFace});
}

AngleButton::AngleButton(std::string_view commandId, const QString& label, QWidget* parent)
    : QAbstractButton(parent)
    , m_commandId(commandId)
{
    setText(label);
    setCheckable(true);
    setAutoExclusive(false);  // the calculator says which unit is chosen
    setFocusPolicy(Qt::NoFocus);
    setFont(fontWithInkHeight(font(), labelInkHeight));
}

std::string_view AngleButton::commandId() const
{
    return m_commandId;
}

void AngleButton::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const QRectF circle(0.5, ((height() - circleDiameter) / 2.0) + 0.5, circleDiameter - 1.0, circleDiameter - 1.0);
    painter.setPen(QPen(qtColour(colours::angleCircleBorder), 1.0));
    painter.setBrush(qtColour(colours::angleCircle));
    painter.drawEllipse(circle);
    if (isChecked()) {
        const double inset = (circleDiameter - dotDiameter) / 2.0;
        painter.setPen(Qt::NoPen);
        painter.setBrush(qtColour(colours::angleDot));
        painter.drawEllipse(circle.adjusted(inset - 0.5, inset - 0.5, -(inset - 0.5), -(inset - 0.5)));
    }
    painter.setPen(qtColour(colours::angleText));
    painter.drawText(rect().adjusted(circleDiameter + circleToLabel, 0, 0, 0), Qt::AlignLeft | Qt::AlignVCenter, text());
}

BracketCounter::BracketCounter(QWidget* parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setFont(fontWithInkHeight(font(), labelInkHeight));
}

void BracketCounter::showCount(unsigned int openBrackets)
{
    const QString text = openBrackets == 0 ? QString() : QStringLiteral("(=%1").arg(openBrackets);
    if (text != m_text) {
        m_text = text;
        update();
    }
}

QString BracketCounter::text() const
{
    return m_text;
}

void BracketCounter::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    paintFlatCell(painter, rect(), {.border = colours::counterBorder, .face = colours::counterFace});
    painter.setPen(qtColour(colours::counterText));
    painter.drawText(rect(), Qt::AlignCenter, m_text);
}

}  // namespace chrono_calc
