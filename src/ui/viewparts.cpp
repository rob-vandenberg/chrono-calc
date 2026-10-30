#include "ui/viewparts.h"

#include <QAbstractButton>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QRect>
#include <QRectF>
#include <QString>
#include <QWidget>

#include "core/colours.h"
#include "core/layout.h"
#include "ui/fonts.h"
#include "ui/look.h"

#include <cstddef>
#include <string>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/ui/viewparts.cpp 0.3.8

// --- Version history ----------------------------------------------------
// v0.3.8: Choices of any kind (bases and word sizes too) and the Programmer view's bit display (D-021); was
//         scientificparts
// v0.2.7: Initial: the Scientific view's angle units, their frame and the bracket counter (D-020)

namespace chrono_calc {

namespace {

constexpr double cornerRadius = 3.0;
// The labels are in the small font of the function buttons (Segoe UI 13 px in Windows 7)
constexpr double labelInkHeight = 9.0;
constexpr int circleDiameter = 13;
constexpr int dotDiameter = 5;
constexpr int circleToLabel = 5;
// The bits and their positions are in the fixed-width font, as Windows 7 (Consolas 15 px: digits 10 px high)
constexpr double bitInkHeight = 10.0;

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

FlatFrame::FlatFrame(QWidget* parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::NoFocus);
}

void FlatFrame::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    paintFlatCell(painter, rect(), {.border = colours::frameBorder, .face = colours::frameFace});
}

ChoiceButton::ChoiceButton(std::string_view commandId, const QString& label, QWidget* parent)
    : QAbstractButton(parent)
    , m_commandId(commandId)
{
    setText(label);
    setCheckable(true);
    setAutoExclusive(false);  // the calculator says which choice is chosen
    setFocusPolicy(Qt::NoFocus);
    setFont(fontWithInkHeight(font(), labelInkHeight));
}

std::string_view ChoiceButton::commandId() const
{
    return m_commandId;
}

void ChoiceButton::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const QRectF circle(0.5, ((height() - circleDiameter) / 2.0) + 0.5, circleDiameter - 1.0, circleDiameter - 1.0);
    painter.setPen(QPen(qtColour(colours::choiceCircleBorder), 1.0));
    painter.setBrush(qtColour(colours::choiceCircle));
    painter.drawEllipse(circle);
    if (isChecked()) {
        const double inset = (circleDiameter - dotDiameter) / 2.0;
        painter.setPen(Qt::NoPen);
        painter.setBrush(qtColour(colours::choiceDot));
        painter.drawEllipse(circle.adjusted(inset - 0.5, inset - 0.5, -(inset - 0.5), -(inset - 0.5)));
    }
    painter.setPen(qtColour(colours::choiceText));
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

BitDisplay::BitDisplay(const BitDisplaySpec& spec, QWidget* parent)
    : QWidget(parent)
    , m_spec(spec)
    , m_bits(64, '0')
{
    setFocusPolicy(Qt::NoFocus);
    setFont(fontWithInkHeight(fixedWidthFont(), bitInkHeight));
    setToolTip(tr("Click a bit to turn it over"));
}

void BitDisplay::showBits(const std::string& bits, int wordBits)
{
    if (bits == m_bits && wordBits == m_wordBits) {
        return;
    }
    m_bits = bits;
    m_wordBits = wordBits;
    update();
}

std::string BitDisplay::bits() const
{
    return m_bits;
}

int BitDisplay::visibleBits() const
{
    return m_wordBits;
}

QRect BitDisplay::bitArea(int bit) const
{
    const Rectangle place = bitRectangle(bit);
    return {place.left - m_spec.frame.left, place.top - m_spec.frame.top, place.width, place.height};
}

void BitDisplay::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    paintFlatCell(painter, rect(), {.border = colours::frameBorder, .face = colours::frameFace});
    painter.setPen(qtColour(colours::bitText));
    for (int bit = 0; bit < m_wordBits && bit < 64; ++bit) {
        const char value = m_bits.at(static_cast<std::size_t>(63 - bit));
        painter.drawText(bitArea(bit), Qt::AlignCenter, QString(QLatin1Char(value)));
    }
    painter.setPen(qtColour(colours::bitPositionText));
    for (const BitLabelSpec& label : m_spec.labels) {
        const QRect area(label.rectangle.left - m_spec.frame.left, label.rectangle.top - m_spec.frame.top, label.rectangle.width,
                         label.rectangle.height);
        const Qt::Alignment alignment = (label.alignRight ? Qt::AlignRight : Qt::AlignLeft) | Qt::AlignVCenter;
        painter.drawText(area, static_cast<int>(alignment),
                         QString::fromUtf8(label.text.data(), static_cast<qsizetype>(label.text.size())));
    }
}

void BitDisplay::mouseReleaseEvent(QMouseEvent* event)
{
    for (int bit = 0; bit < m_wordBits && bit < 64; ++bit) {
        if (bitArea(bit).contains(event->position().toPoint())) {
            emit bitClicked(bit);
            return;
        }
    }
    QWidget::mouseReleaseEvent(event);
}

}  // namespace chrono_calc
