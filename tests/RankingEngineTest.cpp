#include "lottopicker/RankingEngine.h"

#include <array>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cmath>
#include <cstddef>
#include <set>
#include <vector>

using lottopicker::CompositeWeights;
using lottopicker::ModelArtifact;
using lottopicker::RankedCombo;
using lottopicker::RankingEngine;

namespace {

// A ModelArtifact whose norm_decay(k) == k for every pool number, and
// every group weight zeroed out (see the test below), so
// composite(combo) reduces to exactly Sum(combo) -- hand-verifiable
// without needing CORE-200/201/206's own machinery, matching the
// "hand-tractable fixture" technique used throughout CORE-200/201/202/
// 206's own tests. groupScores/baselineCooc are left empty/zero, which
// is fine here since the tests below zero every group weight, making
// the co-occurrence term's value irrelevant to the composite result.
ModelArtifact perNumberEqualsValueArtifact(int poolMin, int poolMax) {
    ModelArtifact artifact;
    for (int number = poolMin; number <= poolMax; ++number) {
        artifact.perNumber[number] = static_cast<double>(number);
    }
    return artifact;
}

CompositeWeights zeroGroupWeights() {
    CompositeWeights weights;
    weights.groupWeights.fill(0.0);
    return weights;
}

// norm_decay(k) == 2^k, every group weight zeroed (see
// zeroGroupWeights()) -- distinct powers of two sum to a distinct total
// for every distinct 6-element subset (uniqueness of binary
// representation), so composite(combo) is guaranteed never to tie
// between two different combinations. Used specifically to verify
// TP-CORE-203's "strictly descending" wording means no adjacent ties,
// not just "non-increasing" -- perNumberEqualsValueArtifact's plain
// Sum(combo) fixture has many genuine ties among lower-ranked
// combinations and isn't suitable for that particular check.
ModelArtifact perNumberPowersOfTwoArtifact(int poolMin, int poolMax) {
    ModelArtifact artifact;
    for (int number = poolMin; number <= poolMax; ++number) {
        artifact.perNumber[number] = std::pow(2.0, number);
    }
    return artifact;
}

} // namespace

// TP-CORE-203 part 1: retention over a full (small, for hand-
// tractability -- same scaled-down technique as CORE-202/206's own
// tests) combination space, `topN` smaller than the space's size.
// Pool [1,20]: C(20,6) = 38,760 combinations, composite(combo) ==
// Sum(2^k for k in combo) per the fixture above (see
// perNumberPowersOfTwoArtifact -- chosen specifically so no two
// combinations can ever tie). Expected exactly 100 results, strictly
// descending (no adjacent ties), no duplicate combinations.
TEST_CASE("RankingEngine retains exactly topN results, strictly descending, no duplicates",
          "[CORE-203]") {
    const ModelArtifact artifact = perNumberPowersOfTwoArtifact(1, 20);
    const std::vector<RankedCombo> results =
        RankingEngine::rank(artifact, /*topN=*/100, zeroGroupWeights(), 1, 20);

    REQUIRE(results.size() == 100);

    std::set<std::array<int, 6>> seen;
    for (std::size_t i = 0; i < results.size(); ++i) {
        CHECK(results[i].rank == static_cast<int>(i) + 1);
        if (i > 0) {
            CHECK(results[i - 1].score > results[i].score); // strictly descending, no ties
        }
        const auto [unusedIt, inserted] = seen.insert(results[i].combo);
        CHECK(inserted); // no duplicate combination
        (void)unusedIt;
    }
}

// TP-DATA-OUT-300 — Objective: ranked-list structure integrity.
// Fixture run with top_n=5 exactly as the test procedure specifies
// (deliberately distinct from TP-CORE-203 part 1's topN=100 case
// above, so this test stands on its own as direct evidence for the
// DATA-OUT-300 item rather than relying on a reader to infer it from
// the CORE-203 case). Reuses the powers-of-two fixture so "strictly
// descending" excludes ties, not just non-increasing order. Each
// combination's 6-distinct-numbers-in-pool-range property is asserted
// explicitly here (via a std::set of size 6 and a range check) even
// though RankingEngine's construction-by-nested-index-loops already
// guarantees it structurally -- TP-DATA-OUT-300 calls it out as an
// expectation to verify, so it gets an explicit assertion rather than
// being taken on faith from CORE-203's design.
TEST_CASE("Ranked-list structure has exactly topN entries, strictly descending scores, "
          "and 6 distinct in-pool numbers per combination",
          "[DATA-OUT-300][CORE-203]") {
    constexpr int poolMin = 1;
    constexpr int poolMax = 20;
    const ModelArtifact artifact = perNumberPowersOfTwoArtifact(poolMin, poolMax);
    const std::vector<RankedCombo> results =
        RankingEngine::rank(artifact, /*topN=*/5, zeroGroupWeights(), poolMin, poolMax);

    REQUIRE(results.size() == 5);

    for (std::size_t i = 0; i < results.size(); ++i) {
        CHECK(results[i].rank == static_cast<int>(i) + 1);
        if (i > 0) {
            CHECK(results[i - 1].score > results[i].score); // strictly descending
        }

        const std::set<int> distinctNumbers(results[i].combo.begin(), results[i].combo.end());
        CHECK(distinctNumbers.size() == 6); // 6 distinct numbers, no repeats
        for (int number : results[i].combo) {
            CHECK(number >= poolMin);
            CHECK(number <= poolMax);
        }
    }
}

