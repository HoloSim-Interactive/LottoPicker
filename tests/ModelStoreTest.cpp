#include "lottopicker/ModelStore.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <random>

#include "lottopicker/CooccurrenceScorer.h"
#include "lottopicker/ModelSerializer.h"
#include "lottopicker/PoolSizeNormalizer.h"

using lottopicker::kMaxGroupSize;
using lottopicker::kMinGroupSize;
using lottopicker::ModelStore;
using lottopicker::PoolSizeNormalizer;

namespace {

std::filesystem::path uniqueTempCsvPath() {
    static std::mt19937_64 rng{std::random_device{}()};
    return std::filesystem::temp_directory_path() /
           ("lottopicker_model_store_test_" + std::to_string(rng()) + ".csv");
}

void writeCsv(const std::filesystem::path &path, const std::string &content) {
    std::ofstream out(path, std::ios::trunc);
    out << content;
}

constexpr const char *kFiveDrawCsv = "date,n1,n2,n3,n4,n5,n6\n"
                                     "2020-01-01,1,2,3,4,5,6\n"
                                     "2020-01-05,7,8,9,10,11,12\n"
                                     "2020-01-08,4,17,20,25,30,35\n"
                                     "2020-01-12,4,22,26,31,36,41\n"
                                     "2020-01-15,1,7,13,19,25,31\n";

} // namespace

// TP-CORE-204 part 1: running the tool's model-build logic twice
// against an unchanged data_file reuses the persisted artifact instead
// of recomputing it from raw history.
TEST_CASE("ModelStore reuses a persisted model when the source CSV is unchanged", "[CORE-204]") {
    const std::filesystem::path csvPath = uniqueTempCsvPath();
    const std::filesystem::path modelPath = ModelStore::defaultModelPath(csvPath);
    std::filesystem::remove(modelPath);
    writeCsv(csvPath, kFiveDrawCsv);

    const auto first = ModelStore::loadOrBuild(csvPath, modelPath);
    REQUIRE(first.wasRebuilt);
    REQUIRE(first.ingestErrors.empty());
    REQUIRE(first.artifact.drawCount == 5);
    REQUIRE(std::filesystem::exists(modelPath));

    const auto second = ModelStore::loadOrBuild(csvPath, modelPath);
    CHECK_FALSE(second.wasRebuilt);
    CHECK(second.artifact == first.artifact);

    std::filesystem::remove(csvPath);
    std::filesystem::remove(modelPath);
}

// TP-CORE-204 part 2: adding one new draw to the fixture CSV and
// rerunning regenerates the model artifact (the stale one is not
// reused).
TEST_CASE("ModelStore rebuilds the model after the source CSV changes", "[CORE-204]") {
    const std::filesystem::path csvPath = uniqueTempCsvPath();
    const std::filesystem::path modelPath = ModelStore::defaultModelPath(csvPath);
    std::filesystem::remove(modelPath);
    writeCsv(csvPath, kFiveDrawCsv);

    const auto first = ModelStore::loadOrBuild(csvPath, modelPath);
    REQUIRE(first.wasRebuilt);
    REQUIRE(first.artifact.drawCount == 5);

    writeCsv(csvPath, std::string(kFiveDrawCsv) + "2020-01-19,2,9,16,23,30,44\n");

    const auto second = ModelStore::loadOrBuild(csvPath, modelPath);
    CHECK(second.wasRebuilt);
    CHECK(second.artifact.drawCount == 6);
    CHECK(second.artifact.sourceHash != first.artifact.sourceHash);

    // The regenerated artifact is what's actually on disk now, not the
    // stale one -- confirms the rebuild path also re-persists.
    const auto reloaded = lottopicker::ModelSerializer::read(modelPath);
    CHECK(reloaded == second.artifact);

    std::filesystem::remove(csvPath);
    std::filesystem::remove(modelPath);
}

TEST_CASE("ModelStore rebuilds when no model artifact exists yet", "[CORE-204]") {
    const std::filesystem::path csvPath = uniqueTempCsvPath();
    const std::filesystem::path modelPath = ModelStore::defaultModelPath(csvPath);
    std::filesystem::remove(modelPath);
    writeCsv(csvPath, kFiveDrawCsv);

    const auto result = ModelStore::loadOrBuild(csvPath, modelPath);
    CHECK(result.wasRebuilt);
    CHECK(std::filesystem::exists(modelPath));

    std::filesystem::remove(csvPath);
    std::filesystem::remove(modelPath);
}

