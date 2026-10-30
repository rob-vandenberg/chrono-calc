#pragma once

#include <QFont>
#include <QString>
#include <QWidget>

#include "core/layout.h"

class QPaintEvent;

// --- Version ------------------------------------------------------------
// src/ui/displaypanel.h 0.4.11

// --- Version history ----------------------------------------------------
// v0.4.11: The Statistics view: no expression line; the count of the list and the thin line above it
// v0.2.7: The measures of the current view (Standard or Scientific)
// v0.1.6: Initial: the display of the Standard view, close to the Windows 7 Calculator (D-005, D-018)

namespace chrono_calc {

// The calculator's display: the expression line at the top, the memory mark "M" at the left and the number (or the
// error text) in large figures at the right. It shows what it is given and computes nothing.
class DisplayPanel : public QWidget
{
    Q_OBJECT

public:
    explicit DisplayPanel(QWidget* parent);

    // The measures of the view the display belongs to (its frame, the expression line, the M and the number)
    void setViewLayout(const ViewLayout& layout);

    void showValues(const QString& number, const QString& expression, bool memoryHolds);
    // The Statistics view's "Count = " and how many values its list holds (shown only in that view)
    void showCount(int count);

    [[nodiscard]] QString number() const;
    [[nodiscard]] QString expression() const;
    [[nodiscard]] bool memoryMarkShown() const;
    [[nodiscard]] QString countText() const;
    // The pixel size the number is drawn in: smaller for a long number, so that every digit stays in view
    [[nodiscard]] int numberPixelSize() const;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    const ViewLayout* m_layout;
    QFont m_numberFont;
    QFont m_expressionFont;
    QFont m_memoryMarkFont;
    QFont m_countFont;
    QString m_number;
    QString m_expression;
    bool m_memoryHolds = false;
    int m_count = 0;
};

}  // namespace chrono_calc
