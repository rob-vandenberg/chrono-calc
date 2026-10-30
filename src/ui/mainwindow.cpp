#include "ui/mainwindow.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QChar>
#include <QClipboard>
#include <QCloseEvent>
#include <QDate>
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
#include <QMimeData>
#include <QPaintEvent>
#include <QPainter>
#include <QPoint>
#include <QRect>
#include <QScreen>
#include <QSettings>
#include <QSize>
#include <QString>
#include <QStringList>
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
#include "core/worksheets.h"
#include "ui/calcbutton.h"
#include "ui/displaypanel.h"
#include "ui/keyboardlayouts.h"
#include "ui/look.h"
#include "ui/sidepanels.h"
#include "ui/statisticsparts.h"
#include "ui/viewparts.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// --- Version ------------------------------------------------------------
// src/ui/mainwindow.cpp 0.8.13

// --- Version history ----------------------------------------------------
// v0.8.13: The side panel beside every view (View > Basic, Unit conversion, Date calculation, Worksheets), remembered;
//          the Unit conversion panel (D-025), the Date calculation panel (D-026) and the Worksheets (D-027) with
//          Windows 7's keys and Tab order
// v0.5.12: History (the list in a taller display, the edit box), Digit grouping, Copy and Paste with the clipboard,
//          the Edit menu of each view, History and Digit grouping remembered (D-024)
// v0.4.11: The Statistics view: its size, the list of values with its selection buttons, the count (D-023)
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
// Windows 7 keeps History shown (ShowHistory) and Digit grouping (UseSep) as well (windows7-history.md part H)
const QString historyKey = QStringLiteral("window/history");
const QString groupingKey = QStringLiteral("window/digitGrouping");
// Windows 7 keeps the side panel shown (UnitConv, DateTime, Templates), not the panel's choices; the name kept is that
// of its command without "view." ("unit_conversion" for "view.unit_conversion")
const QString sidePanelKey = QStringLiteral("window/sidePanel");
const QString viewPrefix = QStringLiteral("view.");

// The name a side panel is kept under: its command's id without "view."; empty for Basic
QString sidePanelName(SidePanel panel)
{
    for (const std::string_view commandId : commandIds()) {
        const std::optional<CommandTarget> target = findCommand(commandId);
        if (panel != SidePanel::Basic && target && target->kind == CommandKind::ViewSidePanel && target->panel == panel) {
            return QString::fromUtf8(commandId.data(), static_cast<qsizetype>(commandId.size())).mid(viewPrefix.size());
        }
    }
    return {};
}

const QString scientificName = QStringLiteral("scientific");
const QString programmerName = QStringLiteral("programmer");
const QString standardName = QStringLiteral("standard");
const QString statisticsName = QStringLiteral("statistics");

QString nameOf(View view)
{
    switch (view) {
    case View::Scientific: return scientificName;
    case View::Programmer: return programmerName;
    case View::Statistics: return statisticsName;
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

// The character a key types for a command of the edit box: a digit, an operator, a bracket, the decimal point, = (in
// the history's edit box = calculates the edit); nothing for any other command
QString typedCharacterOf(std::string_view commandId, wchar_t decimal)
{
    if (commandId.starts_with("digit.") && commandId.size() == std::string_view("digit.0").size()) {
        return QString(QLatin1Char(commandId.back())).toUpper();
    }
    static const std::map<std::string_view, QString> characters = {
        {"add", QStringLiteral("+")}, {"subtract", QStringLiteral("-")}, {"multiply", QStringLiteral("*")},
        {"divide", QStringLiteral("/")}, {"open_bracket", QStringLiteral("(")}, {"close_bracket", QStringLiteral(")")},
        {"power", QStringLiteral("^")}};
    if (commandId == decimalCommand) {
        return {QChar(decimal)};
    }
    const auto found = characters.find(commandId);
    return found == characters.end() ? QString() : found->second;
}

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
    case Qt::Key_Up: return Key::Up;
    case Qt::Key_Down: return Key::Down;
    case Qt::Key_PageUp: return Key::PageUp;
    case Qt::Key_PageDown: return Key::PageDown;
    default: return std::nullopt;
    }
}

}  // namespace

