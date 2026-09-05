#include <cstdlib>
#include <iostream>
#include <vector>

#include "lottopicker/CliArgs.h"
#include "lottopicker/Errors.h"
#include "lottopicker/Version.h"

// Host/entry point only — no domain logic here (see lottopicker_lib in
// src/CMakeLists.txt). Parses/validates CLI args via CliArgs::parse
// (UI-001) and converts any LottoPickerError into the documented
// stderr-message + non-zero-exit-code contract (docs/SDD.md Coding
// Standards -> Error handling). Config parsing (UI-002), the --backtest
// flag (UI-003), and the rest of the ranking/backtest pipeline land
// incrementally in later RTVM feature issues.
int main(int argc, char **argv) {
    const std::vector<std::string> args(argv + 1, argv + argc);

    try {
        const lottopicker::CliArgs cliArgs = lottopicker::CliArgs::parse(args);

        // UI-002 (config parsing) and the ranking/backtest pipeline are not
        // yet implemented; UI-001 only requires that a valid config path
        // proceed without error.
        std::cout << "lottopicker " << lottopicker::kVersion << "\n";
        std::cout << "config: " << cliArgs.configPath.string() << "\n";
        return EXIT_SUCCESS;
    } catch (const lottopicker::LottoPickerError &e) {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }
}
