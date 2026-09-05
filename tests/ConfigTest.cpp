#include "lottopicker/Config.h"

#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <random>

#include "lottopicker/Errors.h"

namespace {

// Writes `content` to a uniquely-named temp config file and returns its
// path. Each TEST_CASE gets its own file so tests can run in parallel
// without clobbering each other (mirrors CsvIngestorTest.cpp's
// writeTempCsv).
std::filesystem::path writeTempConfig(const std::string &content) {
    static std::mt19937_64 rng{std::random_device{}()};
    auto path = std::filesystem::temp_directory_path() /
                ("lottopicker_config_test_" + std::to_string(rng()) + ".cfg");
    std::ofstream out(path);
    out << content;
    out.close();
    return path;
}

} // namespace

using lottopicker::Config;
using lottopicker::ConfigFormatError;
using lottopicker::ConfigValidationError;

// TP-UI-002 part 3: a well-formed fixture config parses into a Config
// whose fields match the fixture values exactly.
TEST_CASE("Config parses a well-formed config file", "[UI-002]") {
    auto path = writeTempConfig("data_file=fixture_5draws.csv\n"
                                "top_n=10\n");

    Config config = Config::parse(path);

    CHECK(config.dataFile == path.parent_path() / "fixture_5draws.csv");
    CHECK(config.topN == 10);

    std::filesystem::remove(path);
}

TEST_CASE("Config ignores comments, blank lines, and unrecognized keys", "[UI-002]") {
    auto path = writeTempConfig("# a comment\n"
                                "\n"
                                "  \n"
                                "unrecognized_key=whatever\n"
                                "data_file=fixture_5draws.csv\n"
                                "top_n=100\n");

    Config config = Config::parse(path);

    CHECK(config.dataFile == path.parent_path() / "fixture_5draws.csv");
    CHECK(config.topN == 100);

    std::filesystem::remove(path);
}

// TP-UI-002 part 1: a fixture config missing `top_n` reports an error
// naming that key specifically.
TEST_CASE("Config reports a missing top_n key", "[UI-002]") {
    auto path = writeTempConfig("data_file=fixture_5draws.csv\n");

    REQUIRE_THROWS_AS(Config::parse(path), ConfigValidationError);
    try {
        Config::parse(path);
        FAIL("expected ConfigValidationError");
    } catch (const ConfigValidationError &e) {
        CHECK(std::string(e.what()).find("top_n") != std::string::npos);
    }

    std::filesystem::remove(path);
}

TEST_CASE("Config reports a missing data_file key", "[UI-002]") {
    auto path = writeTempConfig("top_n=10\n");

    try {
        Config::parse(path);
        FAIL("expected ConfigValidationError");
    } catch (const ConfigValidationError &e) {
        CHECK(std::string(e.what()).find("data_file") != std::string::npos);
    }

    std::filesystem::remove(path);
}

// TP-UI-002 part 2: top_n=0 and top_n=-5 both fail as "not a positive
// integer", naming top_n.
TEST_CASE("Config rejects a zero top_n", "[UI-002]") {
    auto path = writeTempConfig("data_file=fixture_5draws.csv\n"
                                "top_n=0\n");

    try {
        Config::parse(path);
        FAIL("expected ConfigValidationError");
    } catch (const ConfigValidationError &e) {
        const std::string message = e.what();
        CHECK(message.find("top_n") != std::string::npos);
        CHECK(message.find("positive integer") != std::string::npos);
    }

    std::filesystem::remove(path);
}

TEST_CASE("Config rejects a negative top_n", "[UI-002]") {
    auto path = writeTempConfig("data_file=fixture_5draws.csv\n"
                                "top_n=-5\n");

    try {
        Config::parse(path);
        FAIL("expected ConfigValidationError");
    } catch (const ConfigValidationError &e) {
        const std::string message = e.what();
        CHECK(message.find("top_n") != std::string::npos);
        CHECK(message.find("positive integer") != std::string::npos);
    }

    std::filesystem::remove(path);
}

TEST_CASE("Config rejects a non-numeric top_n", "[UI-002]") {
    auto path = writeTempConfig("data_file=fixture_5draws.csv\n"
                                "top_n=ten\n");

    REQUIRE_THROWS_AS(Config::parse(path), ConfigValidationError);

    std::filesystem::remove(path);
}

TEST_CASE("Config resolves a relative data_file against the config's own directory", "[UI-002]") {
    auto dir = std::filesystem::temp_directory_path() /
               ("lottopicker_config_test_dir_" + std::to_string(std::random_device{}()));
    std::filesystem::create_directory(dir);
    auto path = dir / "lotto.cfg";
    std::ofstream out(path);
    out << "data_file=subdir/data.csv\n"
        << "top_n=5\n";
    out.close();

    Config config = Config::parse(path);

    CHECK(config.dataFile == dir / "subdir" / "data.csv");

    std::filesystem::remove_all(dir);
}

TEST_CASE("Config throws ConfigFormatError for a missing config file", "[UI-002]") {
    auto path = std::filesystem::temp_directory_path() / "lottopicker_config_test_missing.cfg";
    std::filesystem::remove(path);

    REQUIRE_THROWS_AS(Config::parse(path), ConfigFormatError);
}
