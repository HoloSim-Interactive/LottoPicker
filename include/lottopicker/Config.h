#pragma once

#include <filesystem>

namespace lottopicker {

// Positive-integer requirement on `top_n` is fixed by docs/RTVM.md's
// UI-002 wording ("top-N not a positive integer"); no upper bound is
// documented, so none is enforced here.
inline constexpr int kMinTopN = 1;

// Parsed, validated config file contents (UI-002): at minimum, the path
// to the historical draw CSV and the retained top-N envelope size.
// docs/SDD.md's Interfaces & File Formats fixes the exact file syntax:
// flat `key=value` text, one per line, `#` starts a comment, blank
// lines ignored.
struct Config {
    // Path to the historical draw CSV, resolved relative to the config
    // file's own directory (not the process's working directory) so a
    // config stays portable alongside its data.
    std::filesystem::path dataFile;

    // Positive integer: the top-N envelope size retained by CORE-203.
    int topN = 0;

    // Parses `configPath`'s contents into a validated Config.
    //
    // Throws ConfigFormatError if the file can't be opened/read.
    // Throws ConfigValidationError, naming the offending key, if
    // `data_file` or `top_n` is missing, or `top_n` is present but not
    // a positive integer. Unrecognized keys are ignored (forward-
    // compatible), not a validation error. Never returns a
    // partially-valid result.
    static Config parse(const std::filesystem::path &configPath);
};

} // namespace lottopicker
