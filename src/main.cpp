#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <vector>

#include "lottopicker/CliArgs.h"
#include "lottopicker/Config.h"
#include "lottopicker/Errors.h"
#include "lottopicker/ModelStore.h"
#include "lottopicker/Version.h"

// Host/entry point only — no domain logic here (see lottopicker_lib in
// src/CMakeLists.txt). Parses/validates CLI args via CliArgs::parse
// (UI-001, UI-003), then the config file via Config::parse (UI-002),
// then Stage 1 of docs/SDD.md's pipeline via ModelStore::loadOrBuild
// (CORE-204: reuse the persisted model artifact if its source hash
// still matches data_file, otherwise ingest/era-tag/score/normalize/
// persist a fresh one) -- converting any LottoPickerError into the
// documented stderr-message + non-zero-exit-code contract (docs/SDD.md
// Coding Standards -> Error handling). UI-003 is flag-parsing scope
// only for the mode branch below: CliArgs::isBacktest selects which
// message prints, but the actual CORE-205 backtest engine and CORE-202/
// 203's ranking pipeline (Stage 2) land in later RTVM feature issues --
// this only wires Stage 1 (the model) all the way through.
int main(int argc, char **argv) {
    const std::vector<std::string> args(argv + 1, argv + argc);

    try {
        const lottopicker::CliArgs cliArgs = lottopicker::CliArgs::parse(args);
        const lottopicker::Config config = lottopicker::Config::parse(cliArgs.configPath);

        std::cout << "lottopicker " << lottopicker::kVersion << "\n";
        std::cout << "config: " << cliArgs.configPath.string() << "\n";
        std::cout << "data_file: " << config.dataFile.string() << "\n";
        std::cout << "top_n: " << config.topN << "\n";

        const std::filesystem::path modelPath =
            lottopicker::ModelStore::defaultModelPath(config.dataFile);
        const lottopicker::ModelStore::LoadOrBuildResult modelResult =
            lottopicker::ModelStore::loadOrBuild(config.dataFile, modelPath);

        if (modelResult.wasRebuilt) {
            std::cout << "model: rebuilt (" << modelResult.artifact.drawCount << " draw(s)) -> "
                      << modelPath.string() << "\n";
            for (const lottopicker::RowError &rowError : modelResult.ingestErrors) {
                std::cout << "  row " << rowError.row << ": " << rowError.message << "\n";
            }
        } else {
            std::cout << "model: reused (source unchanged) <- " << modelPath.string() << "\n";
        }

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
