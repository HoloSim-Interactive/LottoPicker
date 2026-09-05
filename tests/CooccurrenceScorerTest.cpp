#include "lottopicker/CooccurrenceScorer.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using lottopicker::CooccurrenceScorer;
using lottopicker::DrawRecord;
using lottopicker::GroupKey;

namespace {

// TP-CORE-201's fixture: pair {4,17} co-occurs in 3 draws, pair {4,22}
// co-occurs in 1 draw. Extended (per part 2) with a 3-number group
// {4,17,30} appearing in 2 of those {4,17} draws vs. a lower-frequency
// {4,17,99} appearing in 1, and a 4-number group similarly.
const std::vector<DrawRecord> kFixture = {
    // {4,17} draws (3 total); two of them also carry {4,17,30} and,
    // among those, one also carries {4,17,30,50}.
    DrawRecord{"2020-01-01", {4, 17, 30, 50, 51, 52}}, // {4,17}, {4,17,30}, {4,17,30,50}
    DrawRecord{"2020-02-01", {4, 17, 30, 41, 42, 43}}, // {4,17}, {4,17,30}
    DrawRecord{"2020-03-01", {4, 17, 99, 21, 22, 23}}, // {4,17}, {4,17,99}
    // {4,22} draws (1 total).
    DrawRecord{"2020-04-01", {4, 22, 5, 6, 7, 8}},
};

} // namespace

// TP-CORE-201 part 1: co-occurrence scoring favors higher-frequency
// pairs — {4,17} (3 co-occurrences) scores higher than {4,22} (1
// co-occurrence).
TEST_CASE("CooccurrenceScorer ranks a higher-frequency pair above a lower-frequency one",
          "[CORE-201]") {
    auto scores = CooccurrenceScorer::score(kFixture);

    const double pair417 = CooccurrenceScorer::scoreFor(scores, GroupKey{4, 17});
    const double pair422 = CooccurrenceScorer::scoreFor(scores, GroupKey{4, 22});

    CHECK(pair417 > pair422);
    CHECK(pair422 > 0.0);
}

// TP-CORE-201 part 2: the same ordering holds at 3- and 4-number group
// sizes.
TEST_CASE("CooccurrenceScorer preserves frequency ordering at 3- and 4-number group sizes",
          "[CORE-201]") {
    auto scores = CooccurrenceScorer::score(kFixture);

    const double triple41730 = CooccurrenceScorer::scoreFor(scores, GroupKey{4, 17, 30});
    const double triple41799 = CooccurrenceScorer::scoreFor(scores, GroupKey{4, 17, 99});
    CHECK(triple41730 > triple41799);
    CHECK(triple41799 > 0.0);

    // {4,17,30,50} occurs once, {4,17,30,41,...} doesn't form a
    // second 4-group with 50 in it — compare against a group that
    // never occurs (should be 0, distinct from the observed one).
    const double quad4173050 = CooccurrenceScorer::scoreFor(scores, GroupKey{4, 17, 30, 50});
    const double quadNeverSeen = CooccurrenceScorer::scoreFor(scores, GroupKey{4, 17, 30, 99});
    CHECK(quad4173050 > quadNeverSeen);
    CHECK(quadNeverSeen == 0.0);
}

// Unobserved groups score exactly 0.0, never an error (sparse-map
// contract: absence means zero, per docs/SDD.md's Algorithm design).
TEST_CASE("CooccurrenceScorer scores an unobserved group as 0.0", "[CORE-201]") {
    auto scores = CooccurrenceScorer::score(kFixture);
    CHECK(CooccurrenceScorer::scoreFor(scores, GroupKey{1, 2}) == 0.0);
}

// Only groups with at least one historical occurrence are ever stored
// as keys — never the full combinatorial space (docs/SDD.md, sparse
// storage constraint shared with CORE-203/CORE-204/DATA-OUT-301).
TEST_CASE("CooccurrenceScorer stores only observed groups, not the full combinatorial space",
          "[CORE-201]") {
    auto scores = CooccurrenceScorer::score(kFixture);

    // 4 draws x C(6,2)=15 distinct-position pairs each, but far fewer
    // than the full pool's C(53,2)=1378 possible pairs are ever keyed.
    const auto &pairScores = scores[0]; // index 0 == group size 2 (kMinGroupSize)
    CHECK(pairScores.size() < 1378);
    CHECK_FALSE(pairScores.empty());
}

// The most recent draw's groups score higher than an identical-count
// but older group, confirming the "same decay weighting as CORE-200"
// requirement actually applies (recency matters, not just raw count).
TEST_CASE("CooccurrenceScorer weights a more recent co-occurrence higher than an older one",
          "[CORE-201]") {
    const std::vector<DrawRecord> fixture = {
        DrawRecord{"2020-01-01", {1, 2, 10, 11, 12, 13}}, // oldest: pair {1,2}
        DrawRecord{"2020-06-01", {3, 4, 20, 21, 22, 23}}, // unrelated, sits between in time
        DrawRecord{"2020-12-01", {5, 6, 30, 31, 32, 33}}, // most recent: pair {5,6}
    };

    auto scores = CooccurrenceScorer::score(fixture);

    const double olderPair = CooccurrenceScorer::scoreFor(scores, GroupKey{1, 2});
    const double recentPair = CooccurrenceScorer::scoreFor(scores, GroupKey{5, 6});

    CHECK(recentPair > olderPair);
}

// decayWeight itself: age 0 (most recent draw) weighs exactly 1.0, and
// weight strictly decreases as age grows, matching
// w(age) = exp(-ln(2) * age / halfLifeDraws).
TEST_CASE("CooccurrenceScorer::decayWeight is 1.0 at age 0 and strictly decreasing", "[CORE-201]") {
    CHECK(CooccurrenceScorer::decayWeight(0) == Catch::Approx(1.0));
    CHECK(CooccurrenceScorer::decayWeight(104) == Catch::Approx(0.5).margin(1e-9));
    CHECK(CooccurrenceScorer::decayWeight(0) > CooccurrenceScorer::decayWeight(50));
    CHECK(CooccurrenceScorer::decayWeight(50) > CooccurrenceScorer::decayWeight(200));
}

// An empty history yields all-empty score maps, not an error (internal
// algorithmic code doesn't use exceptions for control flow, per
// docs/SDD.md's Coding Standards).
TEST_CASE("CooccurrenceScorer handles empty history without error", "[CORE-201]") {
    auto scores = CooccurrenceScorer::score({});
    for (const auto &map : scores) {
        CHECK(map.empty());
    }
}
