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
// UI-001 fixes only the required positional <config-path>. The optional
// --backtest flag is UI-003's scope and will extend CliArgs::parse when
// that feature lands, rather than being designed speculatively here.
struct CliArgs {
    std::filesystem::path configPath;

    // Parses argv (excluding argv[0], i.e. the program name) into a
    // validated CliArgs. Throws CliUsageError if the argument count/shape
    // is wrong, or CliConfigNotFoundError if <config-path> does not name
    // an existing regular file. Never returns a partially-valid result.
    static CliArgs parse(const std::vector<std::string> &args);
};

} // namespace lottopicker
