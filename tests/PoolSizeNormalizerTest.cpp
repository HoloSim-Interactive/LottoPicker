#include "lottopicker/PoolSizeNormalizer.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>

using lottopicker::DrawRecord;
using lottopicker::GroupKey;
using lottopicker::PoolSizeNormalizer;

namespace {

// TP-CORE-206's fixture: era A (pool size 49, 3 draws) then era B (pool
// size 53, 5 draws), oldest to newest so ages run 7..0 (age 0 = most
// recent draw, index 7). `halfLifeDraws=1` is used everywhere below so
// weight(age) = (1/2)^age -- clean fractions, hand-computed exactly
// (via exact rational arithmetic, not float approximation) below and
// in the issue's implementation notes.
//
// Number 7 ("k") appears in 2 of the 3 era-A draws (indices 0, 2) and
// 2 of the 5 era-B draws (indices 4, 7), per TP-CORE-206 part 1's own
// fixture description. Number 13 co-occurs with 7 in exactly the two
// era-A draws and never in era B, forming an era-A-only pair for part
// 2 ("not discarded").
const std::vector<DrawRecord> kFixture = {
    DrawRecord{"2018-01-01", {7, 13, 20, 25, 30, 35}, 49}, // era A, age 7, has 7 & 13
    DrawRecord{"2018-06-01", {2, 9, 14, 22, 31, 40}, 49},  // era A, age 6
    DrawRecord{"2019-01-01", {7, 13, 18, 27, 33, 44}, 49}, // era A, age 5, has 7 & 13
    DrawRecord{"2020-01-01", {3, 10, 19, 28, 36, 50}, 53}, // era B, age 4
    DrawRecord{"2020-06-01", {5, 7, 21, 29, 37, 51}, 53},  // era B, age 3, has 7
    DrawRecord{"2021-01-01", {4, 11, 17, 24, 38, 52}, 53}, // era B, age 2
    DrawRecord{"2021-06-01", {6, 12, 16, 23, 39, 53}, 53}, // era B, age 1
    DrawRecord{"2022-01-01", {1, 7, 15, 26, 34, 48}, 53},  // era B, age 0, has 7
};

} // namespace

// TP-CORE-206 part 1: norm_decay(k) reflects k's occurrences relative
// to each era's own chance rate (6/49 vs 6/53) rather than a flat raw
// count of 4. Hand-computed exactly (exact rational arithmetic):
//   raw decay_score(7)  = 149/128
//   baselineDecay        = 5367/23744
//   norm_decay(7)         = 44545/47488 = 0.938026448787062...
TEST_CASE("PoolSizeNormalizer computes norm_decay via observed-minus-chance-expected",
          "[CORE-206]") {
    auto result = PoolSizeNormalizer::normalize(kFixture, 1, 53, /*halfLifeDraws=*/1);

    CHECK(result.normDecay(7) == Catch::Approx(0.938026448787062).margin(1e-6));
    // Not the flat raw count of 4, and not the undiscounted raw decay
    // score either (149/128 = 1.1640625) -- confirms the chance-
    // expected baseline was actually subtracted.
    CHECK(result.normDecay(7) != Catch::Approx(4.0));
    CHECK(result.normDecay(7) != Catch::Approx(1.1640625));
}

// TP-CORE-206 part 2: a group fully contained within only pre-change-
// era draws (pair {7,13}, era A only, pool 49) is not discarded --
// its norm_cooc is nonzero, confirming "normalize, don't discard"
// against an actual scoring run.
// Hand-computed exactly: raw cooc_score({7,13}) = 5/128,
// baselineCooc[pair] = 107605/4938752,
// norm_cooc({7,13}) = 85315/4938752 = 0.01727460702622849...
TEST_CASE("PoolSizeNormalizer does not discard a group observed only in a pre-change era",
          "[CORE-206]") {
    auto result = PoolSizeNormalizer::normalize(kFixture, 1, 53, /*halfLifeDraws=*/1);

    const double normPair = result.normCooc(GroupKey{7, 13});
    CHECK(normPair == Catch::Approx(0.01727460702622849).margin(1e-6));
    CHECK(normPair != 0.0);
}

