#include "lottopicker/BacktestEngine.h"

#include <cstddef>

#include "lottopicker/EraTable.h" // poolSizeForDate
#include "lottopicker/Errors.h"
#include "lottopicker/ModelArtifact.h"
#include "lottopicker/ModelStore.h"
#include "lottopicker/PoolSizeNormalizer.h" // hypergeometricProbability
#include "lottopicker/RankingEngine.h"

namespace lottopicker {

namespace {

// Number of positions shared between two sorted, distinct 6-number
// combinations -- a two-pointer merge over both sorted arrays
// (DrawRecord::numbers/RankedCombo::combo's shared "sorted ascending"
// invariant), O(kNumbersPerDraw) rather than a set-based intersection.
int sharedCount(const std::array<int, kNumbersPerDraw> &a,
               const std::array<int, kNumbersPerDraw> &b) {
    int shared = 0;
    std::size_t i = 0;
    std::size_t j = 0;
    while (i < a.size() && j < b.size()) {
        if (a[i] == b[j]) {
            ++shared;
            ++i;
            ++j;
        } else if (a[i] < b[j]) {
            ++i;
        } else {
            ++j;
        }
    }
    return shared;
}

} // namespace

BacktestResult BacktestEngine::run(const std::vector<DrawRecord> &fullHistory,
                                   const std::string &sampleDate, int topN,
                                   const CompositeWeights &weights, int poolMin,
                                   int halfLifeDraws, const std::vector<EraBoundary> &eras) {
    const DrawRecord *actual = nullptr;
    for (const DrawRecord &record : fullHistory) {
        if (record.date == sampleDate) {
            actual = &record;
            break;
        }
    }
    if (actual == nullptr) {
        throw BacktestError("no historical draw found on backtest sample date: " + sampleDate);
    }

    std::vector<DrawRecord> truncated;
    truncated.reserve(fullHistory.size());
    for (const DrawRecord &record : fullHistory) {
        if (record.date < sampleDate) {
            truncated.push_back(record);
        }
    }
    if (truncated.empty()) {
        throw BacktestError("no historical draws available before backtest sample date: " +
                            sampleDate);
    }

    // Rank within the era-appropriate pool in effect on the sample
    // date itself (see BacktestEngine.h's "poolMax is NOT a parameter"
    // comment) -- not the truncated history's own latest-tagged pool,
    // which could differ if `sampleDate` falls exactly on an era
    // boundary.
    const int poolMax = poolSizeForDate(sampleDate, eras);

    const ModelArtifact model =
        ModelStore::buildArtifact(truncated, /*sourceHash=*/"", poolMin, poolMax, halfLifeDraws);
    const std::vector<RankedCombo> ranked = RankingEngine::rank(model, topN, weights, poolMin, poolMax);

    BacktestResult result;
    result.sampleDate = sampleDate;
    result.actualDraw = actual->numbers;
    result.poolMax = poolMax;
    result.topN = static_cast<int>(ranked.size());

    for (const RankedCombo &entry : ranked) {
        const int shared = sharedCount(entry.combo, result.actualDraw);
        if (shared >= kMinContainmentLevel && shared <= kMaxContainmentLevel) {
            ++result.observedContainment[static_cast<std::size_t>(shared - kMinContainmentLevel)];
        }
        if (shared == kNumbersPerDraw) {
            // Combinations are unique by construction (RankingEngine's
            // nested-index-loop generator, see RankingEngine.cpp), so
            // at most one entry can ever reach a full 6-of-6 match --
            // safe to record rank unconditionally without checking for
            // a prior match.
            result.found = true;
            result.rank = entry.rank;
        }
    }

    if (result.found && result.topN > 0) {
        result.percentile = 100.0 * static_cast<double>(result.topN - result.rank + 1) /
                            static_cast<double>(result.topN);
    }

    for (int level = kMinContainmentLevel; level <= kMaxContainmentLevel; ++level) {
        result.chanceExpectedContainment[static_cast<std::size_t>(level - kMinContainmentLevel)] =
            static_cast<double>(result.topN) * PoolSizeNormalizer::hypergeometricProbability(level, poolMax);
    }

    return result;
}

} // namespace lottopicker
