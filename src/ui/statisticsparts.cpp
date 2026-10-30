#include "ui/statisticsparts.h"

#include <QAbstractButton>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QPoint>
#include <QPolygonF>
#include <QPointF>
#include <QRect>
#include <QRectF>
#include <QStringList>
#include <QWheelEvent>
#include <QWidget>

#include "core/colours.h"
#include "core/layout.h"
#include "ui/fonts.h"
#include "ui/look.h"

#include <algorithm>

// --- Version ------------------------------------------------------------
// src/ui/statisticsparts.cpp 0.4.11

// --- Version history ----------------------------------------------------
// v0.4.11: Initial: the Statistics view's list of values and the buttons that move its selection (D-023)

namespace chrono_calc {

namespace {

// The values are in the fixed-width font, as Windows 7 (digits 10 px high), 2 px from the right edge
constexpr double valueInkHeight = 10.0;
constexpr int valueRightMargin = 2;
// The triangle of the selection buttons: 9 px wide, 5 px high, a little below the middle (Windows 7)
constexpr double arrowWidth = 9.0;
constexpr double arrowHeight = 5.0;
constexpr double arrowLeft = 6.0;
constexpr double arrowTop = 8.0;
constexpr double arrowCornerRadius = 2.0;

}  // namespace

StatisticsList::StatisticsList(const StatisticsSpec& spec, QWidget* parent)
    : QWidget(parent)
    , m_spec(spec)
{
    // Only a click gives it the keyboard focus: the calculator's keys stay with the window otherwise
    setFocusPolicy(Qt::ClickFocus);
    setFont(fontWithInkHeight(fixedWidthFont(), valueInkHeight));
}

void StatisticsList::showValues(const QStringList& values, int selected)
{
    if (values == m_values && selected == m_selected) {
        return;
    }
    m_values = values;
    m_selected = selected;
    // The selected row stays in view, as a Windows list keeps it (the newest after Add)
    if (m_selected >= 0) {
        if (m_selected < m_firstVisible) {
            m_firstVisible = m_selected;
        } else if (m_selected >= m_firstVisible + visibleRows()) {
            m_firstVisible = m_selected - visibleRows() + 1;
        }
    }
    keepInRange();
    update();
}

QStringList StatisticsList::values() const
{
    return m_values;
}

int StatisticsList::selected() const
{
    return m_selected;
}

int StatisticsList::firstVisible() const
{
    return m_firstVisible;
}

int StatisticsList::visibleRows() const
{
    return m_spec.rowHeight > 0 ? std::max(1, height() / m_spec.rowHeight) : 1;
}

void StatisticsList::keepInRange()
{
    m_firstVisible = std::clamp(m_firstVisible, 0, std::max(0, static_cast<int>(m_values.size()) - visibleRows()));
}

void StatisticsList::paintEvent(QPaintEvent* /*event*/)
{
    // The display's own face shows through: only the rows are drawn
    QPainter painter(this);
    const int rows = visibleRows() + 1;
    for (int row = 0; row < rows; ++row) {
        const int index = m_firstVisible + row;
        if (index >= m_values.size()) {
            break;
        }
        const int top = row * m_spec.rowHeight;
        const QRect area(0, top, width(), m_spec.rowHeight - 1);
        if (index == m_selected) {
            painter.fillRect(area, qtColour(colours::listSelected));
        }
        painter.setPen(qtColour(colours::listText));
        painter.drawText(area.adjusted(0, 0, -valueRightMargin, 0), Qt::AlignRight | Qt::AlignVCenter, m_values.at(index));
        // A dotted line under the row: every other pixel
        painter.setPen(QPen(qtColour(colours::listRowLine), 1.0));
        for (int x = 0; x < width(); x += 2) {
            painter.drawPoint(x, top + m_spec.rowHeight - 1);
        }
    }
}

void StatisticsList::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_spec.rowHeight > 0) {
        const int index = m_firstVisible + (event->position().toPoint().y() / m_spec.rowHeight);
        if (index >= 0 && index < m_values.size()) {
            emit indexChosen(index);
        }
    }
    QWidget::mousePressEvent(event);
}

// A double click selects, as one click does; the value is not taken into the display (Windows 7)
void StatisticsList::mouseDoubleClickEvent(QMouseEvent* event)
{
    mousePressEvent(event);
}

void StatisticsList::keyPressEvent(QKeyEvent* event)
{
    if (event->modifiers() != Qt::NoModifier && event->modifiers() != Qt::KeypadModifier) {
        QWidget::keyPressEvent(event);
        return;
    }
    const int last = static_cast<int>(m_values.size()) - 1;
    const int page = visibleRows() - 1;
    switch (event->key()) {
    case Qt::Key_Up: emit previousChosen(); break;
    case Qt::Key_Down: emit nextChosen(); break;
    case Qt::Key_Delete: emit deleteChosen(); break;
    case Qt::Key_Home:
        if (last >= 0) {
            emit indexChosen(0);
        }
        break;
    case Qt::Key_End:
        if (last >= 0) {
            emit indexChosen(last);
        }
        break;
    case Qt::Key_PageUp:
        if (m_selected >= 0) {
            emit indexChosen(std::max(0, m_selected - page));
        }
        break;
    case Qt::Key_PageDown:
        if (m_selected >= 0) {
            emit indexChosen(std::min(last, m_selected + page));
        }
        break;
    default:
        // Every other key is the calculator's: the window takes it
        QWidget::keyPressEvent(event);
        return;
    }
    event->accept();
}

// The wheel scrolls the rows in view; the selection stays
void StatisticsList::wheelEvent(QWheelEvent* event)
{
    const int steps = event->angleDelta().y() / 120;
    if (steps != 0) {
        m_firstVisible -= steps;
        keepInRange();
        update();
    }
    event->accept();
}

SelectionArrow::SelectionArrow(bool pointsUp, QWidget* parent)
    : QAbstractButton(parent)
    , m_pointsUp(pointsUp)
{
    setFocusPolicy(Qt::NoFocus);
    setToolTip(pointsUp ? tr("Select the value above") : tr("Select the value below"));
}

void SelectionArrow::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    // A flat face; our own glow under the mouse and when held down, as the calculator's buttons have
    Colour face = colours::arrowFace;
    if (isDown()) {
        face = colours::buttonPressedMiddle;
    } else if (underMouse()) {
        face = colours::buttonHoverMiddle;
    }
    painter.setPen(Qt::NoPen);
    painter.setBrush(qtColour(face));
    painter.drawRoundedRect(QRectF(rect()), arrowCornerRadius, arrowCornerRadius);
    const double left = arrowLeft;
    const double top = arrowTop;
    QPolygonF triangle;
    if (m_pointsUp) {
        triangle << QPointF(left, top + arrowHeight) << QPointF(left + arrowWidth, top + arrowHeight)
                 << QPointF(left + (arrowWidth / 2.0), top);
    } else {
        triangle << QPointF(left, top) << QPointF(left + arrowWidth, top) << QPointF(left + (arrowWidth / 2.0), top + arrowHeight);
    }
    painter.setBrush(qtColour(colours::arrowGlyph));
    painter.drawPolygon(triangle);
}

}  // namespace chrono_calc
