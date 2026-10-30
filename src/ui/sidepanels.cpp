#include "ui/sidepanels.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QEvent>
#include <QFont>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPalette>
#include <QPoint>
#include <QPointer>
#include <QPolygon>
#include <QPushButton>
#include <QRadioButton>
#include <QRect>
#include <QSignalBlocker>
#include <QSize>
#include <QString>
#include <QTimer>
#include <QWidget>

#include "core/calendar.h"
#include "core/colours.h"
#include "core/datecalculation.h"
#include "core/datecalculator.h"
#include "core/layout.h"
#include "core/unitconversion.h"
#include "core/unitconverter.h"
#include "core/worksheetcalculator.h"
#include "core/worksheets.h"
#include "ui/fonts.h"
#include "ui/look.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

// --- Version ------------------------------------------------------------
// src/ui/sidepanels.cpp 0.8.13

// --- Version history ----------------------------------------------------
// v0.8.13: Initial: the side panel beside the calculator, its lists and boxes, and the Unit conversion panel (D-025);
//          the date boxes, number boxes with arrows, the panel's button and the Date calculation panel (D-026); the
//          Worksheet panel and its message balloon (D-027)

namespace chrono_calc {

namespace {

// How high the figures of the labels and lists are, and those of the boxes (Windows 7: "From" 9 px, "12,7" 12 px)
constexpr double labelInkHeight = 9.0;
constexpr double boxInkHeight = 12.0;
// The space before the text of a list or a box, and the arrow of a list: 7 px wide, 4 high, its middle 23 px from the
// right edge (Windows 7: near x 695 of a list ending at 708)
constexpr int textInset = 4;
constexpr int arrowWidth = 7;
constexpr int arrowHeight = 4;
constexpr int arrowFromRight = 13;

QString fromView(std::string_view text)
{
    return QString::fromUtf8(text.data(), static_cast<qsizetype>(text.size()));
}

QRect toRect(Rectangle rectangle)
{
    return {rectangle.left, rectangle.top, rectangle.width, rectangle.height};
}

QLabel* panelLabel(const QString& text, Rectangle place, QWidget* parent)
{
    auto* label = new QLabel(text, parent);
    label->setGeometry(toRect(place));
    label->setFont(fontWithInkHeight(label->font(), labelInkHeight));
    label->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    QPalette palette = label->palette();
    palette.setColor(QPalette::WindowText, qtColour(colours::panelLabel));
    label->setPalette(palette);
    return label;
}

// A list's arrow pointing down (or up), its top left corner given
void drawArrow(QPainter& painter, QPoint corner, bool up, Colour colour)
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(qtColour(colour));
    const int left = corner.x();
    const int top = corner.y();
    if (up) {
        painter.drawPolygon(QPolygon({QPoint(left, top + arrowHeight), QPoint(left + arrowWidth, top + arrowHeight),
                                      QPoint(left + (arrowWidth / 2), top)}));
    } else {
        painter.drawPolygon(QPolygon({QPoint(left, top), QPoint(left + arrowWidth, top),
                                      QPoint(left + (arrowWidth / 2), top + arrowHeight)}));
    }
    painter.restore();
}

// The parts of one mode of the Date calculation panel lie below its list, from this height in the panel on, so that
// they never cover the list
constexpr int datePartTop = 75;
constexpr int datePartHeight = 189;

QRect inDatePart(Rectangle rectangle)
{
    return {rectangle.left, rectangle.top - datePartTop, rectangle.width, rectangle.height};
}

// The Worksheets' boxes: Windows 7's digits are 11 px high there (windows7-worksheets.md part A line 4)
constexpr double worksheetBoxInkHeight = 11.0;

// The balloon of a message (our own drawing): its point 8 px high, 16 px from its left edge, the text 10 px inside; it
// starts 3 px left of the box and 21 px below its top (Windows 7: at 486, 203 for the box at 489, 182)
constexpr int balloonPointHeight = 8;
constexpr int balloonPointLeft = 16;
constexpr int balloonPadding = 10;
constexpr QPoint balloonOffset{-3, 21};
// Windows 7's balloon goes after 8.7 seconds (part D line 4: 8686 and 8736 ms measured)
constexpr int balloonMilliseconds = 8700;

// A box's text as the calculator holds it; the cursor stays after the last character taken
void showBoxText(QLineEdit* box, const QString& text)
{
    if (box->text() == text) {
        return;
    }
    const qsizetype refused = box->text().size() - text.size();
    const qsizetype cursor = box->cursorPosition() - (refused > 0 ? refused : 0);
    box->setText(text);
    box->setCursorPosition(static_cast<int>(cursor < 0 ? 0 : cursor));
}

QDate qtDate(const Date& date)
{
    return {date.year, date.month, date.day};
}

// The regional short date, always with four digits for the year (a date box reaches from 1601 to 9999)
QString shortDateFormat()
{
    QString format = QLocale().dateFormat(QLocale::ShortFormat);
    if (!format.contains(QStringLiteral("yyyy"))) {
        format.replace(QStringLiteral("yy"), QStringLiteral("yyyy"));
    }
    return format;
}

}  // namespace

