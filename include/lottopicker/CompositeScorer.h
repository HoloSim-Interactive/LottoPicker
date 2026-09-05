#pragma once

#include <array>
#include <bitset>

#include "lottopicker/CooccurrenceScorer.h" // kMinGroupSize/kMaxGroupSize, kGroupSizeCount, GroupKey
#include "lottopicker/DrawRecord.h"         // kNumbersPerDraw
#include "lottopicker/PoolSizeNormalizer.h" // PoolSizeNormalizer::Result

namespace lottopicker {

// Default weights, per docs/SDD.md's Algorithm Design: `w1 = 1.0` for
// the per-number term, and group weights increasing with group size
// (`w2=1, w3=2, w4=4, w5=8, w6=16`) -- documented starting points,
// explicitly expected to be re-tuned later using CORE-205's backtest
// as the empirical validation loop, not fixed constants.
inline constexpr double kDefaultNumberWeight = 1.0;
inline constexpr std::array<double, kGroupSizeCount> kDefaultGroupWeights = {1.0, 2.0, 4.0, 8.0,
                                                                             16.0};

// Tunable weighting for CompositeScorer::score(), passed by value
// rather than hardcoded inside it, so re-tuning (CORE-205's backtest
// loop) is a matter of constructing a different CompositeWeights and
// does not require a code change to CompositeScorer. Defaults to the
// values above when default-constructed. (Kept at namespace scope,
// like CooccurrenceScorer's GroupKey/CooccurrenceScores, rather than
// nested inside CompositeScorer: a nested aggregate can't be used as
// its own enclosing member function's default-argument value --
// its default member initializers aren't complete until the
// *outermost* enclosing class is, which CompositeScorer isn't yet at
// the point score() is declared.)
struct CompositeWeights {
    // w1: multiplies the summed per-number norm_decay terms.
    double numberWeight = kDefaultNumberWeight;
    // w_g, g in [kMinGroupSize, kMaxGroupSize], indexed by
    // (groupSize - kMinGroupSize): multiplies the summed
    // norm_cooc terms for every subset of that size.
    std::array<double, kGroupSizeCount> groupWeights = kDefaultGroupWeights;
};

// CORE-202: combines CORE-206's normalized per-number decay scores and
// per-group co-occurrence scores into one composite score per full
// 6-number combination, per docs/SDD.md's Algorithm Design section
// (itself informed by CORE-207's research recommendation 4 -- a
// documented, tunable *linear* combination, deliberately not a more
// expressive model given the limited historical-draw sample size):
//
//   composite(combo) = w1 * Sum_{k in combo} norm_decay(k)
//                     + Sum_{g=2..6} w_g * Sum_{group subset combo, |group|=g} norm_cooc(group)
//
// `norm_decay`/`norm_cooc` come from CORE-206's PoolSizeNormalizer::Result
// (already computed once per ranking/backtest run and reused across
// every combination scored -- CORE-203 iterates ~22.9M combinations, so
// this class does no per-call recomputation of the normalized inputs
// themselves).
class CompositeScorer {
public:
    // composite(combo) for one full 6-number combination, per the
    // formula above. `combo` must be exactly kNumbersPerDraw distinct
    // numbers, sorted ascending (same invariant DrawRecord::numbers and
    // CooccurrenceScorer's GroupKey rely on, so every subset built here
    // is already in sorted order); each number must fall within
    // `normalized`'s [poolMin, poolMax] range. Enumerates all `2^6 - 1`
    // non-empty subsets of `combo` via a kNumbersPerDraw-bit mask (same
    // technique as CooccurrenceScorer::score), keeping only sizes in
    // [kMinGroupSize, kMaxGroupSize] for the co-occurrence term -- no
    // combinatorics library needed since a combo can never contain more
    // than kNumbersPerDraw numbers.
    static double score(const PoolSizeNormalizer::Result &normalized,
                        const std::array<int, kNumbersPerDraw> &combo,
                        const CompositeWeights &weights = CompositeWeights{});
};

} // namespace lottopicker
