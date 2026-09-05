#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <vector>

#include "lottopicker/BacktestEngine.h"
#include "lottopicker/CliArgs.h"
#include "lottopicker/Config.h"
#include "lottopicker/CsvIngestor.h"
#include "lottopicker/EraTagger.h"
#include "lottopicker/Errors.h"
#include "lottopicker/ModelStore.h"
#include "lottopicker/RankedListPresenter.h"
#include "lottopicker/RankingEngine.h"
#include "lottopicker/Version.h"

// Host/entry point only — no domain logic here (see lottopicker_lib in
// src/CMakeLists.txt). Parses/validates CLI args via CliArgs::parse
// (UI-001, UI-003), then the config file via Config::parse (UI-002),
// then Stage 1 of docs/SDD.md's pipeline via ModelStore::loadOrBuild
// (CORE-204: reuse the persisted model artifact if its source hash
// still matches data_file, otherwise ingest/era-tag/score/normalize/
// persist a fresh one) -- converting any LottoPickerError into the
// documented stderr-message + non-zero-exit-code contract (docs/SDD.md
// Coding Standards -> Error handling). Stage 2 branches on
// CliArgs::isBacktest: the normal ranking path scores the persisted
// model (CORE-202/203), while the backtest path (CORE-205) re-ingests/
// re-era-tags the same data_file into row-level DrawRecords (something
// the persisted ModelArtifact deliberately never retains -- it's an
// aggregate, not per-draw data) and runs BacktestEngine::run once per
// requested sample date. Console formatting on both paths is
// intentionally minimal -- OUT-400/OUT-401's fully human-readable
// presentation, and DATA-OUT-302's own report-row structure, are
// separate, not-yet-implemented RTVM items.
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
            // Backtest path (UI-003 -> CORE-205): the persisted
            // ModelArtifact above is an aggregate (per-number/per-group
            // scores only), not row-level draws, so BacktestEngine
            // needs its own ingest + era-tag pass over data_file to get
            // the DrawRecords it truncates per sample date. This is a
            // second read of the same file the model-build step above
            // may have just read (or reused a persisted model for) --
            // accepted here since a backtest run is not the hot path
            // CORE-204's reuse-vs-rebuild caching targets.
            std::cout << "mode: backtest (" << cliArgs.backtestDates.size() << " sample date(s))\n";
            const lottopicker::IngestResult backtestIngest =
                lottopicker::CsvIngestor::ingest(config.dataFile);
            std::vector<lottopicker::DrawRecord> history = backtestIngest.records;
            lottopicker::EraTagger::tag(history);

            for (const std::string &date : cliArgs.backtestDates) {
                try {
                    const lottopicker::BacktestResult result =
                        lottopicker::BacktestEngine::run(history, date, config.topN);
                    std::cout << "  " << date << ": ";
                    if (result.found) {
                        std::cout << "rank " << result.rank << "/" << result.topN << " ("
                                  << result.percentile << " percentile)";
                    } else {
                        std::cout << "not found in top-N";
                    }
                    std::cout << " | containment(3/4/5/6 of 6):";
                    for (int count : result.observedContainment) {
                        std::cout << " " << count;
                    }
                    std::cout << "\n";
                } catch (const lottopicker::BacktestError &e) {
                    // One bad sample date (e.g. no draw recorded on it)
                    // does not abort the rest of the requested dates
                    // (UI-003's list is otherwise independent per date).
                    std::cout << "  " << date << ": " << e.what() << "\n";
                }
            }
        } else {
            // Normal ranking path (CORE-203): score the full combination
            // space against the model just loaded/built above, retaining
            // only config.topN via RankingEngine's fixed-size heap, then
            // render it as OUT-400's human-readable console table.
            std::cout << "mode: rank\n";
            const std::vector<lottopicker::RankedCombo> ranked =
                lottopicker::RankingEngine::rank(modelResult.artifact, config.topN);
            lottopicker::RankedListPresenter::print(std::cout, ranked);
        }
        return EXIT_SUCCESS;
    } catch (const lottopicker::LottoPickerError &e) {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }
}