// TP-CORE-206 part 3: expected_count(group, era) matches the hand-
// computed hypergeometric value C(n_era-g, 6-g)/C(n_era,6) x
// draws_in_era, for one group size/era from the fixture above
// (g=2, n_era=49, 3 draws in era A):
//   p(2,49) = C(47,4)/C(49,6) = 178365/13983816 = 5/392
//   expected_count = 3 x 5/392 = 15/392 = 0.03826530612244898...
TEST_CASE("PoolSizeNormalizer::expectedCount matches the hand-computed hypergeometric value",
          "[CORE-206]") {
    CHECK(PoolSizeNormalizer::expectedCount(2, 49, 3) ==
          Catch::Approx(0.03826530612244898).margin(1e-6));

    // A second, exactly-verifiable-by-hand case: a group the same size
    // as the whole draw, from a pool exactly that size -- every draw
    // is deterministically that one group, so p(6,6) = 1 and
    // expected_count = draws_in_era exactly.
    CHECK(PoolSizeNormalizer::expectedCount(6, 6, 10) == Catch::Approx(10.0));
}

// hypergeometricProbability guards invalid inputs defensively (0.0),
// rather than dividing by zero or indexing invalid combinatorics.
TEST_CASE("PoolSizeNormalizer::hypergeometricProbability guards invalid inputs", "[CORE-206]") {
    CHECK(PoolSizeNormalizer::hypergeometricProbability(1, 49) == 0.0); // groupSize < kMinGroupSize
    CHECK(PoolSizeNormalizer::hypergeometricProbability(7, 49) == 0.0); // groupSize > kMaxGroupSize
    CHECK(PoolSizeNormalizer::hypergeometricProbability(2, 5) == 0.0); // poolSize < kNumbersPerDraw
}

// An untagged record (poolSize == 0, EraTagger's sentinel) defensively
// falls back to kCurrentPoolMax for its baseline contribution rather
// than dividing by zero -- doesn't crash, doesn't corrupt other
// records' normalization.
TEST_CASE("PoolSizeNormalizer tolerates an untagged record without dividing by zero",
          "[CORE-206]") {
    const std::vector<DrawRecord> untaggedFixture = {
        DrawRecord{"2022-01-01", {1, 2, 3, 4, 5, 6}, 0}, // untagged
    };

    auto result = PoolSizeNormalizer::normalize(untaggedFixture);
    CHECK(std::isfinite(result.normDecay(1)));
    CHECK(std::isfinite(result.normCooc(GroupKey{1, 2})));
}

// An unobserved group's norm_cooc is still a well-defined, typically
// negative, "surprise" (0.0 raw minus a positive baseline) rather than
// requiring the group to have been stored -- CORE-201's sparse-map
// contract is preserved.
TEST_CASE("PoolSizeNormalizer computes norm_cooc for an unobserved group", "[CORE-206]") {
    auto result = PoolSizeNormalizer::normalize(kFixture, 1, 53, /*halfLifeDraws=*/1);

    const double normUnobserved = result.normCooc(GroupKey{1, 2});
    CHECK(normUnobserved < 0.0);
}

// Empty history yields norm_decay == 0.0 for every number (no draws to
// be "surprised" relative to) and no groups observed -- not an error,
// per docs/SDD.md's Coding Standards (no exceptions for control flow
// in internal algorithmic code).
TEST_CASE("PoolSizeNormalizer handles empty history without error", "[CORE-206]") {
    auto result = PoolSizeNormalizer::normalize({}, 1, 53);
    CHECK(result.normDecay(1) == 0.0);
    CHECK(result.normCooc(GroupKey{1, 2}) == 0.0);
}