MainWindow::MainWindow(QWidget* parent)
    : MainWindow(systemSeparators(), parent)
{
}

MainWindow::MainWindow(const Separators& separators, QWidget* parent)
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
    // Digit grouping, History and the view as the calculator was closed with, as Windows 7 remembers them; History
    // first, while the calculator is in Standard, where it can be shown
    const QSettings settings;
    if (settings.value(groupingKey).toBool()) {
        m_calculator.run("view.digit_grouping");
    }
    if (settings.value(historyKey).toBool()) {
        m_calculator.run("view.history");
    }
    const QString savedView = settings.value(viewKey).toString();
    if (savedView == scientificName) {
        m_calculator.run("view.scientific");
    } else if (savedView == programmerName) {
        m_calculator.run("view.programmer");
    } else if (savedView == statisticsName) {
        m_calculator.run("view.statistics");
    }
    const QString savedPanel = settings.value(sidePanelKey).toString();
    if (const std::optional<CommandTarget> panel = findCommand((viewPrefix + savedPanel).toStdString());
        !savedPanel.isEmpty() && panel && panel->kind == CommandKind::ViewSidePanel) {
        m_calculator.run((viewPrefix + savedPanel).toStdString());
    }
    buildView(m_calculator.view());
    buildEditMenu();
    connect(QGuiApplication::clipboard(), &QClipboard::dataChanged, this, [this] { updatePaste(); });
    updatePaste();
    // The window has the size of its view, as in Windows 7: it cannot be resized or maximised
    layout()->setSizeConstraint(QLayout::SetFixedSize);
    setWindowFlag(Qt::WindowMaximizeButtonHint, false);
    showCalculator();
    restorePosition();
    m_area->setFocus();
}

Separators MainWindow::systemSeparators()
{
    // The default locale is the system's (main sets no other)
    const QLocale locale;
    Separators separators;
    const QString decimal = locale.decimalPoint();
    const QString thousands = locale.groupSeparator();
    if (decimal.size() == 1) {
        separators.decimal = decimal.at(0).unicode();
    }
    if (thousands.size() == 1) {
        separators.thousands = thousands.at(0).unicode();
    }
    // The grouping pattern, in the engine's form: the sizes of the last two groups the locale writes a long number
    // with ("3;0" every three, India "3;2;0" three and then every two)
    const QStringList groups = locale.toString(Q_INT64_C(1234567890123)).split(thousands);
    if (!thousands.isEmpty() && groups.size() >= 3) {
        const qsizetype first = groups.at(groups.size() - 1).size();
        const qsizetype second = groups.at(groups.size() - 2).size();
        separators.grouping = first == second ? QStringLiteral("%1;0").arg(first).toStdWString()
                                              : QStringLiteral("%1;%2;0").arg(first).arg(second).toStdWString();
    }
    return separators;
}

void MainWindow::buildMenus()
{
    for (const Menu& menu : standardMenus()) {
        QMenu* qtMenu = menuBar()->addMenu(fromView(menu.title));
        if (menu.title == editMenu(View::Standard).title) {
            m_editMenu = qtMenu;
            // Paste is greyed while the clipboard holds no text (Windows 7)
            connect(qtMenu, &QMenu::aboutToShow, this, [this] { updatePaste(); });
            continue;
        }
        addMenuItems(qtMenu, menu.items);
    }
}

void MainWindow::buildEditMenu()
{
    m_editMenu->clear();
    m_pasteAction = nullptr;
    m_editAction = nullptr;
    addMenuItems(m_editMenu, editMenu(m_calculator.view()).items);
    updatePaste();
}

void MainWindow::addMenuItems(QMenu* menu, const std::vector<MenuItem>& items)
{
    std::map<int, QActionGroup*> dotGroups;
    for (const MenuItem& item : items) {
        if (item.entry.separatorBefore) {
            menu->addSeparator();
        }
        if (item.submenu.empty()) {
            addMenuEntry(menu, item.entry, dotGroups);
            continue;
        }
        QMenu* submenu = menu->addMenu(fromView(item.entry.text));
        bool built = false;
        for (const MenuEntry& entry : item.submenu) {
            if (entry.separatorBefore) {
                submenu->addSeparator();
            }
            addMenuEntry(submenu, entry, dotGroups);
            built = built || !entry.commandId.empty();
        }
        // A submenu with nothing usable is greyed (Programmer's History)
        submenu->setEnabled(built);
    }
}

