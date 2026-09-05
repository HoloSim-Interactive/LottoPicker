#include "lottopicker/RankingEngine.h"

#include <algorithm>
#include <cstddef>
#include <queue>
#include <vector>

namespace lottopicker {

namespace {

// RankingEngine's nested-loop combination generator below is written
// out explicitly for a 6-number draw (docs/SDD.md's Algorithm design:
// "nested index loops"). kNumbersPerDraw is a fixed domain constant
// (Florida Lotto draws six numbers, not a config value -- see
// DrawRecord.h), so this isn't expected to change, but the assert makes
// a silent mismatch a compile-time-adjacent failure instead of quietly
// generating wrong-sized combinations.
static_assert(kNumbersPerDraw == 6,
              "RankingEngine's combination generator is hardcoded for 6-number draws");

// Min-heap ordering: std::priority_queue is a max-heap by default
// (top() == largest per Compare); swapping in "a.score > b.score" as
// Compare flips it to a min-heap (top() == smallest score currently
// retained) -- the standard idiom, and exactly what a fixed-size
// top-N-by-score retention needs: evict the current minimum the moment
// something better than it is found.
struct ScoreMinHeapCompare {
    bool operator()(const RankedCombo &a, const RankedCombo &b) const { return a.score > b.score; }
};

} // namespace

std::vector<RankedCombo> RankingEngine::rank(const ModelArtifact &model, int topN,
                                             const CompositeWeights &weights, int poolMin,
                                             int poolMax) {
    if (topN <= 0) {
        return {};
    }

    std::priority_queue<RankedCombo, std::vector<RankedCombo>, ScoreMinHeapCompare> heap;

    // Nested index loops over strictly increasing numbers -- each of
    // the C(poolMax-poolMin+1, 6) combinations is generated, and
    // therefore scored, exactly once (no duplicates possible by
    // construction), and none of them are ever held anywhere except
    // transiently in `combo` and, if it earns a spot, in the
    // fixed-size `heap` -- the full ~22.9M-combination space (for the
    // documented poolMin=1/poolMax=53) is never materialized, per
    // CORE-203's memory bound.
    std::array<int, kNumbersPerDraw> combo{};
    for (combo[0] = poolMin; combo[0] <= poolMax - 5; ++combo[0]) {
        for (combo[1] = combo[0] + 1; combo[1] <= poolMax - 4; ++combo[1]) {
            for (combo[2] = combo[1] + 1; combo[2] <= poolMax - 3; ++combo[2]) {
                for (combo[3] = combo[2] + 1; combo[3] <= poolMax - 2; ++combo[3]) {
                    for (combo[4] = combo[3] + 1; combo[4] <= poolMax - 1; ++combo[4]) {
                        for (combo[5] = combo[4] + 1; combo[5] <= poolMax; ++combo[5]) {
                            const double score = CompositeScorer::score(model, combo, weights);

                            if (static_cast<int>(heap.size()) < topN) {
                                heap.push(RankedCombo{0, combo, score});
                            } else if (score > heap.top().score) {
                                heap.pop();
                                heap.push(RankedCombo{0, combo, score});
                            }
                        }
                    }
                }
            }
        }
    }

    // Drain the heap into a vector, then sort strictly descending by
    // score (the heap's internal order is ascending-by-pop, and gives
    // no tie-break guarantee) -- ties broken by ascending numeric combo
    // order so output is fully deterministic for the same model/weights
    // input, not just "some valid top-N".
    std::vector<RankedCombo> results;
    results.reserve(heap.size());
    while (!heap.empty()) {
        results.push_back(heap.top());
        heap.pop();
    }

    std::sort(results.begin(), results.end(), [](const RankedCombo &a, const RankedCombo &b) {
        if (a.score != b.score) {
            return a.score > b.score;
        }
        return a.combo < b.combo;
    });

    for (std::size_t i = 0; i < results.size(); ++i) {
        results[i].rank = static_cast<int>(i) + 1;
    }

    return results;
}

} // namespace lottopicker