void SidePanelFrame::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.fillRect(rect(), qtColour(colours::panelFace));
    painter.setPen(qtColour(colours::panelFrame));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
}

PanelList::PanelList(QWidget* parent)
    : QComboBox(parent)
{
    setEditable(false);
    setFocusPolicy(Qt::StrongFocus);
    setFont(fontWithInkHeight(font(), labelInkHeight));
    // The open list: white rows, the chosen one in the focus blue with white text
    QPalette palette = view()->palette();
    palette.setColor(QPalette::Base, qtColour(colours::panelBoxFace));
    palette.setColor(QPalette::Text, qtColour(colours::panelText));
    palette.setColor(QPalette::Highlight, qtColour(colours::panelBoxFocusEdge));
    palette.setColor(QPalette::HighlightedText, qtColour(colours::panelBoxFace));
    view()->setPalette(palette);
}

void PanelList::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.fillRect(rect(), qtColour(colours::panelListFace));
    painter.setPen(qtColour(hasFocus() ? colours::panelBoxFocusEdge : colours::panelListEdge));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
    painter.setPen(qtColour(colours::panelText));
    painter.setFont(font());
    const QRect textArea = rect().adjusted(textInset, 0, -(arrowFromRight + arrowWidth + textInset), 0);
    painter.drawText(textArea, Qt::AlignLeft | Qt::AlignVCenter, currentText());
    const int arrowLeft = width() - arrowFromRight - arrowWidth;
    const int arrowTop = (height() - arrowHeight) / 2;
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(qtColour(colours::panelListArrow));
    painter.drawPolygon(QPolygon({QPoint(arrowLeft, arrowTop), QPoint(arrowLeft + arrowWidth, arrowTop),
                                  QPoint(arrowLeft + (arrowWidth / 2), arrowTop + arrowHeight)}));
}

void PanelList::keyPressEvent(QKeyEvent* event)
{
    const int key = event->key();
    const bool alt = (event->modifiers() & Qt::AltModifier) != 0;
    if (key == Qt::Key_Up || key == Qt::Key_Down || key == Qt::Key_F4 || (alt && (key == Qt::Key_Up || key == Qt::Key_Down))) {
        QComboBox::keyPressEvent(event);
        return;
    }
    if (key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_Escape) {
        emit focusToCalculator();
    }
    // The key goes on to the window, which gives it to the calculator
    event->ignore();
}

PanelBox::PanelBox(bool readOnly, QWidget* parent)
    : QLineEdit(parent)
{
    setReadOnly(readOnly);
    setFrame(false);
    setFocusPolicy(Qt::StrongFocus);
    setTextMargins(textInset, 0, textInset, 0);
    setFont(fontWithInkHeight(font(), boxInkHeight));
    QPalette palette = this->palette();
    const Colour face = readOnly ? colours::panelReadOnlyFace : colours::panelBoxFace;
    for (const auto group : {QPalette::Active, QPalette::Inactive, QPalette::Disabled}) {
        palette.setColor(group, QPalette::Base, qtColour(face));
        palette.setColor(group, QPalette::Text, qtColour(colours::panelText));
        palette.setColor(group, QPalette::Highlight, qtColour(colours::panelBoxFocusEdge));
        palette.setColor(group, QPalette::HighlightedText, qtColour(colours::panelBoxFace));
    }
    setPalette(palette);
}

void PanelBox::paintEvent(QPaintEvent* event)
{
    QLineEdit::paintEvent(event);
    QPainter painter(this);
    painter.setPen(qtColour(hasFocus() ? colours::panelBoxFocusEdge : colours::panelBoxEdge));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
}

void PanelBox::limitTyping(int characters)
{
    m_typingLimit = characters;
}

void PanelBox::keyPressEvent(QKeyEvent* event)
{
    // A character the box would insert: printable, and not a Ctrl shortcut (QLineEdit inserts Alt, AltGr and Meta ones)
    const QString typed = event->text();
    const Qt::KeyboardModifiers held = event->modifiers() & ~(Qt::ShiftModifier | Qt::KeypadModifier);
    const bool printable = !typed.isEmpty() && typed.front().isPrint() && held != Qt::ControlModifier;
    if (printable && m_typingLimit > 0 && text().size() - selectedText().size() >= m_typingLimit) {
        event->accept();
        return;
    }
    const int key = event->key();
    if (key != Qt::Key_Return && key != Qt::Key_Enter && key != Qt::Key_Escape && key != Qt::Key_F9) {
        QLineEdit::keyPressEvent(event);
    }
    // Nothing typed into the box goes on to the calculator
    event->accept();
}

