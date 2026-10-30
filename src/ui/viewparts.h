#pragma once

#include <QAbstractButton>
#include <QString>
#include <QWidget>

#include "core/layout.h"

#include <string>
#include <string_view>

class QMouseEvent;
class QPaintEvent;

// --- Version ------------------------------------------------------------
// src/ui/viewparts.h 0.3.8

// --- Version history ----------------------------------------------------
// v0.3.8: Choices of any kind (bases and word sizes too) and the Programmer view's bit display (D-021); was
//         scientificparts
// v0.2.7: Initial: the Scientific view's angle units, their frame and the bracket counter (D-020)

namespace chrono_calc {

// The flat frame around a group of choices
class FlatFrame : public QWidget
{
    Q_OBJECT

public:
    explicit FlatFrame(QWidget* parent);

protected:
    void paintEvent(QPaintEvent* event) override;
};

// One round choice (an angle unit, a number base, a word size) with its name; the chosen one has a dot. It runs its
// command when clicked and shows what the calculator says is chosen.
class ChoiceButton : public QAbstractButton
{
    Q_OBJECT

public:
    ChoiceButton(std::string_view commandId, const QString& label, QWidget* parent);

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

// The Programmer view's bit display: the 64 bits of the number in two rows, with their positions; only the bits of the
// word size show. A click on a bit asks to turn it over (bitClicked).
class BitDisplay : public QWidget
{
    Q_OBJECT

public:
    BitDisplay(const BitDisplaySpec& spec, QWidget* parent);

    // The bits, bit 63 first ('0' or '1'), and how many of them the word has
    void showBits(const std::string& bits, int wordBits);
    [[nodiscard]] std::string bits() const;
    [[nodiscard]] int visibleBits() const;
    // Where a bit sits in the display's own coordinates
    [[nodiscard]] QRect bitArea(int bit) const;

signals:
    void bitClicked(int bit);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    BitDisplaySpec m_spec;
    std::string m_bits;
    int m_wordBits = 64;
};

}  // namespace chrono_calc
