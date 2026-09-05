#include <cstdlib>
#include <iostream>
#include <vector>

#include "lottopicker/CliArgs.h"
#include "lottopicker/Config.h"
#include "lottopicker/Errors.h"
#include "lottopicker/Version.h"

// Host/entry point only — no domain logic here (see lottopicker_lib in
// src/CMakeLists.txt). Parses/validates CLI args via CliArgs::parse
// (UI-001), then the config file via Config::parse (UI-002), and
// converts any LottoPickerError into the documented stderr-message +
// non-zero-exit-code contract (docs/SDD.md Coding Standards -> Error
// handling). The --backtest flag (UI-003) and the rest of the
// ranking/backtest pipeline land incrementally in later RTVM feature
// issues.
int main(int argc, char **argv) {
    const std::vector<std::string> args(argv + 1, argv + argc);

    try {
        const lottopicker::CliArgs cliArgs = lottopicker::CliArgs::parse(args);
        const lottopicker::Config config = lottopicker::Config::parse(cliArgs.configPath);

        // The ranking/backtest pipeline is not yet implemented; UI-002
        // only requires that a valid config parse into its two fields
        // without error.
        std::cout << "lottopicker " << lottopicker::kVersion << "\n";
        std::cout << "config: " << cliArgs.configPath.string() << "\n";
        std::cout << "data_file: " << config.dataFile.string() << "\n";
        std::cout << "top_n: " << config.topN << "\n";
        return EXIT_SUCCESS;
    } catch (const lottopicker::LottoPickerError &e) {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }
}
