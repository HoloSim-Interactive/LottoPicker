#include "lottopicker/BacktestEngine.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>

#include "lottopicker/CompositeScorer.h"
#include "lottopicker/DrawRecord.h"
#include "lottopicker/EraTable.h"
#include "lottopicker/Errors.h"
#include "lottopicker/PoolSizeNormalizer.h"

using lottopicker::BacktestEngine;
using lottopicker::BacktestError;
using lottopicker::BacktestResult;
using lottopicker::CompositeWeights;
using lottopicker::DrawRecord;
using lottopicker::EraBoundary;
using lottopicker::kMaxContainmentLevel;
using lottopicker::kMinContainmentLevel;
using lottopicker::PoolSizeNormalizer;

namespace {

CompositeWeights zeroGroupWeights() {
    CompositeWeights weights;
    weights.groupWeights.fill(0.0);
    return weights;
}

} // namespace

// TP-CORE-205 part 1 -- containment counts hand-verified.
//
// Synthetic era: a single boundary putting the whole fixture at pool
// size 9 (small enough that C(9,6) = 84 is the *entire* combination
// space, hand-enumerable). `topN = 100` therefore retains every one of
// those 84 combinations (RankingEngine::rank's own documented "fewer
// only if the space itself has fewer than topN" behavior), so the
// containment breakdown below is exactly the full space's, independent
// of composite scores/weights entirely -- no need to reason about
// which combinations rank where.
//
// actualDraw = {1,2,3,4,5,6}; the pool's only 3 "outside" numbers are
// {7,8,9}. A random 6-of-9 combination shares `m` numbers with
// actualDraw and (6-m) with {7,8,9} -- since only 3 outside numbers
// exist, 6-m <= 3, i.e. m >= 3 always (every combination in this pool
// necessarily contains at least 3 of actualDraw's numbers, exactly the
// [3,6] range CORE-205 reports). Exact counts, by hypergeometric
// counting (choose m from the 6 matching numbers, 6-m from the 3
// outside ones):
//   m=3: C(6,3)*C(3,3) = 20*1  = 20
//   m=4: C(6,4)*C(3,2) = 15*3  = 45
//   m=5: C(6,5)*C(3,1) = 6*3   = 18
//   m=6: C(6,6)*C(3,0) = 1*1   = 1
//   total = 20+45+18+1 = 84 = C(9,6) -- confirms the derivation.
TEST_CASE("BacktestEngine reports exact hand-verified containment counts when the full "
          "combination space is retained",
          "[CORE-205]") {
    const std::vector<EraBoundary> syntheticEras = {{"1970-01-01", 9}};

    const std::vector<DrawRecord> history = {
        DrawRecord{"2019-01-01", {4, 5, 6, 7, 8, 9}, 9}, // training draw, before sample date
        DrawRecord{"2020-01-01", {1, 2, 3, 4, 5, 6}, 9}, // the held-out "actual" draw
    };

    const BacktestResult result =
        BacktestEngine::run(history, "2020-01-01", /*topN=*/100, zeroGroupWeights(), /*poolMin=*/1,
                            /*halfLifeDraws=*/104, syntheticEras);

    REQUIRE(result.poolMax == 9);
    REQUIRE(result.topN == 84); // the entire space, not the requested 100
    CHECK(result.actualDraw == std::array<int, 6>{1, 2, 3, 4, 5, 6});

    // The actual draw's own combination is necessarily one of the 84
    // retained -- it must be found, at some rank within [1, 84].
    REQUIRE(result.found);
    CHECK(result.rank >= 1);
    CHECK(result.rank <= 84);
    CHECK(result.percentile ==
          Catch::Approx(100.0 * (84.0 - result.rank + 1.0) / 84.0).margin(1e-9));

    REQUIRE(result.observedContainment.size() == 4);
    CHECK(result.observedContainment[3 - kMinContainmentLevel] == 20); // exactly 3 of 6
    CHECK(result.observedContainment[4 - kMinContainmentLevel] == 45); // exactly 4 of 6
    CHECK(result.observedContainment[5 - kMinContainmentLevel] == 18); // exactly 5 of 6
    CHECK(result.observedContainment[6 - kMinContainmentLevel] == 1);  // exactly 6 of 6 (itself)

    // chanceExpectedContainment reuses CORE-206's own hypergeometricProbability
    // formula directly (see BacktestEngine.h's documented rationale) --
    // cross-checked here against that already-Verified public function,
    // not a second hand-typed formula.
    for (int level = kMinContainmentLevel; level <= kMaxContainmentLevel; ++level) {
        const double expected = static_cast<double>(result.topN) *
                                PoolSizeNormalizer::hypergeometricProbability(level, 9);
        CHECK(result.chanceExpectedContainment[static_cast<std::size_t>(
                  level - kMinContainmentLevel)] == Catch::Approx(expected).margin(1e-9));
    }
}

