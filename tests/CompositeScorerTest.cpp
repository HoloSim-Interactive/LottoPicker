#include "lottopicker/CompositeScorer.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "lottopicker/PoolSizeNormalizer.h"

using lottopicker::CompositeScorer;
using lottopicker::DrawRecord;
using lottopicker::PoolSizeNormalizer;

namespace {

// TP-CORE-202's fixture: a single historical draw over a small pool
// (n=10, so hand-computed hypergeometric baselines stay simple),
// halfLifeDraws=1 so its one draw's age-0 weight is exactly 1.0.
// Numbers 1-6 are the draw itself (raw decay/cooc scores of 1.0 for
// every subset); numbers 7-10 never occurred (raw scores of 0.0).
// PoolSizeNormalizer::normalize's observed-minus-chance-expected
// baselines (CORE-206) are pure hypergeometric fractions of a pool of
// 10, hand-verified exactly via `fractions.Fraction` (see issue #15's
// implementation notes) rather than approximated by hand.
const std::vector<DrawRecord> kFixture = {
    DrawRecord{"2022-01-01", {1, 2, 3, 4, 5, 6}, 10},
};

} // namespace

// TP-CORE-202 part 1: composite formula correctness for the exact
// combination matching the one historical draw, default weights
// (w1=1.0, w2=1, w3=2, w4=4, w5=8, w6=16 per docs/SDD.md's Algorithm
// Design). Hand-computed exactly (exact rational arithmetic):
//   norm_decay(k in {1..6}) = 1 - 6/10 = 2/5
//   composite({1,2,3,4,5,6}) = 5748/35 = 164.22857142857143...
TEST_CASE("CompositeScorer combines norm_decay and norm_cooc per the documented formula",
          "[CORE-202]") {
    const auto normalized = PoolSizeNormalizer::normalize(kFixture, 1, 10, /*halfLifeDraws=*/1);

    const double composite = CompositeScorer::score(normalized, {1, 2, 3, 4, 5, 6});
    CHECK(composite == Catch::Approx(164.22857142857143).margin(1e-6));
}

// TP-CORE-202 part 2: a combination that only partially overlaps the
// observed draw (five numbers from it plus one, 7, never observed)
// still combines correctly -- subsets mixing an unobserved number
// contribute a negative (chance-expected-only) norm_cooc rather than
// being skipped or erroring. Hand-computed exactly:
//   composite({1,2,3,4,5,7}) = 1478/35 = 42.22857142857143...
TEST_CASE("CompositeScorer handles a combination only partially matching observed history",
          "[CORE-202]") {
    const auto normalized = PoolSizeNormalizer::normalize(kFixture, 1, 10, /*halfLifeDraws=*/1);

    const double composite = CompositeScorer::score(normalized, {1, 2, 3, 4, 5, 7});
    CHECK(composite == Catch::Approx(42.22857142857143).margin(1e-6));
}

// Weights are a runtime parameter, not hardcoded constants baked into
// the scoring loop (the RTVM item requires re-tuning without a code
// change) -- confirmed here by zeroing every group weight so only the
// w1 * per-number term survives:
//   2 * sum norm_decay(k) for k in {1,2,3,4,5,7} = 2 * 7/5 = 14/5 = 2.8
TEST_CASE("CompositeWeights actually changes the resulting score", "[CORE-202]") {
    const auto normalized = PoolSizeNormalizer::normalize(kFixture, 1, 10, /*halfLifeDraws=*/1);

    lottopicker::CompositeWeights weights;
    weights.numberWeight = 2.0;
    weights.groupWeights.fill(0.0);

    const double composite = CompositeScorer::score(normalized, {1, 2, 3, 4, 5, 7}, weights);
    CHECK(composite == Catch::Approx(2.8).margin(1e-6));
}

// Default weights match docs/SDD.md's Algorithm Design exactly --
// pinned here so an accidental change to the compiled defaults (as
// opposed to a deliberate CORE-205-driven re-tune) is caught by CI.
TEST_CASE("CompositeWeights defaults match docs/SDD.md", "[CORE-202]") {
    const lottopicker::CompositeWeights weights;
    CHECK(weights.numberWeight == 1.0);
    CHECK(weights.groupWeights[0] == 1.0);  // w2
    CHECK(weights.groupWeights[1] == 2.0);  // w3
    CHECK(weights.groupWeights[2] == 4.0);  // w4
    CHECK(weights.groupWeights[3] == 8.0);  // w5
    CHECK(weights.groupWeights[4] == 16.0); // w6
}

// Empty history (no draws yet) yields norm_decay == 0.0 and
// norm_cooc == 0.0 everywhere (PoolSizeNormalizer's own documented
// empty-history behavior), so composite() is well-defined (0.0) rather
// than erroring -- internal algorithmic code does not use exceptions
// for control flow (docs/SDD.md's Coding Standards).
TEST_CASE("CompositeScorer handles an empty-history normalization without error", "[CORE-202]") {
    const auto normalized = PoolSizeNormalizer::normalize({}, 1, 10);

    const double composite = CompositeScorer::score(normalized, {1, 2, 3, 4, 5, 6});
    CHECK(composite == 0.0);
}
