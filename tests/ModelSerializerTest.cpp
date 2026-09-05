#include "lottopicker/ModelSerializer.h"

#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <random>

#include "lottopicker/Errors.h"

using lottopicker::ModelArtifact;
using lottopicker::ModelSerializer;
using lottopicker::ModelStoreError;

namespace {

std::filesystem::path uniqueTempPath() {
    static std::mt19937_64 rng{std::random_device{}()};
    return std::filesystem::temp_directory_path() /
           ("lottopicker_model_serializer_test_" + std::to_string(rng()) + ".model");
}

ModelArtifact sampleArtifact() {
    ModelArtifact artifact;
    artifact.sourceHash = "deadbeefcafef00d";
    artifact.earliestDate = "2018-01-01";
    artifact.latestDate = "2022-01-01";
    artifact.drawCount = 8;
    // A value chosen specifically because it is NOT exactly
    // representable in a short decimal form -- confirms round-trip
    // precision (TP-DATA-OUT-301's bit-for-bit clause), not just a
    // "looks equal after truncation" comparison.
    artifact.perNumber[1] = 0.1;
    artifact.perNumber[7] = 0.938026448787062;
    artifact.perNumber[40] = 0.0;
    artifact.groupScores[2][{4, 17}] = 0.01727460702622849;
    artifact.groupScores[2][{4, 22}] = -0.003;
    artifact.groupScores[3][{1, 2, 3}] = 1.0 / 3.0;
    artifact.groupScores[6][{1, 2, 3, 4, 5, 6}] = 12345.6789;
    return artifact;
}

} // namespace

// TP-DATA-OUT-301 part 1: writing a model artifact then reading it back
// round-trips every score and metadata field exactly.
TEST_CASE("ModelSerializer round-trips a model artifact exactly", "[DATA-OUT-301][CORE-204]") {
    const ModelArtifact original = sampleArtifact();
    const std::filesystem::path path = uniqueTempPath();

    ModelSerializer::write(original, path);
    const ModelArtifact roundTripped = ModelSerializer::read(path);

    CHECK(roundTripped == original);
    CHECK(roundTripped.perNumber.at(7) == original.perNumber.at(7));
    CHECK(roundTripped.groupScores.at(2).at({4, 17}) == original.groupScores.at(2).at({4, 17}));

    std::filesystem::remove(path);
}

TEST_CASE("ModelSerializer round-trips a model built from empty history",
          "[DATA-OUT-301][CORE-204]") {
    ModelArtifact empty;
    empty.sourceHash = "0000000000000000000000000000000000000000000000000000000000000000";
    empty.drawCount = 0;
    // earliestDate/latestDate stay empty (docs comment: "Empty strings
    // if built from zero records"); perNumber/groupScores stay empty.
    const std::filesystem::path path = uniqueTempPath();

    ModelSerializer::write(empty, path);
    const ModelArtifact roundTripped = ModelSerializer::read(path);

    CHECK(roundTripped == empty);

    std::filesystem::remove(path);
}

TEST_CASE("ModelSerializer::read throws ModelStoreError for a missing file",
          "[DATA-OUT-301][CORE-204]") {
    const std::filesystem::path path = uniqueTempPath();
    std::filesystem::remove(path);

    REQUIRE_THROWS_AS(ModelSerializer::read(path), ModelStoreError);
}

TEST_CASE("ModelSerializer::read throws ModelStoreError for a foreign/corrupt file",
          "[DATA-OUT-301][CORE-204]") {
    const std::filesystem::path path = uniqueTempPath();
    std::ofstream out(path);
    out << "not a model artifact at all\n";
    out.close();

    REQUIRE_THROWS_AS(ModelSerializer::read(path), ModelStoreError);

    std::filesystem::remove(path);
}

TEST_CASE("ModelSerializer::tryRead returns nullopt instead of throwing for a corrupt file",
          "[DATA-OUT-301][CORE-204]") {
    const std::filesystem::path path = uniqueTempPath();
    std::ofstream out(path);
    out << "not a model artifact at all\n";
    out.close();

    CHECK_FALSE(ModelSerializer::tryRead(path).has_value());

    std::filesystem::remove(path);
}

TEST_CASE("ModelSerializer::tryRead returns nullopt for a missing file",
          "[DATA-OUT-301][CORE-204]") {
    const std::filesystem::path path = uniqueTempPath();
    std::filesystem::remove(path);

    CHECK_FALSE(ModelSerializer::tryRead(path).has_value());
}
