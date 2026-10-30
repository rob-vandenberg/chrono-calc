#include "ui/look.h"

#include <QApplication>
#include <QPalette>
#include <QStyleFactory>
#include <QToolTip>
#include <QtGlobal>

#include "core/colours.h"

// --- Version ------------------------------------------------------------
// src/ui/look.cpp 0.1.6

// --- Version history ----------------------------------------------------
// v0.1.6: Initial: Qt colours from colours.h, and the program's own look on every desktop (D-018)

namespace chrono_calc {

void applyLook()
{
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    // A palette made from our own colour only, never the desktop's; every role we use is set below
    QPalette palette(qtColour(colours::menuBackground));
    const auto both = [&palette](QPalette::ColorRole role, Colour colour) {
        palette.setColor(QPalette::Active, role, qtColour(colour));
        palette.setColor(QPalette::Inactive, role, qtColour(colour));
        palette.setColor(QPalette::Disabled, role, qtColour(colour));
    };
    both(QPalette::Window, colours::menuBackground);
    both(QPalette::Base, colours::menuBackground);
    both(QPalette::Button, colours::menuBackground);
    both(QPalette::WindowText, colours::menuText);
    both(QPalette::Text, colours::menuText);
    both(QPalette::ButtonText, colours::menuText);
    both(QPalette::Highlight, colours::menuHighlight);
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    both(QPalette::Accent, colours::menuHighlight);
#endif
    both(QPalette::AlternateBase, colours::menuBackground);
    both(QPalette::Midlight, colours::menuBackground);
    both(QPalette::BrightText, colours::menuText);
    both(QPalette::PlaceholderText, colours::menuDisabledText);
    both(QPalette::Link, colours::menuText);
    both(QPalette::LinkVisited, colours::menuText);
    both(QPalette::HighlightedText, colours::menuText);
    both(QPalette::ToolTipBase, colours::toolTipBackground);
    both(QPalette::ToolTipText, colours::toolTipText);
    both(QPalette::Light, colours::toolTipBackground);
    both(QPalette::Mid, colours::menuBorder);
    both(QPalette::Dark, colours::menuBorder);
    both(QPalette::Shadow, colours::menuBorder);
    // Greyed menu items: the items that are not built yet
    for (const auto role : {QPalette::WindowText, QPalette::Text, QPalette::ButtonText}) {
        palette.setColor(QPalette::Disabled, role, qtColour(colours::menuDisabledText));
    }
    palette.setColor(QPalette::Disabled, QPalette::Light, qtColour(colours::menuBackground));
    QApplication::setPalette(palette);
    // The hover texts get the theme's own palette at the start; they take ours as well
    QToolTip::setPalette(palette);
}

}  // namespace chrono_calc