void MainWindow::addMenuEntry(QMenu* menu, const MenuEntry& entry, std::map<int, QActionGroup*>& dotGroups)
{
    QAction* action = menu->addAction(fromView(entry.text));
    const bool built = !entry.commandId.empty();
    action->setEnabled(built);
    if (!entry.shortcut.empty()) {
        if (built && !entry.shortcutShownOnly) {
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
        connectMenuAction(action, commandId);
        // The four views (one dot among them); Basic, History and Digit grouping are not views
        if (commandId == "view.standard" || commandId == "view.scientific" || commandId == "view.programmer"
            || commandId == "view.statistics") {
            action->setData(fromView(commandId));
            m_viewActions.push_back(action);
        }
        if (const std::optional<CommandTarget> target = findCommand(commandId); target && target->kind == CommandKind::ViewSidePanel) {
            action->setData(fromView(commandId));
            m_panelActions.push_back(action);
        }
    }
}

void MainWindow::connectMenuAction(QAction* action, std::string_view commandId)
{
    if (commandId == "view.history") {
        m_historyAction = action;
    } else if (commandId == "view.digit_grouping") {
        m_groupingAction = action;
    } else if (commandId == "edit.paste") {
        m_pasteAction = action;
    } else if (commandId == "history.edit" || commandId == "statistics.edit") {
        m_editAction = action;
    }
    connect(action, &QAction::triggered, this, [this, commandId] {
        if (commandId == "edit.paste") {
            paste();
        } else if (commandId == "history.edit" && m_editBox != nullptr) {
            takeEdit(m_editBox->text());  // Recalculate (Windows 7)
        } else {
            runCommand(commandId);
        }
    });
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
    m_statisticsList = nullptr;
    if (m_editBox != nullptr) {
        m_editBox->hide();
        m_editBox->deleteLater();
        m_editBox = nullptr;
    }
    m_editList = nullptr;
    m_historyList = nullptr;
    // The keyboard focus stays with the calculator when the part that had it goes
    for (const QWidget* panel :
         {static_cast<QWidget*>(m_unitPanel), static_cast<QWidget*>(m_datePanel), static_cast<QWidget*>(m_worksheetPanel)}) {
        if (panel != nullptr && panel->isAncestorOf(QApplication::focusWidget())) {
            m_area->setFocus();
        }
    }
    m_unitPanel = nullptr;
    m_datePanel = nullptr;
    m_worksheetPanel = nullptr;

    m_builtHistory = m_calculator.isHistoryShown();
    const ViewLayout& layout = layoutOf(view, m_builtHistory);
    m_builtView = view;
    m_builtPanel = m_calculator.sidePanel();
    const bool panelShown = m_builtPanel != SidePanel::Basic;
    m_area->setFixedSize(panelShown ? sidePanelLayoutOf(view, m_builtHistory).clientWidth : layout.clientWidth,
                         layout.clientHeight);
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
    buildLists(layout);
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
        connect(button, &CalcButton::clicked, this, [this, commandId] { runButtonCommand(commandId); });
        button->show();
        m_viewParts.push_back(button);
        m_buttons.push_back(button);
    }
    buildSidePanel(view);
}

void MainWindow::buildSidePanel(View view)
{
    if (m_builtPanel == SidePanel::UnitConversion) {
        buildUnitConversionPanel(view);
    } else if (m_builtPanel == SidePanel::DateCalculation) {
        buildDateCalculationPanel(view);
    } else if (const std::optional<Worksheet> worksheet = worksheetOf(m_builtPanel)) {
        buildWorksheetPanel(view, *worksheet);
    }
}

