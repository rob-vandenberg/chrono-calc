#pragma once

#include <QAbstractButton>
#include <QString>
#include <QWidget>

#include <string_view>

class QPaintEvent;

// --- Version ------------------------------------------------------------
// src/ui/scientificparts.h 0.2.7

// --- Version history ----------------------------------------------------
// v0.2.7: Initial: the Scientific view's angle units, their frame and the bracket counter (D-020)

namespace chrono_calc {

// The flat frame around the angle units
class AngleFrame : public QWidget
{
    Q_OBJECT

public:
    explicit AngleFrame(QWidget* parent);

protected:
    void paintEvent(QPaintEvent* event) override;
};

// One angle unit (Degrees, Radians, Grads): a round choice with its name; the chosen one has a dot. It runs its
// command when clicked and shows what the calculator says is chosen.
class AngleButton : public QAbstractButton
{
    Q_OBJECT

public:
    AngleButton(std::string_view commandId, const QString& label, QWidget* parent);

    [[nodiscard]] std::string_view commandId() const;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::string_view m_commandId;
};

// The bracket counter: a flat cell that shows "(=1" while a bracket is open, and nothing otherwise
class BracketCounter : public QWidget
{
    Q_OBJECT

public:
    explicit BracketCounter(QWidget* parent);

    void showCount(unsigned int openBrackets);
    [[nodiscard]] QString text() const;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QString m_text;
};

}  // namespace chrono_calc
