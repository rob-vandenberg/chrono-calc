#pragma once

#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QPointer>
#include <QString>
#include <QTimer>
#include <QWidget>

#include "core/datecalculator.h"
#include "core/unitconverter.h"
#include "core/worksheetcalculator.h"
#include "core/worksheets.h"

#include <array>
#include <cstddef>
#include <vector>

class QEvent;
class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class QRadioButton;

// --- Version ------------------------------------------------------------
// src/ui/sidepanels.h 0.8.13

// --- Version history ----------------------------------------------------
// v0.8.13: Initial: the side panel beside the calculator, its lists and boxes, and the Unit conversion panel (D-025);
//          the date boxes, number boxes with arrows, the panel's button and the Date calculation panel (D-026); the
//          Worksheet panel and its message balloon (D-027)

namespace chrono_calc {

// The panel beside the calculator (View > Unit conversion, Date calculation, Worksheets): flat white inside a thin
// frame (windows7-unitconversion.md part A). Each side panel puts its own labels, lists and boxes on it.
class SidePanelFrame : public QWidget
{
public:
    using QWidget::QWidget;

protected:
    void paintEvent(QPaintEvent* event) override;
};

// A drop-down list of a side panel (not editable), in our own drawing: a light grey face, a grey edge, a dark arrow.
// Up and Down change the choice at once and the list opens with Alt+Down or F4; every other key goes on to the
// calculator, and Enter and Escape give it the keyboard focus as well (Windows 7, part D line 7).
class PanelList : public QComboBox
{
    Q_OBJECT

public:
    explicit PanelList(QWidget* parent);

signals:
    // Enter or Escape: the calculator takes the keyboard focus (and the key)
    void focusToCalculator();

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
};

// A box of a side panel: white with a grey edge, blue while it has the keyboard focus; read-only boxes are light grey.
// The keys it does not use as an edit box (F9, Enter, Escape, the calculator's letters) do nothing, so a key typed
// into it never reaches the calculator (Windows 7, part D line 4).
class PanelBox : public QLineEdit
{
public:
    explicit PanelBox(bool readOnly, QWidget* parent);

    // A typed character beyond this many is refused (Windows 7's edit limit); a pasted text is not cut here, the
    // panel's calculator filters it first and then keeps that many characters
    void limitTyping(int characters);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    int m_typingLimit = 0;
};

// A date box of the Date calculation panel (windows7-datecalculation.md part C): the regional short date with a
// four-digit year, from 1 January 1601 to 31 December 9999; Left and Right move between day, month and year, Up and
// Down change the part (the day and month go round, the year stops at its ends), digits are typed per part; F4, Alt+Down
// or a click on the arrow open the calendar. Enter asks for Calculate; no key typed into it reaches the calculator.
class PanelDate : public QDateEdit
{
    Q_OBJECT

public:
    explicit PanelDate(QWidget* parent);

    void stepBy(int steps) override;

signals:
    void calculateChosen();

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    // Tab and Shift+Tab move to the next part of the panel, not to the next part of the date (Windows 7)
    bool focusNextPrevChild(bool next) override;
};

// A number box of the Date calculation panel: up to three digits; Up and Down step it like its arrows, Enter asks for
// Calculate; no key typed into it reaches the calculator (part E line 1, part F line 2)
class PanelNumberBox : public PanelBox
{
    Q_OBJECT

public:
    explicit PanelNumberBox(QWidget* parent);

signals:
    void stepped(bool up);
    void calculateChosen();

protected:
    void keyPressEvent(QKeyEvent* event) override;
};

// The two arrows beside a number box: a click on the upper half steps it up, on the lower half down; they never take
// the keyboard focus
class PanelArrows : public QWidget
{
    Q_OBJECT

public:
    explicit PanelArrows(QWidget* parent);

signals:
    void stepped(bool up);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
};

// A button of a side panel (Calculate): light grey with the blue edge of the button Enter presses. Enter and Space
// press it; every other key goes on to the calculator.
class PanelButton : public QPushButton
{
public:
    PanelButton(const QString& text, QWidget* parent);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
};

// The Unit conversion panel (windows7-unitconversion.md): the unit type, the From box and unit, the To box and unit.
// It shows what the calculator's UnitConverter holds and reports what the user chose or typed; it computes nothing.
class UnitConversionPanel : public SidePanelFrame
{
    Q_OBJECT

public:
    explicit UnitConversionPanel(QWidget* parent);

    // The type, the units and both boxes as the converter has them
    void showState(const UnitConverter& converter);
    // The parts in the Tab order of Windows 7 (part D line 7): From, its unit, To, its unit; then the calculator, then
    // the type list
    [[nodiscard]] QWidget* fromBox() const;
    [[nodiscard]] QWidget* fromList() const;
    [[nodiscard]] QWidget* toBox() const;
    [[nodiscard]] QWidget* toList() const;
    [[nodiscard]] QWidget* typeList() const;

signals:
    void typeChosen(int type);
    void fromChosen(int unit);
    void toChosen(int unit);
    // The From box's text as the user left it; the converter says what the box takes of it
    void inputEdited(const QString& text);
    void focusToCalculator();

private:
    void fillUnits(int type);

