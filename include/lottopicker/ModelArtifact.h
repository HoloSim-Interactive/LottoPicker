#pragma once

#include <cstddef>
#include <map>
#include <string>

#include "lottopicker/CooccurrenceScorer.h" // GroupKey

namespace lottopicker {

// One group size's normalized co-occurrence scores, sparse (only
// groups actually observed historically -- see CooccurrenceScorer.h's
// GroupScoreMap; absence of a key means "never observed", not 0.0 by
// storage, though CORE-206's normCooc() does still return a
// well-defined value for it).
using ModelGroupScoreMap = std::map<GroupKey, double>;

// DATA-OUT-301: the persisted statistical model CORE-204 builds once
// and reuses across ranking runs -- CORE-200/CORE-201's per-number
// decay and per-group co-occurrence scores, already run through
// CORE-206's pool-size normalization (this is the *final*,
// ranking-ready form CORE-202's composite scorer consumes; the raw,
// pre-normalization intermediates are not persisted, since nothing
// downstream of model-build needs them), plus metadata sufficient to
// detect staleness against the source CSV.
struct ModelArtifact {
    // sha256 hex digest of `data_file`'s bytes at build time
    // (ModelStore::computeSourceHash) -- compared against the current
    // data_file's hash on every run to decide reuse vs. rebuild
    // (CORE-204).
    std::string sourceHash;

    // Earliest/latest DrawRecord::date (YYYY-MM-DD) across the history
    // this model was built from. Empty strings if built from zero
    // records.
    std::string earliestDate;
    std::string latestDate;

    // Number of DrawRecord rows the model was built from (successfully
    // ingested rows only -- DATA-IN-100's per-row error tolerance means
    // this can be less than the CSV's physical row count).
    std::size_t drawCount = 0;

    // norm_decay(number) for every pool number the model was built for
    // (docs/SDD.md's documented `[per_number]` section).
    std::map<int, double> perNumber;

    // norm_cooc(group), sparse, indexed by group size (2..6) --
    // docs/SDD.md's documented `[group_scores:N]` sections.
    std::map<int, ModelGroupScoreMap> groupScores;

    bool operator==(const ModelArtifact &other) const {
        return sourceHash == other.sourceHash && earliestDate == other.earliestDate &&
               latestDate == other.latestDate && drawCount == other.drawCount &&
               perNumber == other.perNumber && groupScores == other.groupScores;
    }
};

} // namespace lottopicker