PanelDate::PanelDate(QWidget* parent)
    : QDateEdit(parent)
{
    setFrame(false);
    setFocusPolicy(Qt::StrongFocus);
    setDisplayFormat(shortDateFormat());
    setDateRange(qtDate(firstDate), qtDate(lastDate));
    setCalendarPopup(true);
    setWrapping(true);
    setFont(fontWithInkHeight(font(), boxInkHeight));
    QPalette palette = this->palette();
    for (const auto group : {QPalette::Active, QPalette::Inactive, QPalette::Disabled}) {
        palette.setColor(group, QPalette::Base, qtColour(colours::panelBoxFace));
        palette.setColor(group, QPalette::Button, qtColour(colours::panelBoxFace));
        palette.setColor(group, QPalette::Text, qtColour(colours::panelText));
        palette.setColor(group, QPalette::Highlight, qtColour(colours::panelBoxFocusEdge));
        palette.setColor(group, QPalette::HighlightedText, qtColour(colours::panelBoxFace));
    }
    setPalette(palette);
}

// The day and the month go round (31 January Up is 1 January), the year stops at 1601 and 9999 (part C lines 3, 4)
void PanelDate::stepBy(int steps)
{
    setWrapping(currentSection() != QDateTimeEdit::YearSection);
    QDateEdit::stepBy(steps);
    setWrapping(true);
}

void PanelDate::paintEvent(QPaintEvent* event)
{
    QDateEdit::paintEvent(event);
    // Our own drop-down arrow and edge over the style's
    QPainter painter(this);
    const int buttonWidth = arrowFromRight + arrowWidth;
    painter.fillRect(QRect(width() - buttonWidth - 1, 1, buttonWidth, height() - 2), qtColour(colours::panelBoxFace));
    drawArrow(painter, QPoint(width() - arrowFromRight - arrowWidth, (height() - arrowHeight) / 2), false, colours::panelDateArrow);
    painter.setPen(qtColour(hasFocus() ? colours::panelBoxFocusEdge : colours::panelBoxEdge));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
}

void PanelDate::keyPressEvent(QKeyEvent* event)
{
    const int key = event->key();
    if (key == Qt::Key_Return || key == Qt::Key_Enter) {
        // A digit still waiting for its second one is taken first, so that Calculate uses the date shown
        interpretText();
        emit calculateChosen();
    } else if (key != Qt::Key_Escape) {
        QDateEdit::keyPressEvent(event);
    }
    // Nothing typed into the box goes on to the calculator (part F line 2)
    event->accept();
}

bool PanelDate::focusNextPrevChild(bool next)
{
    // QDateTimeEdit's own moves between the parts of the date; Windows 7's Tab leaves the box
    // NOLINTNEXTLINE(bugprone-parent-virtual-call): the widget's own Tab, skipping QDateTimeEdit's on purpose
    return QWidget::focusNextPrevChild(next);
}

PanelNumberBox::PanelNumberBox(QWidget* parent)
    : PanelBox(false, parent)
{
    setMaxLength(static_cast<int>(DateCalculator::mostDigits));
}

void PanelNumberBox::keyPressEvent(QKeyEvent* event)
{
    const int key = event->key();
    if (key == Qt::Key_Up || key == Qt::Key_Down) {
        emit stepped(key == Qt::Key_Up);
        event->accept();
        return;
    }
    if (key == Qt::Key_Return || key == Qt::Key_Enter) {
        emit calculateChosen();
        event->accept();
        return;
    }
    PanelBox::keyPressEvent(event);
}

PanelArrows::PanelArrows(QWidget* parent)
    : QWidget(parent)
{
    setFocusPolicy(Qt::NoFocus);
}

void PanelArrows::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    const int half = height() / 2;
    for (const QRect& part : {QRect(0, 0, width(), half), QRect(0, half, width(), height() - half)}) {
        painter.fillRect(part, qtColour(colours::panelListFace));
        painter.setPen(qtColour(colours::panelListEdge));
        painter.drawRect(part.adjusted(0, 0, -1, -1));
    }
    const int left = (width() - arrowWidth) / 2;
    drawArrow(painter, QPoint(left, (half - arrowHeight) / 2), true, colours::panelListArrow);
    drawArrow(painter, QPoint(left, half + ((height() - half - arrowHeight) / 2)), false, colours::panelListArrow);
}

void PanelArrows::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit stepped(event->position().y() < height() / 2.0);
    }
    event->accept();
}

PanelButton::PanelButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setFont(fontWithInkHeight(font(), labelInkHeight));
}

void PanelButton::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.fillRect(rect(), qtColour(colours::panelButtonFace));
    painter.setPen(qtColour(colours::panelButtonEdge));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
    if (hasFocus()) {
        painter.drawRect(rect().adjusted(1, 1, -2, -2));
    }
    painter.setPen(qtColour(colours::panelText));
    painter.setFont(font());
    painter.drawText(rect(), Qt::AlignCenter, text());
}