// TP-CORE-203 part 2 (Analysis, code-review): this test doesn't (and
// can't) directly assert "the full space was never materialized" at
// runtime, but it does confirm the *output* CORE-203's streaming
// top-N-heap design is supposed to produce is correct at the extremes
// -- the single highest-scoring combination in the space, and the
// exact rank-2 combination, both hand-derivable: with composite(combo)
// == Sum(combo) over distinct numbers in [1,10], the unique maximum is
// {5,6,7,8,9,10} (sum 45); the unique second-highest is {4,6,7,8,9,10}
// (sum 44), since the only way to reduce the maximum sum by exactly 1
// is swapping out the smallest included number (5) for the next
// available one (4).
TEST_CASE("RankingEngine's rank 1 and rank 2 combinations match hand-derived expectations",
          "[CORE-203]") {
    const ModelArtifact artifact = perNumberEqualsValueArtifact(1, 10);
    const std::vector<RankedCombo> results =
        RankingEngine::rank(artifact, /*topN=*/3, zeroGroupWeights(), 1, 10);

    REQUIRE(results.size() == 3);
    CHECK(results[0].combo == std::array<int, 6>{5, 6, 7, 8, 9, 10});
    CHECK(results[0].score == Catch::Approx(45.0));
    CHECK(results[1].combo == std::array<int, 6>{4, 6, 7, 8, 9, 10});
    CHECK(results[1].score == Catch::Approx(44.0));
}

// A pool exactly kNumbersPerDraw wide has exactly one possible
// combination -- topN larger than the space's size returns only what
// exists, not a padded/duplicated/crashing result.
TEST_CASE("RankingEngine returns fewer than topN when the combination space is smaller",
          "[CORE-203]") {
    const ModelArtifact artifact = perNumberEqualsValueArtifact(1, 6);
    const std::vector<RankedCombo> results =
        RankingEngine::rank(artifact, /*topN=*/100, zeroGroupWeights(), 1, 6);

    REQUIRE(results.size() == 1);
    CHECK(results[0].rank == 1);
    CHECK(results[0].combo == std::array<int, 6>{1, 2, 3, 4, 5, 6});
}

// topN <= 0 (should not occur on a properly-validated config, see
// UI-002's kMinTopN, but this is internal algorithmic code that
// doesn't re-validate its caller's input) yields an empty result
// rather than undefined behavior from touching an empty heap.
TEST_CASE("RankingEngine returns an empty result for a non-positive topN", "[CORE-203]") {
    const ModelArtifact artifact = perNumberEqualsValueArtifact(1, 10);
    CHECK(RankingEngine::rank(artifact, 0).empty());
    CHECK(RankingEngine::rank(artifact, -5).empty());
}

// Default CompositeWeights (not zeroed) still produces a valid, fully-
// populated top-N result against an artifact with real group data --
// confirms RankingEngine actually calls CompositeScorer's full
// formula (both terms), not just a per-number shortcut, end to end.
TEST_CASE("RankingEngine works with default (non-zero) CompositeWeights", "[CORE-203]") {
    ModelArtifact artifact = perNumberEqualsValueArtifact(1, 8);
    artifact.groupScores[2][{1, 2}] = 5.0;
    artifact.baselineCooc.fill(0.1);

    const std::vector<RankedCombo> results = RankingEngine::rank(artifact, /*topN=*/5, {}, 1, 8);

    REQUIRE(results.size() == 5);
    for (std::size_t i = 1; i < results.size(); ++i) {
        CHECK(results[i - 1].score >= results[i].score);
    }
}