TEST_CASE("ModelStore rebuilds when the persisted artifact is corrupt", "[CORE-204]") {
    const std::filesystem::path csvPath = uniqueTempCsvPath();
    const std::filesystem::path modelPath = ModelStore::defaultModelPath(csvPath);
    writeCsv(csvPath, kFiveDrawCsv);
    writeCsv(modelPath, "not a valid model artifact\n");

    const auto result = ModelStore::loadOrBuild(csvPath, modelPath);
    CHECK(result.wasRebuilt);
    CHECK(result.artifact.drawCount == 5);

    std::filesystem::remove(csvPath);
    std::filesystem::remove(modelPath);
}

TEST_CASE("ModelStore surfaces per-row ingestion errors during a rebuild without failing it",
          "[CORE-204]") {
    const std::filesystem::path csvPath = uniqueTempCsvPath();
    const std::filesystem::path modelPath = ModelStore::defaultModelPath(csvPath);
    std::filesystem::remove(modelPath);
    writeCsv(csvPath, "date,n1,n2,n3,n4,n5,n6\n"
                      "2020-01-01,1,2,3,4,5,6\n"
                      "2020-01-05,1,2,3,4,5\n" // malformed: only 5 numbers
                      "2020-01-08,7,8,9,10,11,12\n");

    const auto result = ModelStore::loadOrBuild(csvPath, modelPath);
    CHECK(result.wasRebuilt);
    CHECK(result.artifact.drawCount == 2);
    REQUIRE(result.ingestErrors.size() == 1);
    CHECK(result.ingestErrors[0].row == 3);

    std::filesystem::remove(csvPath);
    std::filesystem::remove(modelPath);
}

TEST_CASE("ModelStore::computeSourceHash is deterministic and content-sensitive", "[CORE-204]") {
    const std::filesystem::path csvPath = uniqueTempCsvPath();
    writeCsv(csvPath, kFiveDrawCsv);

    const std::string hashA = ModelStore::computeSourceHash(csvPath);
    const std::string hashB = ModelStore::computeSourceHash(csvPath);
    CHECK(hashA == hashB);

    writeCsv(csvPath, std::string(kFiveDrawCsv) + "2020-01-19,2,9,16,23,30,44\n");
    const std::string hashC = ModelStore::computeSourceHash(csvPath);
    CHECK(hashA != hashC);

    std::filesystem::remove(csvPath);
}

// CORE-203 (issue #17): the built artifact's baselineCooc must match
// docs/SDD.md's documented formula, baselineCooc[g] = Sum_d w(age(d)) *
// p(g, n_era(d)) -- a single draw at age 0 (weight 1.0 regardless of
// halfLifeDraws) dated in the current era (poolSize 53, per EraTagger)
// reduces this to exactly hypergeometricProbability(g, 53) for each
// group size, letting this test verify against CORE-206's own already-
// Verified public formula rather than a second hand-typed fraction.
TEST_CASE("ModelStore's built artifact stores baselineCooc per the documented formula",
          "[CORE-203][CORE-204]") {
    const std::filesystem::path csvPath = uniqueTempCsvPath();
    const std::filesystem::path modelPath = ModelStore::defaultModelPath(csvPath);
    std::filesystem::remove(modelPath);
    writeCsv(csvPath, "date,n1,n2,n3,n4,n5,n6\n"
                      "2020-01-01,1,2,3,4,5,6\n");

    const auto result = ModelStore::loadOrBuild(csvPath, modelPath);
    REQUIRE(result.artifact.drawCount == 1);

    for (int groupSize = kMinGroupSize; groupSize <= kMaxGroupSize; ++groupSize) {
        const double expected = PoolSizeNormalizer::hypergeometricProbability(groupSize, 53);
        CHECK(result.artifact.baselineCooc[static_cast<std::size_t>(groupSize - kMinGroupSize)] ==
              Catch::Approx(expected).margin(1e-12));
    }

    std::filesystem::remove(csvPath);
    std::filesystem::remove(modelPath);
}

TEST_CASE("ModelStore::defaultModelPath appends .model alongside the data file", "[CORE-204]") {
    CHECK(ModelStore::defaultModelPath("history.csv") ==
          std::filesystem::path("history.csv.model"));
    CHECK(ModelStore::defaultModelPath(std::filesystem::path("/data/fixture.csv")) ==
          std::filesystem::path("/data/fixture.csv.model"));
}
