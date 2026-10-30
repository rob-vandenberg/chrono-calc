#include "ui/mainwindow.h"

#include <QAction>
#include <QActionGroup>
#include <QCloseEvent>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QKeySequence>
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
#include "core/colours.h"
#include "core/enginetexts.h"
#include "core/keys.h"
#include "core/menus.h"
#include "core/layout.h"
#include "core/view.h"
#include "ui/calcbutton.h"
#include "ui/displaypanel.h"
#include "ui/look.h"
#include "ui/scientificparts.h"

#include <algorithm>
#include <map>
#include <optional>
#include <string>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/ui/mainwindow.cpp 0.2.7

// --- Version history ----------------------------------------------------
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
const QString standardName = QStringLiteral("standard");

// The height the window's gradient is measured on (Windows 7: the same in every view)
constexpr double gradientHeight = 331.0;

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
        gradient.setColorAt(79.0 / gradientHeight, qtColour(colours::windowUpper));
        gradient.setColorAt(165.0 / gradientHeight, qtColour(colours::windowBottom));
        gradient.setColorAt(1.0, qtColour(colours::windowBottom));
        painter.fillRect(rect(), gradient);
    }
};

QRect toRect(Rectangle rectangle)
{
    return {rectangle.left, rectangle.top, rectangle.width, rectangle.height};
}

std::optional<Key> keyOf(int qtKey)
{
    switch (qtKey) {
    case Qt::Key_Escape: return Key::Escape;
    case Qt::Key_Delete: return Key::Delete;
    case Qt::Key_Backspace: return Key::Backspace;
    case Qt::Key_Return:
    case Qt::Key_Enter: return Key::Enter;
    case Qt::Key_F3: return Key::F3;
    case Qt::Key_F4: return Key::F4;
    case Qt::Key_F5: return Key::F5;
    case Qt::Key_F9: return Key::F9;
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
    if (QSettings().value(viewKey).toString() == scientificName) {
        m_calculator.run("view.scientific");
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
        if (commandId == "view.standard" || commandId == "view.scientific") {
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
    m_angleButtons.clear();
    m_bracketCounter = nullptr;

    const ViewLayout& layout = layoutOf(view);
    m_builtView = view;
    m_area->setFixedSize(layout.clientWidth, layout.clientHeight);
    m_display->setViewLayout(layout);
    m_display->setGeometry(toRect(layout.displayFrame));

    if (layout.angleFrame.width > 0) {
        auto* frame = new AngleFrame(m_area);
        frame->setGeometry(toRect(layout.angleFrame));
        frame->show();
        m_viewParts.push_back(frame);
    }
    for (const AngleSpec& spec : layout.angles) {
        auto* angle = new AngleButton(spec.commandId, fromView(spec.label), m_area);
        angle->setObjectName(QStringLiteral("angle.") + fromView(spec.label).toLower());
        angle->setGeometry(toRect(spec.rectangle));
        const std::string_view commandId = spec.commandId;
        connect(angle, &AngleButton::clicked, this, [this, commandId] { runCommand(commandId); });
        angle->show();
        m_viewParts.push_back(angle);
        m_angleButtons.push_back(angle);
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
        button->setEnabled(Calculator::isAvailable(view, spec.commandId));
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
        QSettings().setValue(viewKey, view == View::Scientific ? scientificName : standardName);
    }
    for (QAction* action : m_viewActions) {
        action->setChecked(action->data().toString() == (view == View::Scientific ? QStringLiteral("view.scientific") : QStringLiteral("view.standard")));
    }
    m_display->showValues(QString::fromStdWString(m_calculator.display()), QString::fromStdWString(m_calculator.expression()),
                          m_calculator.hasMemory());
    const bool inverse = m_calculator.isInverse();
    for (CalcButton* button : m_buttons) {
        button->showInverse(inverse);
        if (button->commandId() == "inverse") {
            button->showOn(inverse);
        } else if (button->commandId() == "scientific_notation") {
            button->showOn(m_calculator.isScientificNotation());
        }
    }
    const AngleUnit unit = m_calculator.angleUnit();
    for (AngleButton* angle : m_angleButtons) {
        const auto target = std::ranges::find(layoutOf(view).angles, angle->commandId(), &AngleSpec::commandId);
        angle->setChecked(target != layoutOf(view).angles.end() && target->unit == unit);
    }
    if (m_bracketCounter != nullptr) {
        m_bracketCounter->showCount(m_calculator.openBrackets());
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    // The number pad and AltGr (the third character of a key) change no command
    const Qt::KeyboardModifiers modifiers = event->modifiers() & ~(Qt::KeypadModifier | Qt::GroupSwitchModifier);
    std::optional<std::string_view> command;
    if (modifiers == Qt::ControlModifier) {
        const QString letter = QKeySequence(event->key()).toString();
        if (letter.size() == 1) {
            command = commandForControlLetter(letter.at(0).unicode(), m_calculator.view());
        }
    } else if (modifiers == Qt::NoModifier || modifiers == Qt::ShiftModifier) {
        if (const auto key = keyOf(event->key())) {
            command = commandForKey(*key, m_calculator.view());
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