    PanelList* m_typeList;
    PanelBox* m_fromBox;
    PanelList* m_fromList;
    PanelBox* m_toBox;
    PanelList* m_toList;
    int m_shownType = -1;
};

// The Date calculation panel (windows7-datecalculation.md): the mode list, and below it the parts of one mode: the
// difference between two dates (From, To, the difference in years, months, weeks and days, and in days) or a date with
// years, months and days added or subtracted. It shows what the calculator's DateCalculator holds and reports what the
// user chose or typed; it computes nothing.
class DateCalculationPanel : public SidePanelFrame
{
    Q_OBJECT

public:
    explicit DateCalculationPanel(QWidget* parent);

    // The mode, dates, numbers and results as the calculator has them; the result dates in the regional long date
    void showState(const DateCalculator& calculator);

    // The parts in the Tab order of Windows 7 (part F line 1); the parts of the mode not shown are skipped
    [[nodiscard]] std::vector<QWidget*> tabOrder() const;
    [[nodiscard]] QWidget* modeList() const;

protected:
    // The arrows on Add and Subtract choose one (Left and Up: Add; Right and Down: Subtract, part E line 4)
    bool eventFilter(QObject* watched, QEvent* event) override;

signals:
    void modeChosen(int mode);
    void differenceFromChosen(QDate date);
    void differenceToChosen(QDate date);
    void shiftFromChosen(QDate date);
    void subtractChosen(bool subtract);
    void fieldEdited(int field, const QString& text);
    void fieldStepped(int field, bool up);
    void calculateChosen();
    void focusToCalculator();

private:
    // Calculate, then the keyboard focus to the first result box (Windows 7: Enter in a date or number box)
    void calculateFromKey();

    PanelList* m_modeList;
    QWidget* m_differencePart;
    QWidget* m_shiftPart;
    PanelDate* m_differenceFrom;
    PanelDate* m_differenceTo;
    PanelBox* m_difference;
    PanelBox* m_totalDays;
    PanelDate* m_shiftFrom;
    QRadioButton* m_add;
    QRadioButton* m_subtract;
    std::array<PanelNumberBox*, 3> m_fields{};
    PanelBox* m_shifted;
    PanelButton* m_calculate;
};

// The balloon of a Worksheet's message ("This value cannot be blank."), below the box it points at, in our own drawing
// (windows7-worksheets.md part B line 4, part D line 4): it goes after 8.7 seconds, when the box loses the keyboard
// focus, with a key that moves the cursor in the box, or when the window moves; typing into the box keeps it
class PanelBalloon : public QWidget
{
public:
    explicit PanelBalloon(QWidget* parent);

    // Shown below "box" with the text; any balloon shown before goes
    void showAt(QWidget* box, const QString& text);
    void dismiss();

protected:
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QString m_text;
    QPointer<QWidget> m_box;
    QPointer<QWidget> m_window;
    QTimer m_timer;
};

// A Worksheet (windows7-worksheets.md): the list of the values it can calculate, a box for every other value with its
// label, Calculate and the result box. Choosing another value only relabels the same boxes (part A line 3). It shows
// what the calculator's WorksheetCalculator holds and reports what the user chose or typed; it computes nothing.
class WorksheetPanel : public SidePanelFrame
{
    Q_OBJECT

public:
    WorksheetPanel(Worksheet worksheet, QWidget* parent);

    // The chosen value, the labels, the boxes' texts and the result as the calculator has them
    void showState(const WorksheetCalculator& calculator);
    // After Calculate: the keyboard focus to the box the message points at, or to the result box, and the balloon of
    // the message (part B lines 2 and 4)
    void showCalculated(const WorksheetCalculator& calculator);

    // The parts in the Tab order of Windows 7 (part F line 1): the boxes, Calculate, the result; the window puts the
    // calculator and then the list after them
    [[nodiscard]] std::vector<QWidget*> tabOrder() const;
    [[nodiscard]] QWidget* choiceList() const;

signals:
    void choiceChosen(int item);
    // The text of the box of value "value" (its place in the worksheet's own order) as the user left it
    void valueEdited(int value, const QString& text);
    void calculateChosen();
    // Escape on the list: the calculator takes the keyboard focus (and the key), as in the other panels
    void focusToCalculator();

protected:
    // Enter in a box or in the list is Calculate; the list keeps every other key from the calculator (part F line 2)
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    Worksheet m_worksheet;
    PanelList* m_choiceList;
    std::vector<QLabel*> m_labels;
    std::vector<PanelBox*> m_boxes;
    // The value each box shows, as the calculator last said
    std::vector<std::size_t> m_boxValues;
    PanelButton* m_calculate;
    PanelBox* m_result;
    PanelBalloon* m_balloon;
};

}  // namespace chrono_calc
