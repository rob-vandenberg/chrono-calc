#include "ui/mainwindow.h"

#include <QAction>
#include <QActionGroup>
#include <QChar>
#include <QCloseEvent>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QKeySequence>
#include <QLatin1String>
#include <QLayout>
#include <QLinearGradient>
#include <QLocale>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QPaintEvent>
#include <QPainter>
#include <QPoint>
#include <QRect>
#include <QScreen>
#include <QSettings>
#include <QSize>
#include <QString>
#include <QVariant>
#include <QWidget>

#include "core/calculator.h"
#include "core/commands.h"
#include "core/colours.h"
#include "core/enginetexts.h"
#include "core/keys.h"
#include "core/menus.h"
#include "core/layout.h"
#include "core/view.h"
#include "ui/calcbutton.h"
#include "ui/displaypanel.h"
#include "ui/keyboardlayouts.h"
#include "ui/look.h"
#include "ui/viewparts.h"

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/ui/mainwindow.cpp 0.3.10

// --- Version history ----------------------------------------------------
// v0.3.10: On a layout that is not Latin a key's place letter comes first, a sign only where the view has no command
// v0.3.9: A letter of another script (Russian, Greek, Arabic) is read as the Latin letter at its key's position
// v0.3.8: The Programmer view: its size, the bases and word sizes, the bit display, the digits each base allows
// v0.2.7: The Scientific view; View > Standard and Scientific switch the buttons and the window size; the view is
//         remembered; Inv labels, the toggles, the angle units and the bracket counter follow the calculator (D-020)
// v0.1.6: The Standard view: the menus, the display, the buttons and the keys, a fixed size and the position
//         remembered (tasks 1.1 to 1.4, D-018)
// v0.0.2: Initial: an empty window titled Chrono Calc

