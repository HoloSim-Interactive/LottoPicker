#pragma once

#include <array>
#include <map>
#include <vector>

#include "lottopicker/DecayScorer.h" // kDefaultHalfLifeDraws (shared with CORE-200)
#include "lottopicker/DrawRecord.h"

namespace lottopicker {

// `kDefaultHalfLifeDraws` (the exponential recency-decay half-life,
// "same decay weighting as CORE-200" per docs/SDD.md's Algorithm
// design) now comes from DecayScorer.h rather than being re-declared
// here. It was originally duplicated because, when CORE-201 (#12) was
// implemented, CORE-200 (#11) was mid-flight in parallel with no
// merged code yet, and CORE-201's declared RTVM Dependency is
// DATA-IN-100 only. Both have since merged; consolidating onto one
// definition avoids the redefinition error that surfaces the moment
// any single translation unit needs both headers (as CORE-206's
// PoolSizeNormalizer does).

// Group sizes CORE-201 scores: pairs through the full 6-number draw.
// kMaxGroupSize intentionally equals kNumbersPerDraw (DrawRecord.h) —
// a group can never be larger than one draw.
inline constexpr int kMinGroupSize = 2;
inline constexpr int kMaxGroupSize = kNumbersPerDraw;
inline constexpr std::size_t kGroupSizeCount =
    static_cast<std::size_t>(kMaxGroupSize - kMinGroupSize + 1);

// One co-occurrence group's key: numbers sorted ascending, size in
// [kMinGroupSize, kMaxGroupSize]. DrawRecord::numbers is already
// sorted ascending (its own documented invariant), so every group
// derived from a draw is built already in sorted order.
using GroupKey = std::vector<int>;

// Sparse per-group-size score map: only groups observed at least once
// historically are present as keys (docs/SDD.md's Algorithm design —
// "never the full combinatorial space", consistent with CORE-203's
// memory bound). Absence of a key means a score of 0.0, not an error.
using GroupScoreMap = std::map<GroupKey, double>;

// All group scores, indexed by `groupSize - kMinGroupSize` (index 0 =
// pairs, ... index 4 = full 6-number groups).
using CooccurrenceScores = std::array<GroupScoreMap, kGroupSizeCount>;

// Computes co-occurrence scores for every distinct group of size
// 2..6 observed in `history` (CORE-201):
//
//   cooc_score(group) = Σ_{d : group ⊆ draw(d)} w(age(d))
//   w(age) = exp(-ln(2) · age / halfLifeDraws)
//
// matching docs/SDD.md's Algorithm design section, same decay
// weighting as CORE-200. `age(d)` is the number of draws between `d`
// and the most recent draw in `history` (draw-index based, not
// calendar time, so behavior is deterministic regardless of gaps in
// the draw schedule) — `history` may be supplied in any order; this
// class sorts a local copy by `date` to establish draw order.
//
// Internal algorithmic code per docs/SDD.md's Coding Standards: no
// exceptions for control flow. An empty `history` yields all-empty
// score maps, not an error.
class CooccurrenceScorer {
public:
    static CooccurrenceScores score(const std::vector<DrawRecord> &history,
                                    int halfLifeDraws = kDefaultHalfLifeDraws);

    // Decay weight for one draw's age (0 = the most recent draw).
    // Exposed standalone so CORE-202's composite scorer (and tests)
    // can reuse the exact same weighting CORE-200/CORE-201 use.
    static double decayWeight(int age, int halfLifeDraws = kDefaultHalfLifeDraws);

    // Convenience lookup: score for one specific sorted group of any
    // valid size, 0.0 if never observed historically or if `group`'s
    // size is outside [kMinGroupSize, kMaxGroupSize].
    static double scoreFor(const CooccurrenceScores &scores, const GroupKey &sortedGroup);
};

} // namespace lottopicker
