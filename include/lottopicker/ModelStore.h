#pragma once

#include <filesystem>
#include <vector>

#include "lottopicker/CsvIngestor.h" // RowError, kPoolMin/kCurrentPoolMax
#include "lottopicker/DecayScorer.h" // kDefaultHalfLifeDraws
#include "lottopicker/ModelArtifact.h"

namespace lottopicker {

// CORE-204: decides, on each run, whether a previously persisted model
// artifact (DATA-OUT-301) can be reused as-is or must be rebuilt from
// the source CSV -- docs/SDD.md's Top-level control flow diagram's
// "HashCheck" step. The decision is a single sha256 comparison: a
// persisted artifact is reusable iff its recorded `sourceHash` matches
// the *current* bytes of `dataFile`, so any edit to the CSV (even one
// appended row) invalidates it and forces a rebuild -- deliberately
// coarser than a byte-diff/row-count check (docs/RTVM.md's TP-CORE-204:
// any change regenerates, no partial-update path is required).
class ModelStore {
public:
    // sha256 hex digest of `dataFile`'s bytes -- what a persisted
    // artifact's `sourceHash` is compared against.
    //
    // Throws ModelStoreError if `dataFile` can't be opened/read.
    static std::string computeSourceHash(const std::filesystem::path &dataFile);

    // Derives the on-disk path CORE-204 persists/looks up the model
    // artifact at, from `dataFile`: `dataFile` with `.model` appended
    // (e.g. `history.csv` -> `history.csv.model`), colocated in the
    // same directory. NOT fixed by docs/SDD.md or docs/RTVM.md (only
    // the artifact's *contents* format is, DATA-OUT-301) -- this is
    // Software Engineer's convention, cheap to change in this one spot
    // if a different one (e.g. a config key) is wanted later.
    static std::filesystem::path defaultModelPath(const std::filesystem::path &dataFile);

    // Result of loadOrBuild(): the ready-to-use model, whether this
    // call rebuilt it (the "reused vs. recomputed" distinction
    // TP-CORE-204 part 1 checks the tool's output for), and any
    // per-row ingestion problems encountered during a rebuild (always
    // empty when the model was reused).
    struct LoadOrBuildResult {
        ModelArtifact artifact;
        bool wasRebuilt = false;
        std::vector<RowError> ingestErrors;
    };

    // The full CORE-204 decision + orchestration: compute `dataFile`'s
    // current hash; if `modelPath` holds a valid, matching-hash
    // artifact, load and return it (wasRebuilt = false). Otherwise
    // rebuild from scratch -- ingest (DATA-IN-100), era-tag
    // (DATA-IN-101), score (CORE-200/CORE-201), normalize (CORE-206) --
    // persist the result to `modelPath`, and return it
    // (wasRebuilt = true).
    //
    // `poolMin`/`poolMax`/`halfLifeDraws` are forwarded to the
    // ingestion/scoring stages unchanged (defaults match their own
    // documented defaults) -- exposed here so a rebuild uses exactly
    // the parameters the caller intends, not a second hardcoded copy.
    //
    // Throws ModelStoreError if `dataFile` can't be hashed, or
    // CsvFormatError if `dataFile` fails CsvIngestor's whole-file
    // structural checks during a rebuild -- both are user-facing
    // config problems, caught at main()'s boundary like every other
    // LottoPickerError.
    static LoadOrBuildResult loadOrBuild(const std::filesystem::path &dataFile,
                                         const std::filesystem::path &modelPath,
                                         int poolMin = kPoolMin, int poolMax = kCurrentPoolMax,
                                         int halfLifeDraws = kDefaultHalfLifeDraws);
};

} // namespace lottopicker