void MainWindow::buildUnitConversionPanel(View view)
{
    m_unitPanel = new UnitConversionPanel(m_area);
    m_unitPanel->setObjectName(QStringLiteral("unit.panel"));
    m_unitPanel->setGeometry(toRect(sidePanelLayoutOf(view, m_builtHistory).frame));
    connect(m_unitPanel, &UnitConversionPanel::typeChosen, this, [this](int index) {
        runCommand("unit.type." + std::to_string(index));
    });
    connect(m_unitPanel, &UnitConversionPanel::fromChosen, this, [this](int index) {
        runCommand("unit.from." + std::to_string(index));
    });
    connect(m_unitPanel, &UnitConversionPanel::toChosen, this, [this](int index) {
        runCommand("unit.to." + std::to_string(index));
    });
    connect(m_unitPanel, &UnitConversionPanel::inputEdited, this, [this](const QString& text) {
        m_calculator.run("unit.input", text.toStdWString());
        showCalculator();
    });
    connect(m_unitPanel, &UnitConversionPanel::focusToCalculator, this, [this] { m_area->setFocus(); });
    // Windows 7's Tab order: From, its unit, To, its unit, the calculator, the type list, From again
    setTabOrder(m_unitPanel->fromBox(), m_unitPanel->fromList());
    setTabOrder(m_unitPanel->fromList(), m_unitPanel->toBox());
    setTabOrder(m_unitPanel->toBox(), m_unitPanel->toList());
    setTabOrder(m_unitPanel->toList(), m_area);
    setTabOrder(m_area, m_unitPanel->typeList());
    setTabOrder(m_unitPanel->typeList(), m_unitPanel->fromBox());
    m_unitPanel->show();
    m_viewParts.push_back(m_unitPanel);
}

void MainWindow::buildDateCalculationPanel(View view)
{
    m_datePanel = new DateCalculationPanel(m_area);
    m_datePanel->setObjectName(QStringLiteral("date.panel"));
    m_datePanel->setGeometry(toRect(sidePanelLayoutOf(view, m_builtHistory).frame));
    const auto runDate = [this](const std::string& commandId, const QString& text) {
        m_calculator.run(commandId, text.toStdWString());
        showCalculator();
    };
    connect(m_datePanel, &DateCalculationPanel::modeChosen, this, [this](int mode) { runCommand("date.mode." + std::to_string(mode)); });
    connect(m_datePanel, &DateCalculationPanel::differenceFromChosen, this,
            [runDate](QDate date) { runDate("date.difference_from", date.toString(Qt::ISODate)); });
    connect(m_datePanel, &DateCalculationPanel::differenceToChosen, this,
            [runDate](QDate date) { runDate("date.difference_to", date.toString(Qt::ISODate)); });
    connect(m_datePanel, &DateCalculationPanel::shiftFromChosen, this,
            [runDate](QDate date) { runDate("date.shift_from", date.toString(Qt::ISODate)); });
    connect(m_datePanel, &DateCalculationPanel::subtractChosen, this,
            [this](bool subtract) { runCommand(subtract ? "date.subtract" : "date.add"); });
    static constexpr std::array<std::string_view, 3> fieldNames{"date.years", "date.months", "date.days"};
    connect(m_datePanel, &DateCalculationPanel::fieldEdited, this, [runDate](int field, const QString& text) {
        runDate(std::string(fieldNames.at(static_cast<std::size_t>(field))), text);
    });
    connect(m_datePanel, &DateCalculationPanel::fieldStepped, this, [this](int field, bool up) {
        runCommand(std::string(fieldNames.at(static_cast<std::size_t>(field))) + (up ? ".up" : ".down"));
    });
    connect(m_datePanel, &DateCalculationPanel::calculateChosen, this, [this] { runCommand("date.calculate"); });
    connect(m_datePanel, &DateCalculationPanel::focusToCalculator, this, [this] { m_area->setFocus(); });
    // Windows 7's Tab order: the parts of the mode, Calculate, the calculator, the mode list, the first part again
    const std::vector<QWidget*> parts = m_datePanel->tabOrder();
    for (std::size_t index = 0; index + 1 < parts.size(); ++index) {
        setTabOrder(parts.at(index), parts.at(index + 1));
    }
    setTabOrder(parts.back(), m_area);
    setTabOrder(m_area, m_datePanel->modeList());
    setTabOrder(m_datePanel->modeList(), parts.front());
    m_datePanel->show();
    m_viewParts.push_back(m_datePanel);
}

