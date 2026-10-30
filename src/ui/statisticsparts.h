#pragma once

#include <QAbstractButton>
#include <QLineEdit>
#include <QString>
#include <QStringList>
#include <QWidget>

#include "core/layout.h"

class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class QWheelEvent;

// --- Version ------------------------------------------------------------
// src/ui/statisticsparts.h 0.5.12

// --- Version history ----------------------------------------------------
// v0.5.12: The list also shows the history (its own keys, a double click edits); the edit box over a row (D-024)
// v0.4.11: Initial: the Statistics view's list of values and the buttons that move its selection (D-023)

namespace chrono_calc {

// The Statistics view's list of values, inside the display: five rows, each value at the right, the selected one light
// blue. It shows what the calculator gives and keeps the selected row in view. A click selects a row and gives the
// list the keyboard focus; while it has the focus, the arrow keys, Home, End, Page Up and Page Down move the selection
// and Delete removes the selected value (Windows 7). Every other key goes on to the window.
// The same list shows the history of the Standard and Scientific views (windows7-history.md part C): there Page Up and
// Page Down move by one, as the arrow keys do, Delete does nothing, and a double click opens the entry for editing.
class StatisticsList : public QWidget
{
    Q_OBJECT

public:
    StatisticsList(const StatisticsSpec& spec, bool history, QWidget* parent);
    // Where a row is now, in the list's own coordinates (for the edit box over it); empty when it is out of view
    [[nodiscard]] QRect rowArea(int index) const;
    // Brings a row into view
    void scrollTo(int index);
    // While an edit box lies over a row, the wheel does not scroll the rows under it
    void setScrollLocked(bool locked);

    // The values, the oldest first, and the selected one (-1: none)
    void showValues(const QStringList& values, int selected);
    [[nodiscard]] QStringList values() const;
    [[nodiscard]] int selected() const;
    // The first row in view
    [[nodiscard]] int firstVisible() const;
    [[nodiscard]] int visibleRows() const;

signals:
    // A row to select (a click, Home, End, Page Up, Page Down)
    void indexChosen(int index);
    // The arrow keys: the row above or below
    void previousChosen();
    void nextChosen();
    // Delete: the selected value goes
    void deleteChosen();
    // A double click on a history entry: it is chosen and opened for editing
    void editChosen(int index);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void keepInRange();

    StatisticsSpec m_spec;
    bool m_history;
    bool m_scrollLocked = false;
    QStringList m_values;
    int m_selected = -1;
    int m_firstVisible = 0;
};

// The edit box over a row of the list (Edit, F2): the text at the right, without a frame, as Windows 7 has it. Enter
// (and = in the history) takes the text, Escape drops the edit; every other key edits the text.
class ListEditBox : public QLineEdit
{
    Q_OBJECT

public:
    ListEditBox(const QString& text, bool equalsTakes, QWidget* parent);

signals:
    void taken(const QString& text);
    void dropped();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    bool m_equalsTakes;
};

// One of the two small flat buttons at the top of the display that move the list's selection up or down, with a
// filled triangle (Windows 7: 9 px wide, 5 px high)
class SelectionArrow : public QAbstractButton
{
    Q_OBJECT

public:
    SelectionArrow(bool pointsUp, QWidget* parent);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    bool m_pointsUp;
};

}  // namespace chrono_calc
