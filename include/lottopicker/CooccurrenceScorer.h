#pragma once

#include <array>
#include <map>
#include <vector>

#include "lottopicker/DrawRecord.h"

namespace lottopicker {

// Same exponential recency-decay half-life default as CORE-200
// (docs/SDD.md's Algorithm design: "HALF_LIFE_DRAWS defaults to 104
// (~1 year at Florida Lotto's twice-weekly cadence)"). CORE-201 reuses
// this value verbatim ("same decay weighting as CORE-200"), so it is
// re-declared here rather than depending on CORE-200's own header,
// which is not a declared dependency of this item (see RTVM CORE-201
// Dependencies: DATA-IN-100 only). If CORE-200 lands a shared decay
// utility, this constant/function pair should be consolidated onto it
// rather than kept duplicated.
inline constexpr int kDefaultHalfLifeDraws = 104;

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
