#pragma once

#include <QAbstractButton>
#include <QString>

#include "core/layout.h"

#include <string_view>

class QPaintEvent;
class QTimer;
class QWidget;

// --- Version ------------------------------------------------------------
// src/ui/calcbutton.h 0.2.7

// --- Version history ----------------------------------------------------
// v0.2.7: The faces of Windows 7 (digit, function, memory, toggle), the greyed look, the label under Inv
// v0.1.6: Initial: a calculator button in our own drawing, close to the Windows 7 Calculator (D-005, D-018)

namespace chrono_calc {

// One calculator button: it runs one command and is drawn by us (a soft face, a golden glow under the mouse, a deeper
// orange when pressed). It never takes the keyboard focus: the keys belong to the whole calculator.
class CalcButton : public QAbstractButton
{
    Q_OBJECT

public:
    explicit CalcButton(const ButtonSpec& spec, QWidget* parent);

    [[nodiscard]] std::string_view commandId() const;

    // Shows the button pressed for a moment, as when its key is typed on the keyboard. Only the drawing changes: the
    // button's own pressed state belongs to the mouse, so a click the mouse is making is never disturbed.
    void flash();
    // Whether the button is drawn pressed: held by the mouse, or flashing after its key
    [[nodiscard]] bool looksPressed() const;
    // Shows the label of the inverse form while Inv is on (sin becomes sin⁻¹); a button without one keeps its label
    void showInverse(bool inverse);
    // A toggle (Inv, F-E) shows whether it is on
    void showOn(bool on);
    [[nodiscard]] bool isOn() const;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::string_view m_commandId;
    ButtonFace m_face;
    QString m_label;
    QString m_inverseLabel;
    bool m_on = false;
    QTimer* m_flashTimer = nullptr;
};

}  // namespace chrono_calc
