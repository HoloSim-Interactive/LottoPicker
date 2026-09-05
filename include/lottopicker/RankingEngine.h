#pragma once

#include <array>
#include <vector>

#include "lottopicker/CompositeScorer.h" // CompositeWeights
#include "lottopicker/CsvIngestor.h"     // kPoolMin/kCurrentPoolMax
#include "lottopicker/DrawRecord.h"      // kNumbersPerDraw
#include "lottopicker/ModelArtifact.h"

namespace lottopicker {

// DATA-OUT-300: one retained ranking result -- a full 6-number
// combination (sorted ascending, same invariant as DrawRecord::numbers)
// and its composite score. `rank` is filled in by RankingEngine::rank()
// itself (1-based, per the retained list's position) rather than left
// for the caller to derive from vector index, so a caller printing
// OUT-400's console table doesn't need to re-derive it.
struct RankedCombo {
    int rank = 0;
    std::array<int, kNumbersPerDraw> combo{};
    double score = 0.0;
};

// CORE-203: scores the full combinatorial space (`C(poolMax-poolMin+1,
// kNumbersPerDraw)` combinations -- 22,957,480 for the documented
// poolMin=1/poolMax=53) against CORE-202's composite formula, retaining
// only the top `topN` by score, without ever materializing the full
// ranked list. Per docs/SDD.md's Algorithm design:
//
//   - Combinations are generated via nested index loops over strictly
//     increasing numbers (never via next_permutation or any other
//     technique requiring a materialized list), so each combination is
//     produced -- and scored -- exactly once: no duplicates possible by
//     construction.
//   - A fixed-size (`topN`) min-heap keyed by score is the only
//     structure retaining scored combinations at any point in time:
//     once it holds `topN` entries, a newly scored combination is kept
//     only if it beats the heap's current minimum, which is then
//     evicted. This bounds memory at O(topN) regardless of how large
//     the combination space is, and bounds time at
//     O(C(n,6) log topN) -- exactly the complexity class CORE-203's
//     RTVM entry commits to.
class RankingEngine {
public:
    // Runs the full CORE-203 scan and returns exactly `topN` results
    // (fewer only if the combination space itself has fewer than
    // `topN` members, which cannot happen for any documented pool size)
    // in `rank` order: index 0 is rank 1 (the highest score), strictly
    // descending thereafter. Ties are broken by ascending numeric combo
    // order (lexicographic on `combo`) so output is fully deterministic
    // run-to-run for the same model and weights -- not an RTVM
    // requirement, but needed for TP-CORE-203's "no duplicates"/
    // reproducibility check to be meaningfully verifiable at all.
    //
    // `topN` must be a positive integer (UI-002 already guarantees this
    // for the config-supplied value; not re-validated here since this
    // is internal algorithmic code, not a user-facing entry point).
    static std::vector<RankedCombo> rank(const ModelArtifact &model, int topN,
                                         const CompositeWeights &weights = CompositeWeights{},
                                         int poolMin = kPoolMin, int poolMax = kCurrentPoolMax);
};

} // namespace lottopicker