void MainWindow::buildWorksheetPanel(View view, Worksheet worksheet)
{
    m_worksheetPanel = new WorksheetPanel(worksheet, m_area);
    m_worksheetPanel->setObjectName(QStringLiteral("worksheet.panel"));
    m_worksheetPanel->setGeometry(toRect(sidePanelLayoutOf(view, m_builtHistory).frame));
    connect(m_worksheetPanel, &WorksheetPanel::choiceChosen, this, [this](int item) { runCommand("worksheet.choose." + std::to_string(item)); });
    connect(m_worksheetPanel, &WorksheetPanel::focusToCalculator, this, [this] { m_area->setFocus(); });
    connect(m_worksheetPanel, &WorksheetPanel::valueEdited, this, [this](int value, const QString& text) {
        m_calculator.run("worksheet.value." + std::to_string(value), text.toStdWString());
        showCalculator();
    });
    // After Calculate the keyboard focus goes to the result, or to the box a message points at (windows7-worksheets.md
    // part B line 2, part D)
    connect(m_worksheetPanel, &WorksheetPanel::calculateChosen, this, [this] {
        runCommand("worksheet.calculate");
        if (m_worksheetPanel != nullptr) {
            m_worksheetPanel->showCalculated(m_calculator.worksheetCalculator());
        }
    });
    // Windows 7's Tab order: the boxes, Calculate, the result, the calculator, the list, the first box again
    const std::vector<QWidget*> parts = m_worksheetPanel->tabOrder();
    for (std::size_t index = 0; index + 1 < parts.size(); ++index) {
        setTabOrder(parts.at(index), parts.at(index + 1));
    }
    setTabOrder(parts.back(), m_area);
    setTabOrder(m_area, m_worksheetPanel->choiceList());
    setTabOrder(m_worksheetPanel->choiceList(), parts.front());
    m_worksheetPanel->show();
    m_viewParts.push_back(m_worksheetPanel);
}

// The Statistics view's list and the history list, each with its two selection buttons, on the display (Windows 7)
void MainWindow::buildLists(const ViewLayout& layout)
{
    if (layout.statistics.list.width > 0) {
        const StatisticsSpec& statistics = layout.statistics;
        m_statisticsList = new StatisticsList(statistics, false, m_area);
        m_statisticsList->setObjectName(QStringLiteral("statistics.list"));
        m_statisticsList->setGeometry(toRect(statistics.list));
        connect(m_statisticsList, &StatisticsList::indexChosen, this, [this](int index) { selectValue(index); });
        connect(m_statisticsList, &StatisticsList::previousChosen, this, [this] { runCommand("statistics.select_previous"); });
        connect(m_statisticsList, &StatisticsList::nextChosen, this, [this] { runCommand("statistics.select_next"); });
        connect(m_statisticsList, &StatisticsList::deleteChosen, this, [this] { runCommand("statistics.delete"); });
        m_statisticsList->show();
        m_viewParts.push_back(m_statisticsList);
        for (const bool up : {true, false}) {
            auto* arrow = new SelectionArrow(up, m_area);
            arrow->setObjectName(up ? QStringLiteral("statistics.up") : QStringLiteral("statistics.down"));
            arrow->setGeometry(toRect(up ? statistics.upButton : statistics.downButton));
            // The selection moves, and the list takes the keyboard focus (Windows 7)
            connect(arrow, &SelectionArrow::clicked, this, [this, up] {
                m_statisticsList->setFocus();
                runCommand(up ? "statistics.select_previous" : "statistics.select_next");
            });
            arrow->show();
            m_viewParts.push_back(arrow);
        }
    }
    buildHistoryList(layout);
}