// TP-CORE-205 part 2 -- "not found in top-N" rather than a false rank.
//
// A single training draw {1..6} makes every number in {1..6} score
// positively (norm_decay > 0, since each was observed) and every
// number in {7..12} score negatively (never observed, so norm_decay is
// purely the negative chance-expected baseline) -- with zeroed group
// weights, composite(combo) == Sum(norm_decay(k) for k in combo), so
// {1,2,3,4,5,6} is the unique highest-scoring combination in the whole
// 12-number pool and the fully-disjoint {7,...,12} (chosen as the
// "actual" held-out draw) is the unique *lowest*-scoring one --
// nowhere near topN=1's single retained slot. Since the two share zero
// numbers, none of the actual draw's partial-match variants land in
// the (size-1) top-N either, matching TP-CORE-205 part 2's fixture
// description exactly.
TEST_CASE("BacktestEngine reports \"not found\" when no partial-match variant lands in top-N",
          "[CORE-205]") {
    const std::vector<EraBoundary> syntheticEras = {{"1970-01-01", 12}};

    const std::vector<DrawRecord> history = {
        DrawRecord{"2019-01-01", {1, 2, 3, 4, 5, 6}, 12},    // training draw
        DrawRecord{"2020-01-01", {7, 8, 9, 10, 11, 12}, 12}, // disjoint held-out "actual" draw
    };

    const BacktestResult result =
        BacktestEngine::run(history, "2020-01-01", /*topN=*/1, zeroGroupWeights(), /*poolMin=*/1,
                            /*halfLifeDraws=*/104, syntheticEras);

    REQUIRE(result.topN == 1);
    CHECK_FALSE(result.found);
    CHECK(result.rank == 0);
    CHECK(result.percentile == Catch::Approx(0.0));

    for (int count : result.observedContainment) {
        CHECK(count == 0);
    }
}

TEST_CASE("BacktestEngine throws when no draw is dated exactly on the sample date", "[CORE-205]") {
    const std::vector<EraBoundary> syntheticEras = {{"1970-01-01", 9}};
    const std::vector<DrawRecord> history = {
        DrawRecord{"2019-01-01", {1, 2, 3, 4, 5, 6}, 9},
    };

    CHECK_THROWS_AS(
        BacktestEngine::run(history, "2020-01-01", 10, CompositeWeights{}, 1, 104, syntheticEras),
        BacktestError);
}

TEST_CASE("BacktestEngine throws when no draw exists strictly before the sample date",
          "[CORE-205]") {
    const std::vector<EraBoundary> syntheticEras = {{"1970-01-01", 9}};
    const std::vector<DrawRecord> history = {
        DrawRecord{"2019-01-01", {1, 2, 3, 4, 5, 6}, 9}, // the earliest, and only, record
    };

    CHECK_THROWS_AS(
        BacktestEngine::run(history, "2019-01-01", 10, CompositeWeights{}, 1, 104, syntheticEras),
        BacktestError);
}

