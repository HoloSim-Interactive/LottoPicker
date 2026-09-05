#include "lottopicker/DecayScorer.h"

#include <algorithm>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cmath>

#include "lottopicker/DrawRecord.h"

using lottopicker::DecayScorer;
using lottopicker::DrawRecord;

namespace {

// TP-CORE-200's synthetic 5-draw fixture: number 7 appears only in the
// most recent draw, number 12 appears only in the oldest draw. Every
// other number is unique to its own draw so there's no cross-number
// interference. Number 40 appears in none of them (TP-CORE-200 case 2).
//
// Listed here in chronological (oldest-first) order for readability;
// individual tests below feed it in file/shuffled order too, to prove
// DecayScorer sorts by date itself rather than trusting caller order.
std::vector<DrawRecord> chronologicalFixture() {
    return {
        DrawRecord{"2024-01-01", {1, 2, 3, 4, 5, 12}}, // oldest
        DrawRecord{"2024-01-08", {13, 14, 15, 16, 17, 18}},
        DrawRecord{"2024-01-15", {19, 20, 21, 22, 23, 24}},
        DrawRecord{"2024-01-22", {25, 26, 27, 28, 29, 30}},
        DrawRecord{"2024-01-29", {7, 31, 32, 33, 34, 35}}, // most recent
    };
}

} // namespace

// TP-CORE-200 part 1: decay scoring favors recency — a number that only
// appears in the most recent draw outscores one that only appears in
// the oldest draw.
TEST_CASE("DecayScorer favors a number seen only in the most recent draw", "[CORE-200]") {
    const auto history = chronologicalFixture();

    const auto result = DecayScorer::score(history);

    CHECK(result.at(7) > result.at(12));
    CHECK(result.at(7) > 0.0);
    CHECK(result.at(12) > 0.0);
}

// TP-CORE-200 part 2: a number with zero historical occurrences gets
// the documented floor score of 0.0, not an error.
TEST_CASE("DecayScorer floors an unseen number's score at zero", "[CORE-200]") {
    const auto history = chronologicalFixture();

    const auto result = DecayScorer::score(history);

    CHECK(result.at(40) == 0.0);
}

// Recency ordering must not depend on the order DrawRecords are handed
// in — DecayScorer sorts by DrawRecord::date internally (see header),
// since CsvIngestor only preserves file order, not chronological order.
TEST_CASE("DecayScorer is independent of input vector order", "[CORE-200]") {
    auto shuffled = chronologicalFixture();
    std::reverse(shuffled.begin(), shuffled.end()); // now most-recent-first on disk

    const auto result = DecayScorer::score(shuffled);

    CHECK(result.at(7) > result.at(12));
}

// Exact-value check against the documented formula:
// decay_score(k) = Sum_d w(age(d)) * [k in draw(d)], w(age) = exp(-ln2 * age / HALF_LIFE_DRAWS).
// With the default half-life (104 draws) and 7 at age 0 in a 5-draw
// history, decay_score(7) == exp(0) == 1.0 exactly (only one occurrence,
// at age 0). decay_score(12) == exp(-ln2 * 4 / 104).
TEST_CASE("DecayScorer matches the documented exponential-decay formula exactly", "[CORE-200]") {
    const auto history = chronologicalFixture();

    const auto result =
        DecayScorer::score(history, lottopicker::kPoolMin, lottopicker::kCurrentPoolMax,
                           lottopicker::kDefaultHalfLifeDraws);

    CHECK(result.at(7) == Catch::Approx(1.0));
    const double expected12 = std::exp(-std::log(2.0) * 4.0 / 104.0);
    CHECK(result.at(12) == Catch::Approx(expected12));
}

// A custom half-life is honored, not just the compiled default.
TEST_CASE("DecayScorer honors a custom half-life", "[CORE-200]") {
    const auto history = chronologicalFixture();

    const auto result =
        DecayScorer::score(history, lottopicker::kPoolMin, lottopicker::kCurrentPoolMax,
                           /*halfLifeDraws=*/1);

    // age(12) = 4 draws, half-life 1 draw -> heavily decayed, much
    // smaller than with the default 104-draw half-life.
    const double expected12 = std::exp(-std::log(2.0) * 4.0 / 1.0);
    CHECK(result.at(12) == Catch::Approx(expected12));
}

TEST_CASE("DecayScorer returns all-zero scores for empty history", "[CORE-200]") {
    const std::vector<DrawRecord> empty;

    const auto result = DecayScorer::score(empty);

    CHECK(result.at(7) == 0.0);
    CHECK(result.at(53) == 0.0);
}