void PanelButton::keyPressEvent(QKeyEvent* event)
{
    const int key = event->key();
    if (key == Qt::Key_Return || key == Qt::Key_Enter || key == Qt::Key_Space) {
        click();
        event->accept();
        return;
    }
    // The key goes on to the window, which gives it to the calculator
    event->ignore();
}

UnitConversionPanel::UnitConversionPanel(QWidget* parent)
    : SidePanelFrame(parent)
    , m_typeList(new PanelList(this))
    , m_fromBox(new PanelBox(false, this))
    , m_fromList(new PanelList(this))
    , m_toBox(new PanelBox(true, this))
    , m_toList(new PanelList(this))
{
    const UnitConversionLayout& layout = unitConversionLayout();
    panelLabel(tr("Select the type of unit you want to convert"), layout.typeLabel, this);
    panelLabel(tr("From"), layout.fromLabel, this);
    panelLabel(tr("To"), layout.toLabel, this);
    m_typeList->setGeometry(toRect(layout.typeList));
    m_fromBox->setGeometry(toRect(layout.fromBox));
    m_fromList->setGeometry(toRect(layout.fromList));
    m_toBox->setGeometry(toRect(layout.toBox));
    m_toList->setGeometry(toRect(layout.toList));
    m_typeList->setObjectName(QStringLiteral("unit.type"));
    m_fromBox->setObjectName(QStringLiteral("unit.input"));
    // A 26th typed character is refused, not taken in place of the last one (part G line 3)
    m_fromBox->limitTyping(static_cast<int>(mostNumberCharacters));
    m_fromList->setObjectName(QStringLiteral("unit.from"));
    m_toBox->setObjectName(QStringLiteral("unit.result"));
    m_toList->setObjectName(QStringLiteral("unit.to"));
    for (const UnitTypeSpec& type : unitTypes()) {
        m_typeList->addItem(fromView(type.name));
    }
    connect(m_typeList, &QComboBox::currentIndexChanged, this, [this](int index) { emit typeChosen(index); });
    connect(m_fromList, &QComboBox::currentIndexChanged, this, [this](int index) { emit fromChosen(index); });
    connect(m_toList, &QComboBox::currentIndexChanged, this, [this](int index) { emit toChosen(index); });
    connect(m_fromBox, &QLineEdit::textEdited, this, [this](const QString& text) { emit inputEdited(text); });
    for (const PanelList* list : {m_typeList, m_fromList, m_toList}) {
        connect(list, &PanelList::focusToCalculator, this, [this] { emit focusToCalculator(); });
    }
}

void UnitConversionPanel::fillUnits(int type)
{
    m_fromList->clear();
    m_toList->clear();
    for (const UnitSpec& unit : unitTypes()[static_cast<std::size_t>(type)].units) {
        m_fromList->addItem(fromView(unit.name));
        m_toList->addItem(fromView(unit.name));
    }
    m_shownType = type;
}

void UnitConversionPanel::showState(const UnitConverter& converter)
{
    const QSignalBlocker typeBlocker(m_typeList);
    const QSignalBlocker fromBlocker(m_fromList);
    const QSignalBlocker toBlocker(m_toList);
    const auto type = static_cast<int>(converter.type());
    if (type != m_shownType) {
        fillUnits(type);
    }
    m_typeList->setCurrentIndex(type);
    m_fromList->setCurrentIndex(static_cast<int>(converter.from()));
    m_toList->setCurrentIndex(static_cast<int>(converter.to()));
    // The From box shows what it took of the typed text
    showBoxText(m_fromBox, QString::fromStdWString(converter.input()));
    m_toBox->setText(QString::fromStdWString(converter.result()));
}

QWidget* UnitConversionPanel::fromBox() const
{
    return m_fromBox;
}

QWidget* UnitConversionPanel::fromList() const
{
    return m_fromList;
}

QWidget* UnitConversionPanel::toBox() const
{
    return m_toBox;
}

QWidget* UnitConversionPanel::toList() const
{
    return m_toList;
}

QWidget* UnitConversionPanel::typeList() const
{
    return m_typeList;
}