namespace chrono_calc {

namespace {

const QString positionKey = QStringLiteral("window/position");
const QString viewKey = QStringLiteral("window/view");

const QString scientificName = QStringLiteral("scientific");
const QString programmerName = QStringLiteral("programmer");
const QString standardName = QStringLiteral("standard");

QString nameOf(View view)
{
    switch (view) {
    case View::Scientific: return scientificName;
    case View::Programmer: return programmerName;
    case View::Standard: break;
    }
    return standardName;
}

// Where the window's gradient changes, in pixels from the top (Windows 7, windows7-standard.md part G)
constexpr double gradientUpper = 79.0;
constexpr double gradientBottom = 165.0;

QString fromView(std::string_view text)
{
    return QString::fromUtf8(text.data(), static_cast<qsizetype>(text.size()));
}

// The area below the menus: a soft blue running from top to bottom, behind the display and the buttons
class ViewArea : public QWidget
{
public:
    using QWidget::QWidget;

protected:
    void paintEvent(QPaintEvent* /*event*/) override
    {
        QPainter painter(this);
        QLinearGradient gradient(0, 0, 0, height());
        gradient.setColorAt(0.0, qtColour(colours::windowTop));
        // Measured in Windows 7: the upper colour at y 79, the bottom colour from y 165 down
        gradient.setColorAt(gradientUpper / height(), qtColour(colours::windowUpper));
        gradient.setColorAt(gradientBottom / height(), qtColour(colours::windowBottom));
        gradient.setColorAt(1.0, qtColour(colours::windowBottom));
        painter.fillRect(rect(), gradient);
    }
};

QRect toRect(Rectangle rectangle)
{
    return {rectangle.left, rectangle.top, rectangle.width, rectangle.height};
}

// The Latin letter key a key stands for on a layout that is not Latin (Russian, Greek, Hebrew, Thai, Arabic): the
// letter of its place, as Windows 7 reads its letter keys by their place (tests/reference/windows7-programmer.md part
// F). Nothing for a key of a Latin layout (AZERTY, QWERTZ, Dvorak), which is read as it is.
// Windows: the virtual key, which on such a layout is the letter of the key's place.
// Linux (XKB, also in the tests): the place is the XKB key code, the PC scan code plus 8. The layout's own key symbol
// tells the script, because with Ctrl Qt gives the Latin letter of another layout in use (Ctrl+я is Y with German and
// Russian; Windows 7 reads Z); a sign on a letter's place (the ; of the Greek Q key, the , of the AZERTY M key) is
// looked up in the layouts in use (KeyboardLayouts). Any other platform: nothing.
std::optional<int> placeKeyOf(const QKeyEvent& event)
{
    const int key = event.key();
    const QString platform = QGuiApplication::platformName();
    if (platform == QLatin1String("windows")) {
        const std::uint32_t virtualKey = event.nativeVirtualKey();
        if (key >= Qt::Key_A && key <= Qt::Key_Z) {
            return std::nullopt;
        }
        if (virtualKey >= 'A' && virtualKey <= 'Z') {
            return Qt::Key_A + static_cast<int>(virtualKey - 'A');
        }
        return std::nullopt;
    }
    if (platform != QLatin1String("xcb") && !platform.startsWith(QLatin1String("wayland")) &&
        platform != QLatin1String("offscreen")) {
        return std::nullopt;
    }
    constexpr std::uint32_t xkbOffset = 8;
    const std::uint32_t keyCode = event.nativeScanCode();
    if (keyCode < xkbOffset) {
        return std::nullopt;
    }
    const std::optional<char32_t> letter = latinLetterAtScanCode(keyCode - xkbOffset);
    if (!letter) {
        return std::nullopt;
    }
    const std::uint32_t keySymbol = event.nativeVirtualKey();
    char32_t character = key > 0 ? static_cast<char32_t>(key) : U'\0';
    if (keySymbol != 0) {
        character = characterOfKeySymbol(keySymbol);
    }
    bool nonLatin = isOtherScriptLetter(character);
    if (!nonLatin && keySymbol != 0 && character <= QChar::LastValidCodePoint && !QChar::isLetter(character)) {
        nonLatin = KeyboardLayouts::system().isFromNonLatinLayout({.code = keyCode, .symbol = keySymbol});
    }
    if (!nonLatin) {
        return std::nullopt;
    }
    return Qt::Key_A + static_cast<int>(*letter - U'a');
}

std::optional<Key> keyOf(int qtKey)
{
    switch (qtKey) {
    case Qt::Key_Escape: return Key::Escape;
    case Qt::Key_Delete: return Key::Delete;
    case Qt::Key_Backspace: return Key::Backspace;
    case Qt::Key_Return:
    case Qt::Key_Enter: return Key::Enter;
    case Qt::Key_F2: return Key::F2;
    case Qt::Key_F3: return Key::F3;
    case Qt::Key_F4: return Key::F4;
    case Qt::Key_F5: return Key::F5;
    case Qt::Key_F6: return Key::F6;
    case Qt::Key_F7: return Key::F7;
    case Qt::Key_F8: return Key::F8;
    case Qt::Key_F9: return Key::F9;
    case Qt::Key_F12: return Key::F12;
    default: return std::nullopt;
    }
}

}  // namespace

MainWindow::MainWindow(QWidget* parent)
    : MainWindow(systemSeparators(), parent)
{
}

MainWindow::MainWindow(Separators separators, QWidget* parent)
    : QMainWindow(parent)
    , m_calculator(separators)
    , m_separators(separators)
    , m_area(new ViewArea(this))
    , m_display(new DisplayPanel(m_area))
{
    setWindowTitle(QStringLiteral("Chrono Calc"));
    buildMenus();
    m_area->setFocusPolicy(Qt::StrongFocus);
    setCentralWidget(m_area);
    m_display->setObjectName(QStringLiteral("display"));
    // The view the calculator was closed in, as Windows 7 remembers it
    const QString savedView = QSettings().value(viewKey).toString();
    if (savedView == scientificName) {
        m_calculator.run("view.scientific");
    } else if (savedView == programmerName) {
        m_calculator.run("view.programmer");
    }
    buildView(m_calculator.view());
    // The window has the size of its view, as in Windows 7: it cannot be resized or maximised
    layout()->setSizeConstraint(QLayout::SetFixedSize);
    setWindowFlag(Qt::WindowMaximizeButtonHint, false);
    showCalculator();
    restorePosition();
    m_area->setFocus();
}

Separators MainWindow::systemSeparators()
{
    const QLocale locale = QLocale::system();
    Separators separators;
    const QString decimal = locale.decimalPoint();
    const QString thousands = locale.groupSeparator();
    if (decimal.size() == 1) {
        separators.decimal = decimal.at(0).unicode();
    }
    if (thousands.size() == 1) {
        separators.thousands = thousands.at(0).unicode();
    }
    return separators;
}

void MainWindow::buildMenus()
{
    for (const Menu& menu : standardMenus()) {
        QMenu* qtMenu = menuBar()->addMenu(fromView(menu.title));
        std::map<int, QActionGroup*> dotGroups;
        for (const MenuItem& item : menu.items) {
            if (item.entry.separatorBefore) {
                qtMenu->addSeparator();
            }
            if (item.submenu.empty()) {
                addMenuEntry(qtMenu, item.entry, dotGroups);
                continue;
            }
            QMenu* submenu = qtMenu->addMenu(fromView(item.entry.text));
            for (const MenuEntry& entry : item.submenu) {
                if (entry.separatorBefore) {
                    submenu->addSeparator();
                }
                addMenuEntry(submenu, entry, dotGroups);
            }
        }
    }
}

void MainWindow::addMenuEntry(QMenu* menu, const MenuEntry& entry, std::map<int, QActionGroup*>& dotGroups)
{
    QAction* action = menu->addAction(fromView(entry.text));
    const bool built = !entry.commandId.empty();
    action->setEnabled(built);
    if (!entry.shortcut.empty()) {
        if (built) {
            action->setShortcut(QKeySequence(fromView(entry.shortcut)));
        } else {
            // A greyed entry shows its key but does not take it
            action->setText(action->text() + QLatin1Char('\t') + fromView(entry.shortcut));
        }
    }
    if (entry.mark != MenuMark::None) {
        action->setCheckable(true);
        action->setChecked(entry.chosen);
    }
    if (entry.mark == MenuMark::Dot && entry.dotGroup != 0) {
        QActionGroup*& group = dotGroups[entry.dotGroup];
        if (group == nullptr) {
            group = new QActionGroup(menu);
        }
        group->addAction(action);
    }
    if (built) {
        const std::string_view commandId = entry.commandId;
        connect(action, &QAction::triggered, this, [this, commandId] { runCommand(commandId); });
        if (commandId.starts_with("view.") && commandId != "view.basic") {
            action->setData(fromView(commandId));
            m_viewActions.push_back(action);
        }
    }
}

void MainWindow::buildView(View view)
{
    // The parts of the previous view go; they may be in the middle of their own click, so they go once it is over
    for (QWidget* part : m_viewParts) {
        part->hide();
        part->deleteLater();
    }
    m_viewParts.clear();
    m_buttons.clear();
    m_choiceButtons.clear();
    m_bracketCounter = nullptr;
    m_bitDisplay = nullptr;

    const ViewLayout& layout = layoutOf(view);
    m_builtView = view;
    m_area->setFixedSize(layout.clientWidth, layout.clientHeight);
    m_display->setViewLayout(layout);
    m_display->setGeometry(toRect(layout.displayFrame));

    for (const Rectangle& place : layout.frames) {
        auto* frame = new FlatFrame(m_area);
        frame->setGeometry(toRect(place));
        frame->show();
        m_viewParts.push_back(frame);
    }
    for (const ChoiceSpec& spec : layout.choices) {
        auto* choice = new ChoiceButton(spec.commandId, fromView(spec.label), m_area);
        choice->setObjectName(QStringLiteral("choice.") + fromView(spec.commandId));
        choice->setGeometry(toRect(spec.rectangle));
        const std::string_view commandId = spec.commandId;
        connect(choice, &ChoiceButton::clicked, this, [this, commandId] { runCommand(commandId); });
        choice->show();
        m_viewParts.push_back(choice);
        m_choiceButtons.push_back(choice);
    }
    if (layout.bitDisplay.frame.width > 0) {
        m_bitDisplay = new BitDisplay(layout.bitDisplay, m_area);
        m_bitDisplay->setObjectName(QStringLiteral("bits"));
        m_bitDisplay->setGeometry(toRect(layout.bitDisplay.frame));
        connect(m_bitDisplay, &BitDisplay::bitClicked, this, [this](int bit) { runBitCommand(bit); });
        m_bitDisplay->show();
        m_viewParts.push_back(m_bitDisplay);
    }
    if (layout.bracketCounter.width > 0) {
        m_bracketCounter = new BracketCounter(m_area);
        m_bracketCounter->setObjectName(QStringLiteral("brackets"));
        m_bracketCounter->setGeometry(toRect(layout.bracketCounter));
        m_bracketCounter->show();
        m_viewParts.push_back(m_bracketCounter);
    }
    for (ButtonSpec spec : layout.buttons) {
        // The decimal-point button shows the regional decimal separator, as Windows 7 does
        const std::string decimalLabel = QString(QChar(m_separators.decimal)).toStdString();
        if (spec.commandId == decimalCommand) {
            spec.label = decimalLabel;
        }
        auto* button = new CalcButton(spec, m_area);
        button->setObjectName(QStringLiteral("button.") + fromView(spec.commandId));
        button->setGeometry(toRect(spec.rectangle));
        const std::string_view commandId = spec.commandId;
        connect(button, &CalcButton::clicked, this, [this, commandId] { runCommand(commandId); });
        button->show();
        m_viewParts.push_back(button);
        m_buttons.push_back(button);
    }
}

void MainWindow::runCommand(std::string_view commandId)
{
    m_calculator.run(commandId);
    showCalculator();
}

void MainWindow::runBitCommand(int bit)
{
    // The command ids "bit.0" to "bit.63" are in the registry (static text), so the view stays valid
    for (const std::string_view id : commandIds()) {
        if (id == "bit." + std::to_string(bit)) {
            runCommand(id);
            return;
        }
    }
}

void MainWindow::runKeyCommand(std::string_view commandId)
{
    for (CalcButton* button : m_buttons) {
        if (button->commandId() == commandId && button->isEnabled()) {
            button->flash();
        }
    }
    runCommand(commandId);
}

void MainWindow::showCalculator()
{
    const View view = m_calculator.view();
    if (view != m_builtView) {
        buildView(view);
        QSettings().setValue(viewKey, nameOf(view));
    }
    for (QAction* action : m_viewActions) {
        action->setChecked(action->data().toString() == QStringLiteral("view.") + nameOf(view));
    }
    m_display->showValues(QString::fromStdWString(m_calculator.display()), QString::fromStdWString(m_calculator.expression()),
                          m_calculator.hasMemory());
    const bool inverse = m_calculator.isInverse();
    for (CalcButton* button : m_buttons) {
        // Greyed where the view or the base does not allow it (the decimal point in Programmer, 2 to 9 in Bin)
        button->setEnabled(m_calculator.isAvailable(button->commandId()));
        button->showInverse(inverse);
        if (button->commandId() == "inverse") {
            button->showOn(inverse);
        } else if (button->commandId() == "scientific_notation") {
            button->showOn(m_calculator.isScientificNotation());
        }
    }
    for (ChoiceButton* choice : m_choiceButtons) {
        choice->setChecked(m_calculator.isChosen(choice->commandId()));
    }
    if (m_bitDisplay != nullptr) {
        m_bitDisplay->showBits(m_calculator.bits(), m_calculator.wordBits());
    }
    if (m_bracketCounter != nullptr) {
        m_bracketCounter->showCount(m_calculator.openBrackets());
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    // The number pad and AltGr (the third character of a key) change no command
    const Qt::KeyboardModifiers modifiers = event->modifiers() & ~(Qt::KeypadModifier | Qt::GroupSwitchModifier);
    // On a layout that is not Latin the letter of the key's place comes first, as in Windows 7: a sign on that place
    // counts only where the view has no command for the letter (the Greek Q key is x² in Scientific, nothing in Standard)
    const bool readable = modifiers == Qt::NoModifier || modifiers == Qt::ShiftModifier || modifiers == Qt::ControlModifier;
    const std::optional<int> placeKey = readable ? placeKeyOf(*event) : std::nullopt;
    std::optional<std::string_view> command;
    if (modifiers == Qt::ControlModifier) {
        const QString letter = QKeySequence(placeKey.value_or(event->key())).toString();
        if (letter.size() == 1) {
            command = commandForControlLetter(letter.at(0).unicode(), m_calculator.view());
        }
    } else if (modifiers == Qt::NoModifier || modifiers == Qt::ShiftModifier) {
        const char32_t first = modifiers == Qt::ShiftModifier ? U'A' : U'a';
        if (placeKey) {
            command = commandForCharacter(static_cast<char32_t>(first + (*placeKey - Qt::Key_A)), m_calculator.view());
        }
        if (command) {
            // the place's letter has a command here
        } else if (const auto key = keyOf(event->key())) {
            command = commandForKey(*key, m_calculator.view());
        } else if (event->key() >= Qt::Key_A && event->key() <= Qt::Key_Z) {
            // A letter is read from its key, not its character, as Windows 7 does: without Shift it is the small
            // letter, with Shift the capital, whatever Caps Lock is (S is sin with Caps Lock on, Shift+S is nothing)
            command = commandForCharacter(static_cast<char32_t>(first + (event->key() - Qt::Key_A)),
                                          m_calculator.view());
        } else if (event->text().size() == 1) {
            command = commandForCharacter(event->text().at(0).unicode(), m_calculator.view());
        }
    }
    if (!command) {
        QMainWindow::keyPressEvent(event);
        return;
    }
    event->accept();
    runKeyCommand(*command);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    savePosition();
    QMainWindow::closeEvent(event);
}

// The window opens where it was last closed, as in Windows 7, unless its top (where it is taken to move it) would not
// lie wholly on one screen any more, for example after a second screen was removed; then the desktop places it
void MainWindow::restorePosition()
{
    const QSettings settings;
    const QVariant saved = settings.value(positionKey);
    if (!saved.canConvert<QPoint>()) {
        return;
    }
    const QPoint position = saved.toPoint();
    const QRect top(position, QSize(sizeHint().width(), menuBar()->sizeHint().height()));
    for (const QScreen* screen : QGuiApplication::screens()) {
        if (screen->availableGeometry().contains(top)) {
            move(position);
            return;
        }
    }
}

void MainWindow::savePosition() const
{
    QSettings settings;
    settings.setValue(positionKey, pos());
}

}  // namespace chrono_calc