void MainWindow::buildHistoryList(const ViewLayout& layout)
{
    if (layout.history.list.width > 0) {
        const StatisticsSpec& history = layout.history;
        m_historyList = new StatisticsList(history, true, m_area);
        m_historyList->setObjectName(QStringLiteral("history.list"));
        m_historyList->setGeometry(toRect(history.list));
        connect(m_historyList, &StatisticsList::indexChosen, this, [this](int index) { chooseEntry(index); });
        connect(m_historyList, &StatisticsList::previousChosen, this, [this] { runListCommand("history.select_previous"); });
        connect(m_historyList, &StatisticsList::nextChosen, this, [this] { runListCommand("history.select_next"); });
        connect(m_historyList, &StatisticsList::editChosen, this, [this] { runCommand("history.edit"); });
        m_historyList->show();
        m_viewParts.push_back(m_historyList);
        for (const bool up : {true, false}) {
            auto* arrow = new SelectionArrow(up, m_area);
            arrow->setObjectName(up ? QStringLiteral("history.up") : QStringLiteral("history.down"));
            arrow->setGeometry(toRect(up ? history.upButton : history.downButton));
            // The selection moves and the entry's result shows; the keyboard focus stays where it is (Windows 7)
            connect(arrow, &SelectionArrow::clicked, this,
                    [this, up] { runCommand(up ? "history.select_previous" : "history.select_next"); });
            arrow->show();
            m_viewParts.push_back(arrow);
        }
    }
}

void MainWindow::runCommand(std::string_view commandId)
{
    // A view change applies an edit of the history first (Windows 7, windows7-history.md part D line 2)
    if (m_editBox != nullptr && m_calculator.editedHistoryEntry()
        && (commandId == "view.standard" || commandId == "view.scientific" || commandId == "view.programmer"
            || commandId == "view.statistics")) {
        m_calculator.run("history.recalculate", m_editBox->text().toStdWString());
    }
    m_calculator.run(commandId);
    if (const std::optional<std::wstring> copied = m_calculator.takeCopiedText()) {
        QGuiApplication::clipboard()->setText(QString::fromStdWString(*copied));
    }
    if (commandId == "view.history") {
        QSettings().setValue(historyKey, m_calculator.isHistoryShown());
    } else if (commandId == "view.digit_grouping") {
        QSettings().setValue(groupingKey, m_calculator.isDigitGrouping());
    }
    showCalculator();
}

void MainWindow::paste()
{
    const QString text = QGuiApplication::clipboard()->text();
    if (!text.isEmpty()) {
        m_calculator.run("edit.paste", text.toStdWString());
    }
    showCalculator();
}

void MainWindow::updatePaste()
{
    if (m_pasteAction != nullptr) {
        const QMimeData* data = QGuiApplication::clipboard()->mimeData();
        m_pasteAction->setEnabled(data != nullptr && data->hasText() && !data->text().isEmpty());
    }
}

void MainWindow::chooseEntry(int index)
{
    if (index >= 0) {
        m_calculator.run("history.select." + std::to_string(index));
    }
    showCalculator();
}

void MainWindow::runListCommand(std::string_view commandId)
{
    runCommand(commandId);
    m_area->setFocus();
}

void MainWindow::showEditBox()
{
    std::optional<std::size_t> edited = m_calculator.editedHistoryEntry();
    StatisticsList* list = m_historyList;
    const bool history = edited.has_value();
    if (!history) {
        edited = m_calculator.editedStatisticsValue();
        list = m_statisticsList;
    }
    if (!edited || list == nullptr) {
        closeEditBox();
        return;
    }
    const int index = static_cast<int>(*edited);
    if (m_editBox != nullptr || index >= list->values().size()) {
        return;
    }
    // Over the row, one pixel in from the list's left edge (Windows 7: 21, 82, 223, 18 for the third row); the row is
    // brought into view first, and the list does not scroll while the box is open
    list->scrollTo(index);
    const QRect row = list->rowArea(index);
    m_editBox = new ListEditBox(list->values().at(index), history, m_area);
    m_editBox->setObjectName(QStringLiteral("edit.box"));
    m_editBox->setGeometry(list->x() + 1, list->y() + row.top(), list->width() - 1, row.height());
    connect(m_editBox, &ListEditBox::taken, this, [this](const QString& text) { takeEdit(text); });
    connect(m_editBox, &ListEditBox::dropped, this, [this] { dropEdit(); });
    m_editList = list;
    list->setScrollLocked(true);
    m_editBox->show();
    m_editBox->setFocus();
}