DateCalculationPanel::DateCalculationPanel(QWidget* parent)
    : SidePanelFrame(parent)
    , m_modeList(new PanelList(this))
    , m_differencePart(new QWidget(this))
    , m_shiftPart(new QWidget(this))
    , m_differenceFrom(new PanelDate(m_differencePart))
    , m_differenceTo(new PanelDate(m_differencePart))
    , m_difference(new PanelBox(true, m_differencePart))
    , m_totalDays(new PanelBox(true, m_differencePart))
    , m_shiftFrom(new PanelDate(m_shiftPart))
    , m_add(new QRadioButton(tr("Add"), m_shiftPart))
    , m_subtract(new QRadioButton(tr("Subtract"), m_shiftPart))
    , m_shifted(new PanelBox(true, m_shiftPart))
    , m_calculate(new PanelButton(tr("Calculate"), this))
{
    const DateCalculationLayout& layout = dateCalculationLayout();
    panelLabel(tr("Select the date calculation you want"), layout.modeLabel, this);
    m_modeList->setGeometry(toRect(layout.modeList));
    m_modeList->addItem(tr("Calculate the difference between two dates"));
    m_modeList->addItem(tr("Add or subtract days to a specified date"));
    m_calculate->setGeometry(toRect(layout.calculate));
    for (QWidget* part : {m_differencePart, m_shiftPart}) {
        part->setGeometry(0, datePartTop, toRect(layout.modeList).right() + 1, datePartHeight);
    }

    panelLabel(tr("From"), layout.differenceFromLabel, m_differencePart)->move(inDatePart(layout.differenceFromLabel).topLeft());
    panelLabel(tr("To"), layout.differenceToLabel, m_differencePart)->move(inDatePart(layout.differenceToLabel).topLeft());
    panelLabel(tr("Difference (years, months, weeks, days)"), layout.differenceLabel, m_differencePart)
        ->move(inDatePart(layout.differenceLabel).topLeft());
    panelLabel(tr("Difference (days)"), layout.totalDaysLabel, m_differencePart)->move(inDatePart(layout.totalDaysLabel).topLeft());
    m_differenceFrom->setGeometry(inDatePart(layout.differenceFrom));
    m_differenceTo->setGeometry(inDatePart(layout.differenceTo));
    m_difference->setGeometry(inDatePart(layout.difference));
    m_totalDays->setGeometry(inDatePart(layout.totalDays));

    panelLabel(tr("From"), layout.shiftFromLabel, m_shiftPart)->move(inDatePart(layout.shiftFromLabel).topLeft());
    m_shiftFrom->setGeometry(inDatePart(layout.shiftFrom));
    m_add->setGeometry(inDatePart(layout.add));
    m_subtract->setGeometry(inDatePart(layout.subtract));
    for (QRadioButton* choice : {m_add, m_subtract}) {
        choice->installEventFilter(this);
        choice->setFont(fontWithInkHeight(choice->font(), labelInkHeight));
        QPalette palette = choice->palette();
        palette.setColor(QPalette::WindowText, qtColour(colours::panelText));
        palette.setColor(QPalette::Base, qtColour(colours::panelBoxFace));
        choice->setPalette(palette);
    }
    const std::array fieldNames{tr("Year(s)"), tr("Month(s)"), tr("Day(s)")};
    for (std::size_t index = 0; index < m_fields.size(); ++index) {
        panelLabel(fieldNames.at(index), layout.fieldLabels.at(index), m_shiftPart)
            ->move(inDatePart(layout.fieldLabels.at(index)).topLeft());
        auto* field = new PanelNumberBox(m_shiftPart);
        field->setGeometry(inDatePart(layout.fields.at(index)));
        auto* arrows = new PanelArrows(m_shiftPart);
        arrows->setGeometry(inDatePart(layout.fieldArrows.at(index)));
        const auto fieldIndex = static_cast<int>(index);
        connect(field, &QLineEdit::textEdited, this, [this, fieldIndex](const QString& text) { emit fieldEdited(fieldIndex, text); });
        connect(field, &PanelNumberBox::stepped, this, [this, fieldIndex](bool up) { emit fieldStepped(fieldIndex, up); });
        connect(arrows, &PanelArrows::stepped, this, [this, fieldIndex](bool up) { emit fieldStepped(fieldIndex, up); });
        connect(field, &PanelNumberBox::calculateChosen, this, [this] { calculateFromKey(); });
        m_fields.at(index) = field;
    }
    panelLabel(tr("Date"), layout.shiftedLabel, m_shiftPart)->move(inDatePart(layout.shiftedLabel).topLeft());
    m_shifted->setGeometry(inDatePart(layout.shifted));

    m_modeList->setObjectName(QStringLiteral("date.mode"));
    m_differenceFrom->setObjectName(QStringLiteral("date.difference_from"));
    m_differenceTo->setObjectName(QStringLiteral("date.difference_to"));
    m_difference->setObjectName(QStringLiteral("date.difference"));
    m_totalDays->setObjectName(QStringLiteral("date.total_days"));
    m_shiftFrom->setObjectName(QStringLiteral("date.shift_from"));
    m_add->setObjectName(QStringLiteral("date.add"));
    m_subtract->setObjectName(QStringLiteral("date.subtract"));
    m_fields.at(0)->setObjectName(QStringLiteral("date.years"));
    m_fields.at(1)->setObjectName(QStringLiteral("date.months"));
    m_fields.at(2)->setObjectName(QStringLiteral("date.days"));
    m_shifted->setObjectName(QStringLiteral("date.shifted"));
    m_calculate->setObjectName(QStringLiteral("date.calculate"));

    connect(m_modeList, &QComboBox::currentIndexChanged, this, [this](int index) { emit modeChosen(index); });
    connect(m_modeList, &PanelList::focusToCalculator, this, [this] { emit focusToCalculator(); });
    connect(m_differenceFrom, &QDateEdit::dateChanged, this, [this](QDate date) { emit differenceFromChosen(date); });
    connect(m_differenceTo, &QDateEdit::dateChanged, this, [this](QDate date) { emit differenceToChosen(date); });
    connect(m_shiftFrom, &QDateEdit::dateChanged, this, [this](QDate date) { emit shiftFromChosen(date); });
    for (const PanelDate* date : {m_differenceFrom, m_differenceTo, m_shiftFrom}) {
        connect(date, &PanelDate::calculateChosen, this, [this] { calculateFromKey(); });
    }
    connect(m_add, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) {
            emit subtractChosen(false);
        }
    });
    connect(m_subtract, &QRadioButton::toggled, this, [this](bool checked) {
        if (checked) {
            emit subtractChosen(true);
        }
    });
    connect(m_calculate, &QPushButton::clicked, this, [this] { emit calculateChosen(); });
}

