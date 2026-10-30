#pragma once

#include <QFont>
#include <QString>
#include <QWidget>

class QPaintEvent;

// --- Version ------------------------------------------------------------
// src/ui/displaypanel.h 0.1.6

// --- Version history ----------------------------------------------------
// v0.1.6: Initial: the display of the Standard view, close to the Windows 7 Calculator (D-005, D-018)

namespace chrono_calc {

// The calculator's display: the expression line at the top, the memory mark "M" at the left and the number (or the
// error text) in large figures at the right. It shows what it is given and computes nothing.
class DisplayPanel : public QWidget
{
    Q_OBJECT

public:
    explicit DisplayPanel(QWidget* parent);

    void showValues(const QString& number, const QString& expression, bool memoryHolds);

    [[nodiscard]] QString number() const;
    [[nodiscard]] QString expression() const;
    [[nodiscard]] bool memoryMarkShown() const;
    // The pixel size the number is drawn in: smaller for a long number, so that every digit stays in view
    [[nodiscard]] int numberPixelSize() const;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QFont m_numberFont;
    QFont m_expressionFont;
    QFont m_memoryMarkFont;
    QString m_number;
    QString m_expression;
    bool m_memoryHolds = false;
};

}  // namespace chrono_calc
