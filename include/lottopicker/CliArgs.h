#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace lottopicker {

// Fixed usage string (docs/SDD.md Interfaces & File Formats -> "CLI
// invocation"), shared by CliUsageError's message so the two never drift
// apart.
extern const char *const kUsageMessage;

// Parsed, validated command-line invocation:
//   lottopicker <config-path> [--backtest <date1>[,<date2>,...]]
//
// UI-001 fixed the required positional <config-path>. UI-003 (this
// struct's `backtestDates`) adds the optional --backtest flag: parsing
// only, per that issue's scope -- wiring the parsed dates to CORE-205's
// backtest engine happens in a later issue.
struct CliArgs {
    std::filesystem::path configPath;

    // Empty means no --backtest flag was given (normal ranking run).
    // Non-empty holds one or more ISO-8601 (YYYY-MM-DD) date strings, in
    // the order listed on the command line, one per requested backtest
    // sample date (docs/SDD.md Interfaces & File Formats). Kept as plain
    // strings rather than a calendar type for the same reason as
    // DrawRecord::date -- no stage needs calendar arithmetic on them, and
    // era/date comparisons elsewhere already compare zero-padded
    // ISO-8601 strings lexicographically.
    std::vector<std::string> backtestDates;

    // True if --backtest was given (backtestDates is non-empty); the
    // caller should run the backtest path instead of the normal ranking
    // path. A named accessor rather than a bare `!backtestDates.empty()`
    // check at every call site, so the mode decision reads as intent.
    bool isBacktest() const { return !backtestDates.empty(); }

    // Parses argv (excluding argv[0], i.e. the program name) into a
    // validated CliArgs. Throws CliUsageError if the argument count/shape
    // is wrong (including an unrecognized flag, or a malformed/empty
    // --backtest date list), or CliConfigNotFoundError if <config-path>
    // does not name an existing regular file. Never returns a
    // partially-valid result.
    static CliArgs parse(const std::vector<std::string> &args);
};

} // namespace lottopicker