bool DateCalculationPanel::eventFilter(QObject* watched, QEvent* event)
{
    const auto* keyEvent = dynamic_cast<const QKeyEvent*>(event);
    if ((watched == m_add || watched == m_subtract) && event->type() == QEvent::KeyPress && keyEvent != nullptr) {
        const int key = keyEvent->key();
        if (key == Qt::Key_Left || key == Qt::Key_Up || key == Qt::Key_Right || key == Qt::Key_Down) {
            QRadioButton* chosen = (key == Qt::Key_Left || key == Qt::Key_Up) ? m_add : m_subtract;
            chosen->setFocus(Qt::TabFocusReason);
            chosen->click();
            return true;
        }
    }
    return SidePanelFrame::eventFilter(watched, event);
}

void DateCalculationPanel::calculateFromKey()
{
    emit calculateChosen();
    (m_modeList->currentIndex() == 0 ? m_difference : m_shifted)->setFocus();
}

void DateCalculationPanel::showState(const DateCalculator& calculator)
{
    const QSignalBlocker modeBlocker(m_modeList);
    const QSignalBlocker fromBlocker(m_differenceFrom);
    const QSignalBlocker toBlocker(m_differenceTo);
    const QSignalBlocker shiftBlocker(m_shiftFrom);
    const QSignalBlocker addBlocker(m_add);
    const QSignalBlocker subtractBlocker(m_subtract);
    const bool difference = calculator.mode() == DateMode::Difference;
    m_modeList->setCurrentIndex(difference ? 0 : 1);
    m_differencePart->setVisible(difference);
    m_shiftPart->setVisible(!difference);
    // A date box keeps what is being typed into it while its date stays the same
    const auto showDate = [](PanelDate* box, const Date& date) {
        if (box->date() != qtDate(date)) {
            box->setDate(qtDate(date));
        }
    };
    showDate(m_differenceFrom, calculator.differenceFrom());
    showDate(m_differenceTo, calculator.differenceTo());
    showDate(m_shiftFrom, calculator.shiftFrom());
    m_difference->setText(QString::fromStdWString(calculator.differenceText()));
    m_totalDays->setText(QString::fromStdWString(calculator.totalDaysText()));
    (calculator.isSubtract() ? m_subtract : m_add)->setChecked(true);
    // Tab reaches only the chosen one (part F line 1); a click still gives the other one the focus
    m_add->setFocusPolicy(calculator.isSubtract() ? Qt::ClickFocus : Qt::StrongFocus);
    m_subtract->setFocusPolicy(calculator.isSubtract() ? Qt::StrongFocus : Qt::ClickFocus);
    for (std::size_t index = 0; index < m_fields.size(); ++index) {
        showBoxText(m_fields.at(index), QString::fromStdWString(calculator.field(static_cast<DateField>(index))));
    }
    const ShiftedDate& shifted = calculator.shifted();
    QString shiftedText;
    if (shifted.shown) {
        shiftedText = shifted.date ? QLocale().toString(qtDate(*shifted.date), QLocale::LongFormat)
                                   : QString::fromStdWString(std::wstring(outOfBoundText));
    }
    m_shifted->setText(shiftedText);
}

std::vector<QWidget*> DateCalculationPanel::tabOrder() const
{
    // Difference: From, To, both results, Calculate; Add or subtract: From, Add or Subtract (the one chosen), Year(s),
    // Month(s), Day(s), the result, Calculate. The window puts the calculator and then the mode list after Calculate.
    return {m_differenceFrom, m_differenceTo, m_difference, m_totalDays, m_shiftFrom, m_add, m_subtract,
            m_fields.at(0), m_fields.at(1), m_fields.at(2), m_shifted, m_calculate};
}

QWidget* DateCalculationPanel::modeList() const
{
    return m_modeList;
}

