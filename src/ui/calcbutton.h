#pragma once

#include <QAbstractButton>
#include <QString>

#include <string_view>

class QPaintEvent;
class QTimer;
class QWidget;

// --- Version ------------------------------------------------------------
// src/ui/calcbutton.h 0.1.6

// --- Version history ----------------------------------------------------
// v0.1.6: Initial: a calculator button in our own drawing, close to the Windows 7 Calculator (D-005, D-018)

namespace chrono_calc {

// One calculator button: it runs one command and is drawn by us (a soft face, a golden glow under the mouse, a deeper
// orange when pressed). It never takes the keyboard focus: the keys belong to the whole calculator.
class CalcButton : public QAbstractButton
{
    Q_OBJECT

public:
    CalcButton(std::string_view commandId, const QString& label, QWidget* parent);

    [[nodiscard]] std::string_view commandId() const;

    // Shows the button pressed for a moment, as when its key is typed on the keyboard. Only the drawing changes: the
    // button's own pressed state belongs to the mouse, so a click the mouse is making is never disturbed.
    void flash();
    // Whether the button is drawn pressed: held by the mouse, or flashing after its key
    [[nodiscard]] bool looksPressed() const;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::string_view m_commandId;
    QTimer* m_flashTimer = nullptr;
};

}  // namespace chrono_calc
