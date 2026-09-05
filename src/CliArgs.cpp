#include "lottopicker/CliArgs.h"

#include <cctype>
#include <sstream>
#include <system_error>

#include "lottopicker/Errors.h"

namespace lottopicker {

namespace {

// Validates `YYYY-MM-DD` shape only (digit positions/dashes, coarse
// month/day ranges) -- deliberately the same coarse check
// `CsvIngestor.cpp`'s `looksLikeIsoDate` applies to draw dates, kept as
// its own copy here rather than a shared header: the two call sites
// belong to different features (UI-003's CLI parsing vs. DATA-IN-100's
// CSV ingestion) and this is a few lines, not logic worth coupling two
// otherwise-independent modules over.
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

// Splits a comma-separated `--backtest` value into its individual date
// tokens. An empty `value` (e.g. `--backtest` given with a blank
// argument) yields one empty token, which fails looksLikeIsoDate and so
// is reported the same way as any other malformed date -- no special
// casing needed.
std::vector<std::string> splitOnComma(const std::string &value) {
    std::vector<std::string> tokens;
    std::stringstream ss(value);
    std::string token;
    while (std::getline(ss, token, ',')) {
        tokens.push_back(token);
    }
    // A trailing comma would otherwise silently drop the final empty
    // token instead of being reported as a malformed (empty) date.
    if (!value.empty() && value.back() == ',') {
        tokens.emplace_back();
    }
    if (tokens.empty()) {
        tokens.emplace_back();
    }
    return tokens;
}

} // namespace

const char *const kUsageMessage =
    "usage: lottopicker <config-path> [--backtest <date1>[,<date2>,...]]";

CliArgs CliArgs::parse(const std::vector<std::string> &args) {
    // Exactly one positional argument (UI-001), optionally followed by
    // `--backtest <dates>` (UI-003): a 3-argument shape only, since the
    // flag and its value are always two separate argv tokens (never
    // `--backtest=<dates>`) per docs/SDD.md's fixed invocation syntax.
    if (args.size() != 1 && args.size() != 3) {
        throw CliUsageError(kUsageMessage);
    }

    std::filesystem::path configPath(args.front());

    // is_regular_file's error_code overload never throws (no exception on
    // a permissions/race failure); treat any such failure as "not found"
    // rather than crashing the process on an OS-level edge case.
    std::error_code ec;
    const bool exists = std::filesystem::is_regular_file(configPath, ec);
    if (ec || !exists) {
        throw CliConfigNotFoundError("config file not found: " + configPath.string());
    }

    CliArgs result;
    result.configPath = std::move(configPath);

    if (args.size() == 3) {
        if (args[1] != "--backtest") {
            throw CliUsageError(kUsageMessage);
        }

        std::vector<std::string> dates = splitOnComma(args[2]);
        for (const std::string &date : dates) {
            if (!looksLikeIsoDate(date)) {
                throw CliUsageError("invalid --backtest date '" + date +
                                    "', expected YYYY-MM-DD (ISO-8601)");
            }
        }
        result.backtestDates = std::move(dates);
    }

    return result;
}

} // namespace lottopicker
