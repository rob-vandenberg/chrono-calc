#include "core/calculator.h"
#include "core/commands.h"
#include "core/keys.h"
#include "core/view.h"

#include <cstddef>
#include <iostream>
#include <span>
#include <string>
#include <string_view>
#include <optional>
#include <utility>
#include <vector>

// --- Version ------------------------------------------------------------
// src/app/cli.cpp 0.5.12

// --- Version history ----------------------------------------------------
// v0.5.12: Commands with a text (edit.paste=TEXT); the history, its selected entry and the copied text
// v0.4.11: In the Statistics view also the count, the list and the selected item
// v0.2.7: Typed characters are read in the calculator's current view (the Scientific keys)
// v0.0.4: Initial: runs key presses and commands without a screen and prints what the calculator shows

// chrono-calc-cli: for tests, AI agents and quick result checks (AGENTS.md 4.3). Each argument is a command id
// (memory.store, clear_entry), a command with its text (edit.paste=2+3=, history.recalculate=6*7) or a run of typed
// characters (12+3=). Prints the display, the expression line, whether an error shows and whether a number is in
// memory; in the Statistics view also the count, the list (values separated by " | ") and the selected item (from 0,
// "none"); where the view has a history that is not empty, the history and its selected entry; and the text the last
// copy command gave ("\n" for a line end). Exit 2 for an argument it cannot read.
namespace {

std::string narrow(const std::wstring& text)
{
    std::string result;
    for (const wchar_t character : text) {
        const auto code = static_cast<char32_t>(character);
        if (code < 0x80) {
            result += static_cast<char>(code);
        } else if (code < 0x800) {
            result += static_cast<char>(0xC0 | (code >> 6));
            result += static_cast<char>(0x80 | (code & 0x3F));
        } else if (code < 0x10000) {
            result += static_cast<char>(0xE0 | (code >> 12));
            result += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
            result += static_cast<char>(0x80 | (code & 0x3F));
        } else {
            result += static_cast<char>(0xF0 | (code >> 18));
            result += static_cast<char>(0x80 | ((code >> 12) & 0x3F));
            result += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
            result += static_cast<char>(0x80 | (code & 0x3F));
        }
    }
    return result;
}

// The text of a command that takes one, read as UTF-8
std::wstring widen(std::string_view text)
{
    std::wstring result;
    for (std::size_t index = 0; index < text.size(); ++index) {
        const auto byte = static_cast<unsigned char>(text[index]);
        char32_t code = byte;
        std::size_t following = 0;
        if (byte >= 0xF0) {
            code = byte & 0x07U;
            following = 3;
        } else if (byte >= 0xE0) {
            code = byte & 0x0FU;
            following = 2;
        } else if (byte >= 0xC0) {
            code = byte & 0x1FU;
            following = 1;
        }
        for (; following > 0 && index + 1 < text.size(); --following) {
            code = (code << 6U) | (static_cast<unsigned char>(text[++index]) & 0x3FU);
        }
        result += static_cast<wchar_t>(code);
    }
    return result;
}

bool runArgument(chrono_calc::Calculator& calculator, std::string_view argument)
{
    if (chrono_calc::findCommand(argument)) {
        return calculator.run(argument);
    }
    if (const std::size_t equals = argument.find('='); equals != std::string_view::npos && chrono_calc::findCommand(argument.substr(0, equals))) {
        // "\n" in the text is a line end
        std::wstring text = widen(argument.substr(equals + 1));
        for (std::size_t found = text.find(L"\\n"); found != std::wstring::npos; found = text.find(L"\\n", found + 1)) {
            text.replace(found, 2, L"\n");
        }
        return calculator.run(argument.substr(0, equals), text);
    }
    for (const char character : argument) {
        const auto command = chrono_calc::commandForCharacter(static_cast<unsigned char>(character), calculator.view());
        if (!command) {
            std::cerr << "cannot read '" << character << "' in " << argument << '\n';
            return false;
        }
        calculator.run(*command);
    }
    return true;
}

}  // namespace

int main(int argc, char* argv[])
{
    chrono_calc::Calculator calculator;
    std::optional<std::wstring> copied;
    for (const char* argument : std::span(argv, static_cast<std::size_t>(argc)).subspan(1)) {
        if (!runArgument(calculator, argument)) {
            return 2;
        }
        if (std::optional<std::wstring> text = calculator.takeCopiedText()) {
            copied = std::move(text);
        }
    }
    std::cout << "display: " << narrow(calculator.display()) << '\n'
              << "expression: " << narrow(calculator.expression()) << '\n'
              << "error: " << (calculator.isError() ? "yes" : "no") << '\n'
              << "memory: " << (calculator.hasMemory() ? "yes" : "no") << '\n';
    if (calculator.view() == chrono_calc::View::Statistics) {
        std::cout << "count: " << calculator.statisticsCount() << '\n' << "values:";
        const char* separator = " ";
        for (const std::wstring& value : calculator.statisticsValues()) {
            std::cout << separator << narrow(value);
            separator = " | ";
        }
        const std::optional<std::size_t> selected = calculator.selectedValue();
        std::cout << '\n' << "selected: " << (selected ? std::to_string(*selected) : std::string("none")) << '\n';
    }
    if (const std::vector<std::wstring> entries = calculator.historyEntries(); !entries.empty()) {
        std::cout << "history:";
        const char* separator = " ";
        for (const std::wstring& entry : entries) {
            std::cout << separator << narrow(entry);
            separator = " | ";
        }
        const std::optional<std::size_t> selected = calculator.selectedHistoryEntry();
        std::cout << '\n' << "selected entry: " << (selected ? std::to_string(*selected) : std::string("none")) << '\n';
    }
    if (copied) {
        std::wstring shown = *copied;
        for (std::size_t found = shown.find(L'\n'); found != std::wstring::npos; found = shown.find(L'\n', found + 2)) {
            shown.replace(found, 1, L"\\n");
        }
        std::cout << "copied: " << narrow(shown) << '\n';
    }
    return 0;
}