PanelBalloon::PanelBalloon(QWidget* parent)
    : QWidget(parent, Qt::ToolTip | Qt::FramelessWindowHint)
{
    setObjectName(QStringLiteral("worksheet.balloon"));
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_TranslucentBackground);
    setFont(fontWithInkHeight(font(), labelInkHeight));
    m_timer.setSingleShot(true);
    m_timer.setInterval(balloonMilliseconds);
    connect(&m_timer, &QTimer::timeout, this, &PanelBalloon::dismiss);
}

void PanelBalloon::showAt(QWidget* box, const QString& text)
{
    dismiss();
    m_text = text;
    m_box = box;
    m_window = box->window();
    box->installEventFilter(this);
    m_window->installEventFilter(this);
    const QSize textSize = fontMetrics().size(Qt::TextSingleLine, text);
    resize(textSize.width() + (2 * balloonPadding), textSize.height() + (2 * balloonPadding) + balloonPointHeight);
    move(box->mapToGlobal(balloonOffset));
    show();
    raise();
    m_timer.start();
}

void PanelBalloon::dismiss()
{
    m_timer.stop();
    if (m_box != nullptr) {
        m_box->removeEventFilter(this);
    }
    if (m_window != nullptr) {
        m_window->removeEventFilter(this);
    }
    m_box = nullptr;
    m_window = nullptr;
    hide();
}

void PanelBalloon::paintEvent(QPaintEvent* /*event*/)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(qtColour(colours::panelBalloonEdge));
    painter.setBrush(qtColour(colours::panelBalloonFace));
    const QRect body = rect().adjusted(0, balloonPointHeight, -1, -1);
    painter.drawRoundedRect(body, 3, 3);
    // The point up to the box, drawn over the body's edge
    const QPolygon point({QPoint(balloonPointLeft, body.top() + 1), QPoint(balloonPointLeft, 0),
                          QPoint(balloonPointLeft + balloonPointHeight, body.top() + 1)});
    painter.drawPolygon(point);
    painter.setPen(Qt::NoPen);
    painter.drawRect(balloonPointLeft + 1, body.top(), balloonPointHeight - 1, 2);
    painter.setPen(qtColour(colours::panelText));
    painter.drawText(body.adjusted(balloonPadding, 0, -balloonPadding, 0), Qt::AlignLeft | Qt::AlignVCenter, m_text);
}

bool PanelBalloon::eventFilter(QObject* watched, QEvent* event)
{
    // The box loses the keyboard focus, or a key moves its cursor (Windows 7: Right; a typed character keeps it)
    if (watched == m_box) {
        const auto* keyEvent = dynamic_cast<const QKeyEvent*>(event);
        const bool cursorKey = event->type() == QEvent::KeyPress && keyEvent != nullptr
                               && (keyEvent->key() == Qt::Key_Left || keyEvent->key() == Qt::Key_Right
                                   || keyEvent->key() == Qt::Key_Home || keyEvent->key() == Qt::Key_End);
        if (cursorKey || event->type() == QEvent::FocusOut || event->type() == QEvent::Hide) {
            dismiss();
        }
    } else if (watched == m_window && event->type() == QEvent::Move) {
        // The balloon does not follow a moved window: it goes
        dismiss();
    }
    return QWidget::eventFilter(watched, event);
}

