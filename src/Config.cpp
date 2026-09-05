#include "lottopicker/Config.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <optional>
#include <string>

#include "lottopicker/Errors.h"

namespace lottopicker {

namespace {

constexpr const char *kDataFileKey = "data_file";
constexpr const char *kTopNKey = "top_n";

std::string trim(const std::string &s) {
    const auto isSpace = [](unsigned char c) { return std::isspace(c) != 0; };
    auto begin = std::find_if_not(s.begin(), s.end(), isSpace);
    auto end = std::find_if_not(s.rbegin(), s.rend(), isSpace).base();
    if (begin >= end) {
        return "";
    }
    return std::string(begin, end);
}

// Parses a strict positive-integer token (digits only, no sign, no
// whitespace/trailing garbage) -- mirrors CsvIngestor's parseStrictInt
// but only ever needs to distinguish "valid positive integer" from
// "not one", per UI-002's validation wording.
std::optional<int> parseStrictPositiveInt(const std::string &token) {
    if (token.empty()) {
        return std::nullopt;
    }
    long value = 0;
    for (char ch : token) {
        if (std::isdigit(static_cast<unsigned char>(ch)) == 0) {
            return std::nullopt;
        }
        value = value * 10 + (ch - '0');
        if (value > 1'000'000'000) {
            // Well past any plausible top-N; stop before overflow.
            return std::nullopt;
        }
    }
    return static_cast<int>(value);
}

} // namespace

Config Config::parse(const std::filesystem::path &configPath) {
    std::ifstream file(configPath);
    if (!file.is_open()) {
        throw ConfigFormatError("cannot open config file: " + configPath.string());
    }

    std::map<std::string, std::string> values;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        const std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed.front() == '#') {
            continue;
        }

        const std::size_t eq = trimmed.find('=');
        if (eq == std::string::npos) {
            // Not a recognizable `key=value` line; ignored rather than
            // fatal, consistent with UI-002's "unrecognized keys are
            // ignored, not an error" -- only a missing/invalid required
            // key is a validation error.
            continue;
        }

        const std::string key = trim(trimmed.substr(0, eq));
        const std::string value = trim(trimmed.substr(eq + 1));
        if (!key.empty()) {
            values[key] = value;
        }
    }

    const auto dataFileIt = values.find(kDataFileKey);
    if (dataFileIt == values.end()) {
        throw ConfigValidationError("missing required config key: " + std::string(kDataFileKey));
    }

    const auto topNIt = values.find(kTopNKey);
    if (topNIt == values.end()) {
        throw ConfigValidationError("missing required config key: " + std::string(kTopNKey));
    }

    const std::optional<int> topN = parseStrictPositiveInt(topNIt->second);
    if (!topN.has_value() || *topN < kMinTopN) {
        throw ConfigValidationError("config key '" + std::string(kTopNKey) +
                                    "' must be a positive integer, found '" + topNIt->second + "'");
    }

    Config config;
    // Relative to the config file's own directory, not the process's
    // working directory, per docs/SDD.md. std::filesystem::path's
    // operator/ already discards the left operand if the right one is
    // absolute, so an absolute `data_file` value works unchanged.
    config.dataFile = configPath.parent_path() / dataFileIt->second;
    config.topN = *topN;
    return config;
}

} // namespace lottopicker
