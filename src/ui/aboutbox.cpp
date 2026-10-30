#include "ui/aboutbox.h"

#include <QDialog>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QPalette>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

#include "core/colours.h"
#include "core/version.h"
#include "ui/fonts.h"
#include "ui/look.h"
#include "ui/programicon.h"
#include "ui/sidepanels.h"

#include <string_view>

// --- Version ------------------------------------------------------------
// src/ui/aboutbox.cpp 0.9.14

// --- Version history ----------------------------------------------------
// v0.9.14: Initial: the About box (D-028)

namespace chrono_calc {

namespace {

// The measures of the box (our own, close to Windows 10's About box)
constexpr int boxWidth = 480;
constexpr int bandHeight = 96;
constexpr int bandIconSize = 64;
constexpr int smallIconSize = 32;
constexpr int margin = 20;
constexpr int textWidth = boxWidth - (3 * margin) - smallIconSize;
constexpr double titleInkHeight = 22.0;
constexpr double textInkHeight = 9.0;
constexpr int okWidth = 75;
constexpr int okHeight = 23;

QString fromView(std::string_view text)
{
    return QString::fromUtf8(text.data(), static_cast<qsizetype>(text.size()));
}

QLabel* textLabel(const QString& text, QWidget* parent)
{
    auto* label = new QLabel(text, parent);
    label->setFont(fontWithInkHeight(label->font(), textInkHeight));
    label->setFixedWidth(textWidth);
    label->setWordWrap(true);
    return label;
}

// A label with a link that opens in the browser
QLabel* linkLabel(const QString& html, QWidget* parent)
{
    QLabel* label = textLabel(html, parent);
    label->setTextFormat(Qt::RichText);
    label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    label->setOpenExternalLinks(true);
    return label;
}

QLabel* iconLabel(int size, QWidget* parent)
{
    auto* label = new QLabel(parent);
    label->setPixmap(programIconPixmap(size, parent->devicePixelRatioF()));
    label->setFixedSize(size, size);
    return label;
}

}  // namespace

AboutBox::AboutBox(QWidget* parent)
    : QDialog(parent)
{
    setObjectName(QStringLiteral("about.box"));
    setWindowTitle(tr("About Chrono Calc"));
    QPalette colours = palette();
    for (const auto group : {QPalette::Active, QPalette::Inactive, QPalette::Disabled}) {
        colours.setColor(group, QPalette::Window, qtColour(colours::aboutFace));
        colours.setColor(group, QPalette::WindowText, qtColour(colours::aboutText));
        colours.setColor(group, QPalette::Text, qtColour(colours::aboutText));
        colours.setColor(group, QPalette::Link, qtColour(colours::aboutLink));
        colours.setColor(group, QPalette::LinkVisited, qtColour(colours::aboutLink));
    }
    setPalette(colours);
    setAutoFillBackground(true);

    // The top band: the icon large and the name in large blue letters
    auto* band = new QWidget(this);
    band->setFixedSize(boxWidth, bandHeight);
    auto* bandLayout = new QHBoxLayout(band);
    bandLayout->setContentsMargins(margin, 0, margin, 0);
    bandLayout->setSpacing(margin);
    bandLayout->addWidget(iconLabel(bandIconSize, this));
    auto* title = new QLabel(QStringLiteral("Chrono Calc"), band);
    title->setObjectName(QStringLiteral("about.title"));
    title->setFont(fontWithInkHeight(title->font(), titleInkHeight));
    QPalette titleColours = colours;
    titleColours.setColor(QPalette::WindowText, qtColour(colours::aboutTitle));
    title->setPalette(titleColours);
    bandLayout->addWidget(title, 1);

    // The thin grey line below the band
    auto* line = new QWidget(this);
    line->setFixedSize(boxWidth, 1);
    QPalette lineColours = colours;
    lineColours.setColor(QPalette::Window, qtColour(colours::aboutLine));
    line->setPalette(lineColours);
    line->setAutoFillBackground(true);

    // The small icon beside the name, the version, the copyright, the licence, the disclaimer and the project's page
    auto* text = new QVBoxLayout;
    text->setSpacing(4);
    text->addWidget(textLabel(QStringLiteral("Chrono Calc"), this));
    QLabel* version = textLabel(tr("Version %1").arg(fromView(versionText())), this);
    version->setObjectName(QStringLiteral("about.version"));
    text->addWidget(version);
    text->addWidget(textLabel(QStringLiteral("© 2026 Rob Vandenberg"), this));
    text->addSpacing(12);
    QLabel* licence = linkLabel(tr("Chrono Calc is free software: you can redistribute it and/or change it under the terms "
                                   "of the <a href=\"%1\">GNU Affero General Public License</a>, version 3 or later.")
                                    .arg(fromView(licenceAddress())),
                                this);
    licence->setObjectName(QStringLiteral("about.licence"));
    text->addWidget(licence);
    text->addSpacing(8);
    text->addWidget(textLabel(tr("This program comes with ABSOLUTELY NO WARRANTY, without even the implied warranty of "
                                 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. In no event shall the author be "
                                 "liable for any damage arising from its use."),
                              this));
    text->addSpacing(8);
    const QString address = fromView(repositoryAddress());
    QLabel* repository = linkLabel(QStringLiteral("<a href=\"%1\">%1</a>").arg(address), this);
    repository->setObjectName(QStringLiteral("about.repository"));
    text->addWidget(repository);
    auto* body = new QHBoxLayout;
    body->setContentsMargins(margin, margin, margin, 0);
    body->setSpacing(margin);
    body->addWidget(iconLabel(smallIconSize, this), 0, Qt::AlignTop);
    body->addLayout(text);

    // OK at the bottom right: a click, Enter or Space on it, and Escape anywhere close the box
    auto* ok = new PanelButton(tr("OK"), this);
    ok->setObjectName(QStringLiteral("about.ok"));
    ok->setFixedSize(okWidth, okHeight);
    // Enter anywhere in the box presses OK (a link with the keyboard focus opens instead)
    ok->setDefault(true);
    connect(ok, &PanelButton::clicked, this, &QDialog::accept);
    auto* buttons = new QHBoxLayout;
    buttons->setContentsMargins(margin, margin, margin, margin);
    buttons->addStretch(1);
    buttons->addWidget(ok);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(band);
    layout->addWidget(line);
    layout->addLayout(body);
    layout->addLayout(buttons);
    // A fixed size: the box cannot be resized
    layout->setSizeConstraint(QLayout::SetFixedSize);
    ok->setFocus();
}

}  // namespace chrono_calc