WorksheetPanel::WorksheetPanel(Worksheet worksheet, QWidget* parent)
    : SidePanelFrame(parent)
    , m_worksheet(worksheet)
    , m_choiceList(new PanelList(this))
    , m_calculate(new PanelButton(tr("Calculate"), this))
    , m_result(new PanelBox(true, this))
    , m_balloon(new PanelBalloon(this))
{
    const WorksheetLayout& layout = worksheetLayout();
    const WorksheetSpec& spec = worksheetSpec(worksheet);
    panelLabel(tr("Select the value you want to calculate"), layout.choiceLabel, this);
    m_choiceList->setGeometry(toRect(layout.choiceList));
    for (const std::size_t choice : spec.choices) {
        m_choiceList->addItem(fromView(spec.values.at(choice)));
    }
    m_choiceList->installEventFilter(this);
    m_choiceList->setObjectName(QStringLiteral("worksheet.list"));
    // One box fewer than the worksheet has values: the chosen one is the result
    for (std::size_t index = 0; index + 1 < spec.values.size() && index < mostWorksheetBoxes; ++index) {
        m_labels.push_back(panelLabel({}, layout.valueLabels.at(index), this));
        m_labels.back()->setObjectName(QStringLiteral("worksheet.label.%1").arg(index));
        auto* box = new PanelBox(false, this);
        box->setGeometry(toRect(layout.boxes.at(index)));
        box->setFont(fontWithInkHeight(box->font(), worksheetBoxInkHeight));
        // An empty box shows the grey hint "Enter value" in italics (part A line 3)
        box->setPlaceholderText(tr("Enter value"));
        QPalette palette = box->palette();
        palette.setColor(QPalette::PlaceholderText, qtColour(colours::panelHint));
        box->setPalette(palette);
        const auto italicWhenEmpty = [box](const QString& text) {
            QFont font = box->font();
            font.setItalic(text.isEmpty());
            box->setFont(font);
        };
        italicWhenEmpty({});
        connect(box, &QLineEdit::textChanged, this, italicWhenEmpty);
        box->setObjectName(QStringLiteral("worksheet.box.%1").arg(index));
        box->limitTyping(static_cast<int>(mostNumberCharacters));
        box->installEventFilter(this);
        connect(box, &QLineEdit::textEdited, this, [this, index](const QString& text) {
            if (index < m_boxValues.size()) {
                emit valueEdited(static_cast<int>(m_boxValues.at(index)), text);
            }
        });
        m_boxes.push_back(box);
    }
    m_calculate->setGeometry(toRect(layout.calculate));
    m_calculate->setObjectName(QStringLiteral("worksheet.calculate"));
    m_result->setGeometry(toRect(layout.result));
    m_result->setFont(fontWithInkHeight(m_result->font(), worksheetBoxInkHeight));
    m_result->setObjectName(QStringLiteral("worksheet.result"));
    // Another value relabels the boxes: a balloon would point at a box that now holds another value
    connect(m_choiceList, &QComboBox::currentIndexChanged, this, [this](int index) {
        m_balloon->dismiss();
        emit choiceChosen(index);
    });
    connect(m_choiceList, &PanelList::focusToCalculator, this, [this] { emit focusToCalculator(); });
    connect(m_calculate, &QPushButton::clicked, this, [this] { emit calculateChosen(); });
}

bool WorksheetPanel::eventFilter(QObject* watched, QEvent* event)
{
    const auto* keyEvent = dynamic_cast<const QKeyEvent*>(event);
    if (event->type() != QEvent::KeyPress || keyEvent == nullptr) {
        return SidePanelFrame::eventFilter(watched, event);
    }
    const int key = keyEvent->key();
    const bool enter = key == Qt::Key_Return || key == Qt::Key_Enter;
    const bool isBox = std::ranges::find(m_boxes, watched) != m_boxes.end();
    if (enter && (isBox || watched == m_choiceList)) {
        emit calculateChosen();
        return true;
    }
    if (watched == m_choiceList) {
        // The list changes with Up and Down, opens with F4 and Alt+Down; Tab moves on; Escape gives the calculator the
        // keyboard focus as in the other panels; no other key reaches the calculator (part F line 2)
        const bool listKey = key == Qt::Key_Up || key == Qt::Key_Down || key == Qt::Key_F4 || key == Qt::Key_Tab
                             || key == Qt::Key_Backtab || key == Qt::Key_Escape;
        return !listKey;
    }
    return SidePanelFrame::eventFilter(watched, event);
}

void WorksheetPanel::showState(const WorksheetCalculator& calculator)
{
    const QSignalBlocker listBlocker(m_choiceList);
    const WorksheetSpec& spec = worksheetSpec(m_worksheet);
    const WorksheetState& state = calculator.state(m_worksheet);
    m_choiceList->setCurrentIndex(static_cast<int>(calculator.chosenItem(m_worksheet)));
    m_boxValues = calculator.boxValues(m_worksheet);
    for (std::size_t index = 0; index < m_boxes.size() && index < m_boxValues.size(); ++index) {
        const std::size_t value = m_boxValues.at(index);
        m_labels.at(index)->setText(fromView(spec.values.at(value)));
        showBoxText(m_boxes.at(index), QString::fromStdWString(state.texts.at(value)));
    }
    m_result->setText(QString::fromStdWString(state.texts.at(state.chosen)));
}

void WorksheetPanel::showCalculated(const WorksheetCalculator& calculator)
{
    const WorksheetState& state = calculator.state(m_worksheet);
    PanelBox* target = m_result;
    if (state.message == WorksheetMessage::Blank || state.message == WorksheetMessage::NotValid) {
        const auto found = std::ranges::find(m_boxValues, state.messageValue);
        const auto index = static_cast<std::size_t>(found - m_boxValues.begin());
        if (index < m_boxes.size()) {
            target = m_boxes.at(index);
        }
    }
    target->setFocus();
    if (state.message == WorksheetMessage::None) {
        m_balloon->dismiss();
        return;
    }
    m_balloon->showAt(target, QString::fromStdWString(std::wstring(messageText(state.message))));
}

std::vector<QWidget*> WorksheetPanel::tabOrder() const
{
    std::vector<QWidget*> parts(m_boxes.begin(), m_boxes.end());
    parts.push_back(m_calculate);
    parts.push_back(m_result);
    return parts;
}

QWidget* WorksheetPanel::choiceList() const
{
    return m_choiceList;
}

}  // namespace chrono_calc
