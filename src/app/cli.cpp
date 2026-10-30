#include "core/calculator.h"
#include "core/commands.h"
#include "core/keys.h"

#include <cstddef>
#include <iostream>
#include <span>
#include <string>
#include <string_view>

// --- Version ------------------------------------------------------------
// src/app/cli.cpp 0.2.7

// --- Version history ----------------------------------------------------
// v0.2.7: Typed characters are read in the calculator's current view (the Scientific keys)
// v0.0.4: Initial: runs key presses and commands without a screen and prints what the calculator shows

// chrono-calc-cli: for tests, AI agents and quick result checks (AGENTS.md 4.3). Each argument is a command id
// (memory.store, clear_entry) or a run of typed characters (12+3=). Prints the display, the expression line, whether
// an error shows and whether a number is in memory. Exit 2 for an argument it cannot read.
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

bool runArgument(chrono_calc::Calculator& calculator, std::string_view argument)
{
    if (chrono_calc::findCommand(argument)) {
        return calculator.run(argument);
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
    for (const char* argument : std::span(argv, static_cast<std::size_t>(argc)).subspan(1)) {
        if (!runArgument(calculator, argument)) {
            return 2;
        }
    }
    std::cout << "display: " << narrow(calculator.display()) << '\n'
              << "expression: " << narrow(calculator.expression()) << '\n'
              << "error: " << (calculator.isError() ? "yes" : "no") << '\n'
              << "memory: " << (calculator.hasMemory() ? "yes" : "no") << '\n';
    return 0;
}