// TP-DATA-OUT-302 -- backtest report structure integrity across 3
// sampled years.
//
// The report structure itself is just `std::vector<BacktestResult>`
// with one entry per requested date (see BacktestEngine.h's DATA-OUT-302
// comment) -- so this test builds that vector the same way a
// `--backtest <date1,date2,date3>` caller (UI-003/main.cpp) would, one
// `BacktestEngine::run` call per year, and checks the resulting
// structure directly.
//
// Reuses TP-CORE-205 part 1's exact pool-size-9 fixture shape (a single
// era boundary putting the whole history at pool 9, so C(9,6) = 84 is
// the entire combination space and topN=100 retains all of it) for all
// three years. That full-space-retention property makes the observed
// containment breakdown depend only on the *sizes* of the actual draw
// (6 numbers) and its outside-numbers complement (9-6=3), not on which
// specific numbers are drawn -- see TP-CORE-205 part 1's derivation --
// so every year is expected to reproduce that same hand-verified
// 20/45/18/1 breakdown at levels 3/4/5/6, "consistent with TP-CORE-205's
// results" per TP-DATA-OUT-302's own wording, even though each year's
// actual draw and truncated training set differ.
TEST_CASE("Backtest report is one populated row per sampled year, matching TP-CORE-205's "
          "containment counts",
          "[DATA-OUT-302][CORE-205]") {
    const std::vector<EraBoundary> syntheticEras = {{"1970-01-01", 9}};

    const std::vector<DrawRecord> history = {
        DrawRecord{"2016-01-01", {4, 5, 6, 7, 8, 9}, 9}, // training draw, before every sample year
        DrawRecord{"2017-01-01", {1, 2, 3, 4, 5, 6}, 9}, // year 1's held-out actual draw
        DrawRecord{"2018-01-01", {2, 3, 4, 5, 6, 7}, 9}, // year 2's held-out actual draw
        DrawRecord{"2019-01-01", {3, 4, 5, 6, 7, 8}, 9}, // year 3's held-out actual draw
    };
    const std::vector<std::string> sampleDates = {"2017-01-01", "2018-01-01", "2019-01-01"};

    std::vector<BacktestResult> report;
    report.reserve(sampleDates.size());
    for (const std::string &date : sampleDates) {
        report.push_back(BacktestEngine::run(history, date, /*topN=*/100, zeroGroupWeights(),
                                             /*poolMin=*/1, /*halfLifeDraws=*/104, syntheticEras));
    }

    // Exactly one row per sampled year.
    REQUIRE(report.size() == 3);

    for (std::size_t i = 0; i < report.size(); ++i) {
        const BacktestResult &row = report[i];
        CHECK(row.sampleDate == sampleDates[i]);

        // All fields populated: echoed actual draw, era-appropriate
        // pool/topN, a definite found/rank/percentile outcome (found is
        // guaranteed here since topN=100 > the 84-combination full
        // space), and a fully filled containment breakdown.
        for (int n : row.actualDraw) {
            CHECK(n >= 1);
            CHECK(n <= 9);
        }
        CHECK(row.poolMax == 9);
        CHECK(row.topN == 84); // full space, not the requested 100

        REQUIRE(row.found);
        CHECK(row.rank >= 1);
        CHECK(row.rank <= 84);
        CHECK(row.percentile == Catch::Approx(100.0 * (84.0 - row.rank + 1.0) / 84.0).margin(1e-9));

        REQUIRE(row.observedContainment.size() == 4);
        CHECK(row.observedContainment[3 - kMinContainmentLevel] == 20); // exactly 3 of 6
        CHECK(row.observedContainment[4 - kMinContainmentLevel] == 45); // exactly 4 of 6
        CHECK(row.observedContainment[5 - kMinContainmentLevel] == 18); // exactly 5 of 6
        CHECK(row.observedContainment[6 - kMinContainmentLevel] == 1);  // exactly 6 of 6

        for (int level = kMinContainmentLevel; level <= kMaxContainmentLevel; ++level) {
            const double expected =
                static_cast<double>(row.topN) *
                PoolSizeNormalizer::hypergeometricProbability(level, row.poolMax);
            CHECK(row.chanceExpectedContainment[static_cast<std::size_t>(
                      level - kMinContainmentLevel)] == Catch::Approx(expected).margin(1e-9));
        }
    }
}

// The era-appropriate pool bound (not a fixed constant) governs the
// ranking domain -- a sample date on/after the second boundary uses
// that era's (larger) pool size, not the first era's.
TEST_CASE("BacktestEngine ranks within the pool size in effect on the sample date", "[CORE-205]") {
    const std::vector<EraBoundary> syntheticEras = {
        {"1970-01-01", 7},
        {"2000-01-01", 9},
    };

    const std::vector<DrawRecord> history = {
        DrawRecord{"1999-01-01", {1, 2, 3, 4, 5, 6}, 7},
        DrawRecord{"2020-01-01", {1, 2, 3, 4, 5, 6}, 9}, // era B, pool 9
    };

    const BacktestResult result =
        BacktestEngine::run(history, "2020-01-01", /*topN=*/1, CompositeWeights{}, /*poolMin=*/1,
                            /*halfLifeDraws=*/104, syntheticEras);

    CHECK(result.poolMax == 9);
}
