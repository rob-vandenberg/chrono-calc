#include <QObject>
#include <QTest>

#include <algorithm>
#include <array>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <regex>
#include <string>
#include <vector>

// --- Version ------------------------------------------------------------
// tests/test_layers.cpp 0.0.3

// --- Version history ----------------------------------------------------
// v0.0.3: third_party includes no Qt, core, ui and app
// v0.0.2: Initial: core includes no Qt, ui and app; ui includes no app; relative includes (../ui/x.h) are resolved first,
//         every C++ file type is read, and lowercase Qt headers (qstring.h) count as Qt

// Enforces OR-13 with D-011: the layers third_party <- core <- ui <- app. A layer includes only from the layers below it,
// and core and third_party are plain C++ without Qt. Reads every #include of every file in src/ and third_party/.

namespace {

struct Include
{
    std::string file;
    int line = 0;
    std::string target;
};

// The target of an include line ("core/version.h" or QWidget), or empty when the line is no include.
// Read by hand, not with a regex: Qt's moc misreads a regex literal that holds quotes and a hash sign.
std::string includeTarget(const std::string& text)
{
    std::size_t position = text.find_first_not_of(" \t");
    if (position == std::string::npos || text[position] != '#') {
        return {};
    }
    position = text.find_first_not_of(" \t", position + 1);
    const std::string keyword = "include";
    if (position == std::string::npos || text.compare(position, keyword.size(), keyword) != 0) {
        return {};
    }
    position = text.find_first_not_of(" \t", position + keyword.size());
    if (position == std::string::npos || (text[position] != '<' && text[position] != '"')) {
        return {};
    }
    const char closing = text[position] == '<' ? '>' : '"';
    const std::size_t end = text.find(closing, position + 1);
    return end == std::string::npos ? std::string() : text.substr(position + 1, end - position - 1);
}

// Every C++ file type that may hold an #include
bool isSourceFile(const std::filesystem::path& path)
{
    static const std::array<std::string, 10> extensions = {".cpp", ".cc", ".cxx", ".c", ".h", ".hpp", ".hh", ".hxx", ".inl", ".ipp"};
    const std::string extension = path.extension().string();
    return std::ranges::any_of(extensions, [&extension](const std::string& known) { return extension == known; });
}

// A relative include ("../ui/mainwindow.h") as the path from src/ ("ui/mainwindow.h"); any other include unchanged
std::string fromSourceFolder(const std::filesystem::path& including, const std::string& target)
{
    if (!target.starts_with("./") && !target.starts_with("../")) {
        return target;
    }
    const std::filesystem::path resolved = (including.parent_path() / target).lexically_normal();
    return resolved.lexically_relative(std::filesystem::path(CHRONO_CALC_SOURCE_DIR).lexically_normal()).generic_string();
}

// Every #include in the files of one layer folder (src/core, third_party, ...), with file and line
std::vector<Include> includesOf(const std::filesystem::path& folder, int& filesFound)
{
    std::vector<Include> found;
    if (!std::filesystem::exists(folder)) {
        return found;
    }
    for (const auto& entry : std::filesystem::recursive_directory_iterator(folder)) {
        if (!entry.is_regular_file() || !isSourceFile(entry.path())) {
            continue;
        }
        ++filesFound;
        std::ifstream stream(entry.path());
        std::string text;
        int number = 0;
        while (std::getline(stream, text)) {
            ++number;
            const std::string target = includeTarget(text);
            if (!target.empty()) {
                found.push_back({entry.path().string(), number, fromSourceFolder(entry.path(), target)});
            }
        }
    }
    return found;
}

// The includes of a layer that name a forbidden target, as "file:line includes target" lines
QStringList forbiddenIncludes(const std::filesystem::path& folder, const std::regex& forbidden, int& filesFound)
{
    QStringList faults;
    for (const Include& include : includesOf(folder, filesFound)) {
        if (std::regex_search(include.target, forbidden)) {
            faults << QString::fromStdString(include.file + ":" + std::to_string(include.line) + " includes " + include.target);
        }
    }
    return faults;
}

}  // namespace

class TestLayers : public QObject
{
    Q_OBJECT

private slots:
    static void coreIncludesNoQtNoUiAndNoApp()
    {
        int filesFound = 0;
        const QStringList faults = forbiddenIncludes(std::filesystem::path(CHRONO_CALC_SOURCE_DIR) / "core", std::regex(R"(^(Q|q[a-z0-9_]*\.h$|ui/|app/))"), filesFound);
        QVERIFY2(filesFound > 0, "no files found in src/core: the check would prove nothing");
        QVERIFY2(faults.isEmpty(), qPrintable(faults.join(QStringLiteral("\n"))));
    }

    static void uiIncludesNoApp()
    {
        int filesFound = 0;
        const QStringList faults = forbiddenIncludes(std::filesystem::path(CHRONO_CALC_SOURCE_DIR) / "ui", std::regex(R"(^app/)"), filesFound);
        QVERIFY2(filesFound > 0, "no files found in src/ui: the check would prove nothing");
        QVERIFY2(faults.isEmpty(), qPrintable(faults.join(QStringLiteral("\n"))));
    }

    static void thirdPartyIncludesNoQtNoCoreNoUiAndNoApp()
    {
        int filesFound = 0;
        const QStringList faults
            = forbiddenIncludes(std::filesystem::path(CHRONO_CALC_THIRD_PARTY_DIR), std::regex(R"(^(Q|q[a-z0-9_]*\.h$|core/|ui/|app/))"), filesFound);
        QVERIFY2(filesFound > 0, "no files found in third_party: the check would prove nothing");
        QVERIFY2(faults.isEmpty(), qPrintable(faults.join(QStringLiteral("\n"))));
    }
};

QTEST_GUILESS_MAIN(TestLayers)
#include "test_layers.moc"
