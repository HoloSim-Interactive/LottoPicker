#include "lottopicker/CliArgs.h"

#include <system_error>

#include "lottopicker/Errors.h"

namespace lottopicker {

const char *const kUsageMessage =
    "usage: lottopicker <config-path> [--backtest <date1>[,<date2>,...]]";

CliArgs CliArgs::parse(const std::vector<std::string> &args) {
    // UI-001 only: exactly one positional argument, the config path.
    // --backtest is added here by UI-003, not before.
    if (args.size() != 1) {
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
    return result;
}

} // namespace lottopicker