// The edit box goes; the list it belongs to takes the keyboard focus back when the box had it (Windows 7)
void MainWindow::closeEditBox()
{
    if (m_editBox == nullptr) {
        return;
    }
    const bool focused = m_editBox->hasFocus();
    m_editBox->hide();
    m_editBox->deleteLater();
    m_editBox = nullptr;
    if (m_editList != nullptr) {
        m_editList->setScrollLocked(false);
        if (focused) {
            m_editList->setFocus();
        }
    }
    m_editList = nullptr;
}

void MainWindow::takeEdit(const QString& text)
{
    StatisticsList* list = m_editList;
    m_calculator.run(m_calculator.editedHistoryEntry() ? "history.recalculate" : "statistics.apply_edit", text.toStdWString());
    showCalculator();
    if (m_editBox == nullptr && list != nullptr) {
        list->setFocus();
    }
}

void MainWindow::dropEdit()
{
    StatisticsList* list = m_editList;
    m_calculator.run(m_calculator.editedHistoryEntry() ? "history.cancel_edit" : "statistics.cancel_edit");
    showCalculator();
    if (list != nullptr) {
        list->setFocus();
    }
}

void MainWindow::runButtonCommand(std::string_view commandId)
{
    // While an entry or a value is edited, a calculator button types its character into the edit box, as a click does
    // in Windows 7 (windows7-history.md part D line 3); a button without one does nothing then
    if (m_editBox != nullptr) {
        if (commandId == "equals" && m_calculator.editedHistoryEntry()) {
            takeEdit(m_editBox->text());
            return;
        }
        const QString typed = typedCharacterOf(commandId, m_separators.decimal);
        if (!typed.isEmpty()) {
            m_editBox->insert(typed);
        }
        m_editBox->setFocus();
        return;
    }
    if (m_statisticsList != nullptr && m_statisticsList->hasFocus()) {
        m_area->setFocus();
    }
    runCommand(commandId);
}

