#include "lottopicker/CsvIngestor.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <optional>
#include <set>
#include <sstream>

namespace lottopicker {

namespace {

constexpr const char *kExpectedHeader = "date,n1,n2,n3,n4,n5,n6";

// Strips a trailing '\r' so files with CRLF line endings (common for a
// client-supplied CSV originating on Windows) parse identically to LF-only
// files.
std::string stripTrailingCr(std::string line) {
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
    return line;
}

std::vector<std::string> splitOnComma(const std::string &line) {
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string field;
    while (std::getline(ss, field, ',')) {
        fields.push_back(field);
    }
    // A trailing comma would otherwise silently drop the final empty field.
    if (!line.empty() && line.back() == ',') {
        fields.emplace_back();
    }
    return fields;
}

bool isBlank(const std::string &line) {
    return std::all_of(line.begin(), line.end(),
                       [](unsigned char c) { return std::isspace(c) != 0; });
}

// Validates `YYYY-MM-DD` shape without pulling in <chrono>'s calendar
// machinery (DrawRecord.h explains why the date is kept as a plain
// string) — checks digit positions/dashes and coarse month/day ranges,
// not full calendar validity (e.g. doesn't reject 2021-02-30).
bool looksLikeIsoDate(const std::string &s) {
    if (s.size() != 10 || s[4] != '-' || s[7] != '-') {
        return false;
    }
    for (std::size_t i : {0, 1, 2, 3, 5, 6, 8, 9}) {
        if (std::isdigit(static_cast<unsigned char>(s[i])) == 0) {
            return false;
        }
    }
    int month = (s[5] - '0') * 10 + (s[6] - '0');
    int day = (s[8] - '0') * 10 + (s[9] - '0');
    return month >= 1 && month <= 12 && day >= 1 && day <= 31;
}

// Parses a strict integer (optional leading '-', digits only, no
// whitespace/trailing garbage). Returns std::nullopt if `token` isn't
// one, rather than relying on std::stoi's partial-parse behavior.
std::optional<int> parseStrictInt(const std::string &token) {
    if (token.empty()) {
        return std::nullopt;
    }
    std::size_t i = 0;
    bool negative = false;
    if (token[i] == '-') {
        negative = true;
        ++i;
    }
    if (i == token.size()) {
        return std::nullopt;
    }
    long value = 0;
    for (; i < token.size(); ++i) {
        if (std::isdigit(static_cast<unsigned char>(token[i])) == 0) {
            return std::nullopt;
        }
        value = value * 10 + (token[i] - '0');
        if (value > 1'000'000) {
            // Well past any plausible pool bound; stop before overflow.
            return std::nullopt;
        }
    }
    return static_cast<int>(negative ? -value : value);
}

} // namespace

IngestResult CsvIngestor::ingest(const std::filesystem::path &csvPath, int poolMin, int poolMax) {
    std::ifstream file(csvPath);
    if (!file.is_open()) {
        throw CsvFormatError("cannot open historical draw CSV: " + csvPath.string());
    }

    std::string headerLine;
    if (!std::getline(file, headerLine)) {
        throw CsvFormatError("historical draw CSV is empty: " + csvPath.string());
    }
    headerLine = stripTrailingCr(headerLine);
    if (headerLine != kExpectedHeader) {
        throw CsvFormatError("historical draw CSV has an unexpected header (expected `" +
                             std::string(kExpectedHeader) + "`, found `" + headerLine +
                             "`): " + csvPath.string());
    }

    IngestResult result;
    std::string line;
    std::size_t row = 1; // The header itself is row 1.
    while (std::getline(file, line)) {
        ++row;
        line = stripTrailingCr(line);
        if (isBlank(line)) {
            continue;
        }

        std::vector<std::string> fields = splitOnComma(line);
        if (fields.empty()) {
            result.errors.push_back({row, "empty row"});
            continue;
        }

        const std::string &date = fields.front();
        const std::size_t numberCount = fields.size() - 1;
        if (numberCount != static_cast<std::size_t>(kNumbersPerDraw)) {
            result.errors.push_back({row, "expected " + std::to_string(kNumbersPerDraw) +
                                              " numbers, found " + std::to_string(numberCount)});
            continue;
        }

        if (!looksLikeIsoDate(date)) {
            result.errors.push_back({row, "invalid date '" + date + "', expected YYYY-MM-DD"});
            continue;
        }

        std::array<int, kNumbersPerDraw> numbers{};
        std::set<int> seen;
        bool rowOk = true;
        for (int i = 0; i < kNumbersPerDraw && rowOk; ++i) {
            const std::string &token = fields[static_cast<std::size_t>(i) + 1];
            std::optional<int> parsed = parseStrictInt(token);
            if (!parsed.has_value()) {
                result.errors.push_back({row, "invalid number '" + token + "'"});
                rowOk = false;
                break;
            }
            const int value = *parsed;
            if (value < poolMin || value > poolMax) {
                result.errors.push_back(
                    {row, "number " + std::to_string(value) + " outside valid pool range (" +
                              std::to_string(poolMin) + "-" + std::to_string(poolMax) + ")"});
                rowOk = false;
                break;
            }
            if (!seen.insert(value).second) {
                result.errors.push_back(
                    {row, "duplicate number " + std::to_string(value) + " within row"});
                rowOk = false;
                break;
            }
            numbers[static_cast<std::size_t>(i)] = value;
        }
        if (!rowOk) {
            continue;
        }

        std::sort(numbers.begin(), numbers.end());
        result.records.push_back(DrawRecord{date, numbers});
    }

    return result;
}

} // namespace lottopicker
