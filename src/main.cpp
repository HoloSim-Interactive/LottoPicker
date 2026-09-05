#include <cstdlib>
#include <iostream>
#include <vector>

#include "lottopicker/CliArgs.h"
#include "lottopicker/Config.h"
#include "lottopicker/Errors.h"
#include "lottopicker/Version.h"

// Host/entry point only — no domain logic here (see lottopicker_lib in
// src/CMakeLists.txt). Parses/validates CLI args via CliArgs::parse
// (UI-001, UI-003), then the config file via Config::parse (UI-002),
// and converts any LottoPickerError into the documented stderr-message
// + non-zero-exit-code contract (docs/SDD.md Coding Standards -> Error
// handling). UI-003 is flag-parsing scope only here: CliArgs::isBacktest
// selects which message prints below, but the actual CORE-205 backtest
// engine (and the rest of the ranking pipeline) lands incrementally in
// later RTVM feature issues -- ingestion isn't even wired into main()
// yet, so neither path can do real work today.
int main(int argc, char **argv) {
    const std::vector<std::string> args(argv + 1, argv + argc);

    try {
        const lottopicker::CliArgs cliArgs = lottopicker::CliArgs::parse(args);
        const lottopicker::Config config = lottopicker::Config::parse(cliArgs.configPath);

        std::cout << "lottopicker " << lottopicker::kVersion << "\n";
        std::cout << "config: " << cliArgs.configPath.string() << "\n";
        std::cout << "data_file: " << config.dataFile.string() << "\n";
        std::cout << "top_n: " << config.topN << "\n";

        if (cliArgs.isBacktest()) {
            // Backtest path selected (UI-003): one placeholder line per
            // requested sample date, standing in for DATA-OUT-302's
            // report row until CORE-205 is wired in.
            std::cout << "mode: backtest (" << cliArgs.backtestDates.size() << " sample date(s))\n";
            for (const std::string &date : cliArgs.backtestDates) {
                std::cout << "  " << date << ": backtest engine not yet implemented (CORE-205)\n";
            }
        } else {
            // Normal ranking path; the ranking pipeline itself is not yet
            // implemented (CORE-202/203).
            std::cout << "mode: rank\n";
        }
        return EXIT_SUCCESS;
    } catch (const lottopicker::LottoPickerError &e) {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }
}