void MainWindow::selectValue(int index)
{
    if (index >= 0) {
        m_calculator.run("statistics.select." + std::to_string(index));
    }
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
    if (view != m_builtView || m_calculator.isHistoryShown() != m_builtHistory || m_calculator.sidePanel() != m_builtPanel) {
        const bool viewChanged = view != m_builtView;
        const bool panelChanged = m_calculator.sidePanel() != m_builtPanel;
        if (panelChanged) {
            QSettings().setValue(sidePanelKey, sidePanelName(m_calculator.sidePanel()));
        }
        buildView(view);
        // The Date calculation panel takes the keyboard focus on its mode list when it is chosen (Windows 7, part A
        // line 4)
        if (panelChanged && m_datePanel != nullptr) {
            m_datePanel->modeList()->setFocus();
        }
        // A Worksheet's list as well (windows7-worksheets.md part B line 2)
        if (panelChanged && m_worksheetPanel != nullptr) {
            m_worksheetPanel->choiceList()->setFocus();
        }
        if (viewChanged) {
            QSettings().setValue(viewKey, nameOf(view));
            buildEditMenu();
        }
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
    showLists();
    if (m_unitPanel != nullptr) {
        m_unitPanel->showState(m_calculator.unitConverter());
    }
    if (m_datePanel != nullptr) {
        m_datePanel->showState(m_calculator.dateCalculator());
    }
    if (m_worksheetPanel != nullptr) {
        m_worksheetPanel->showState(m_calculator.worksheetCalculator());
    }
    showEditBox();
    // After the edit box: it decides whether Edit reads Recalculate
    showMenuStates();
}

void MainWindow::showMenuStates()
{
    if (m_historyAction != nullptr) {
        // Greyed in Programmer and Statistics, where it keeps its tick (Windows 7)
        m_historyAction->setChecked(m_calculator.isHistoryShown());
        m_historyAction->setEnabled(m_calculator.isAvailable("view.history"));
    }
    if (m_groupingAction != nullptr) {
        m_groupingAction->setChecked(m_calculator.isDigitGrouping());
    }
    if (m_editAction != nullptr) {
        // While a history entry is edited, Edit reads Recalculate (=); the Statistics view's Edit is greyed then
        const bool editingHistory = m_calculator.editedHistoryEntry().has_value() && m_editBox != nullptr;
        m_editAction->setText(editingHistory ? tr("&Recalculate") + QStringLiteral("\t=") : tr("&Edit") + QStringLiteral("\tF2"));
        m_editAction->setEnabled(!m_calculator.editedStatisticsValue());
    }
    for (QAction* action : m_viewActions) {
        action->setChecked(action->data().toString() == QStringLiteral("view.") + nameOf(m_calculator.view()));
    }
    for (QAction* action : m_panelActions) {
        const std::optional<CommandTarget> target = findCommand(action->data().toString().toStdString());
        action->setChecked(target && target->panel == m_calculator.sidePanel());
    }
}

void MainWindow::showLists()
{
    if (m_statisticsList != nullptr) {
        QStringList values;
        for (const std::wstring& value : m_calculator.statisticsValues()) {
            values.append(QString::fromStdWString(value));
        }
        const std::optional<std::size_t> selected = m_calculator.selectedValue();
        m_statisticsList->showValues(values, selected ? static_cast<int>(*selected) : -1);
        m_display->showCount(static_cast<int>(m_calculator.statisticsCount()));
    }
    if (m_historyList != nullptr) {
        QStringList entries;
        for (const std::wstring& entry : m_calculator.historyEntries()) {
            entries.append(QString::fromStdWString(entry));
        }
        const std::optional<std::size_t> selected = m_calculator.selectedHistoryEntry();
        m_historyList->showValues(entries, selected ? static_cast<int>(*selected) : -1);
    }
}

std::optional<std::string_view> MainWindow::commandOfKey(const QKeyEvent& event, Qt::KeyboardModifiers modifiers) const
{
    // On a layout that is not Latin the letter of the key's place comes first, as in Windows 7: a sign on that place
    // counts only where the view has no command for the letter (the Greek Q key is x² in Scientific, nothing in Standard)
    const bool readable = modifiers == Qt::NoModifier || modifiers == Qt::ShiftModifier || modifiers == Qt::ControlModifier;
    const std::optional<int> placeKey = readable ? placeKeyOf(event) : std::nullopt;
    std::optional<std::string_view> command;
    if (modifiers == Qt::ControlModifier) {
        const QString letter = QKeySequence(placeKey.value_or(event.key())).toString();
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
        } else if (const auto key = keyOf(event.key())) {
            command = commandForKey(*key, m_calculator.view());
        } else if (event.key() >= Qt::Key_A && event.key() <= Qt::Key_Z) {
            // A letter is read from its key, not its character, as Windows 7 does: without Shift it is the small
            // letter, with Shift the capital, whatever Caps Lock is (S is sin with Caps Lock on, Shift+S is nothing)
            command = commandForCharacter(static_cast<char32_t>(first + (event.key() - Qt::Key_A)),
                                          m_calculator.view());
        } else if (event.text().size() == 1) {
            command = commandForCharacter(event.text().at(0).unicode(), m_calculator.view());
        }
    }
    return command;
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    // The number pad and AltGr (the third character of a key) change no command
    const Qt::KeyboardModifiers modifiers = event->modifiers() & ~(Qt::KeypadModifier | Qt::GroupSwitchModifier);
    // Ctrl+Insert copies, as in Windows 7; Shift+Insert pastes, as everywhere on the desktop
    if (event->key() == Qt::Key_Insert && (modifiers == Qt::ControlModifier || modifiers == Qt::ShiftModifier)) {
        event->accept();
        if (modifiers == Qt::ControlModifier) {
            runCommand("edit.copy");
        } else {
            paste();
        }
        return;
    }
    const std::optional<std::string_view> command = commandOfKey(*event, modifiers);
    if (!command) {
        QMainWindow::keyPressEvent(event);
        return;
    }
    event->accept();
    // Add or = (Enter) and C (Escape) take the keyboard focus back from the list (Windows 7)
    const bool listFocused = (m_statisticsList != nullptr && m_statisticsList->hasFocus())
                             || (m_historyList != nullptr && m_historyList->hasFocus());
    if (listFocused && (*command == "statistics.add" || *command == "equals" || *command == "clear")) {
        m_area->setFocus();
    }
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
