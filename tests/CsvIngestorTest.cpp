#include "lottopicker/CsvIngestor.h"

#include <catch2/catch_test_macros.hpp>
#include <fstream>
#include <random>

#include "lottopicker/DrawRecord.h"

namespace {

// Writes `content` to a uniquely-named temp file and returns its path.
// Each TEST_CASE gets its own file so tests can run in parallel without
// clobbering each other.
std::filesystem::path writeTempCsv(const std::string &content) {
    static std::mt19937_64 rng{std::random_device{}()};
    auto path = std::filesystem::temp_directory_path() /
                ("lottopicker_csv_ingestor_test_" + std::to_string(rng()) + ".csv");
    std::ofstream out(path);
    out << content;
    out.close();
    return path;
}

} // namespace

using lottopicker::CsvFormatError;
using lottopicker::CsvIngestor;
using lottopicker::DrawRecord;

// TP-DATA-IN-100 part 1: a fixture CSV with 10 well-formed rows ingests
// as exactly 10 in-memory draw records, each matching the fixture's
// date and numbers (sorted ascending regardless of file order, per
// docs/SDD.md).
TEST_CASE("CsvIngestor ingests all rows of a well-formed CSV", "[DATA-IN-100]") {
    const std::string csv = "date,n1,n2,n3,n4,n5,n6\n"
                            "2020-01-01,5,12,20,33,40,53\n"
                            "2020-01-05,1,2,3,4,5,6\n"
                            "2020-01-08,10,9,8,7,6,5\n" // out of order on disk, sorted on ingest
                            "2020-01-12,53,1,2,3,4,5\n"
                            "2020-01-15,7,14,21,28,35,42\n"
                            "2020-01-19,2,4,6,8,10,12\n"
                            "2020-01-22,11,22,33,44,53,1\n"
                            "2020-01-26,3,6,9,12,15,18\n"
                            "2020-01-29,13,17,19,23,29,31\n"
                            "2020-02-02,6,5,4,3,2,1\n";
    auto path = writeTempCsv(csv);

    auto result = CsvIngestor::ingest(path);

    REQUIRE(result.errors.empty());
    REQUIRE(result.records.size() == 10);
    CHECK(result.records[0] == DrawRecord{"2020-01-01", {5, 12, 20, 33, 40, 53}});
    CHECK(result.records[2] == DrawRecord{"2020-01-08", {5, 6, 7, 8, 9, 10}});
    CHECK(result.records[9] == DrawRecord{"2020-02-02", {1, 2, 3, 4, 5, 6}});

    std::filesystem::remove(path);
}

// TP-DATA-IN-100 part 2: malformed rows are reported as row-specific
// errors, not a silent skip or a whole-run crash, and the remaining
// well-formed rows still ingest (partial-file tolerance).
//
// Row numbering convention: `row <n>` is the 1-based physical line
// number in the file, header included (so the header is row 1, the
// first data row is row 2) — matches what a user sees opening the file
// in a text editor or spreadsheet. Below, the malformed rows are the
// 4th and 7th *data* rows, i.e. file lines 5 and 8.
TEST_CASE("CsvIngestor reports malformed rows without aborting the rest", "[DATA-IN-100]") {
    const std::string csv =
        "date,n1,n2,n3,n4,n5,n6\n"
        "2020-01-01,5,12,20,33,40,53\n"  // data row 1 / file line 2 — valid
        "2020-01-05,1,2,3,4,5,6\n"       // data row 2 / file line 3 — valid
        "2020-01-08,10,9,8,7,6,5\n"      // data row 3 / file line 4 — valid
        "2020-01-12,1,2,3,4,5\n"         // data row 4 / file line 5 — only 5 numbers
        "2020-01-15,7,14,21,28,35,42\n"  // data row 5 / file line 6 — valid
        "2020-01-19,2,4,6,8,10,12\n"     // data row 6 / file line 7 — valid
        "2020-01-22,11,22,33,44,53,60\n" // data row 7 / file line 8 — 60 out of pool range
        "2020-01-26,3,6,9,12,15,18\n"    // data row 8 / file line 9 — valid
        "2020-01-29,13,17,19,23,29,31\n" // data row 9 / file line 10 — valid
        "2020-02-02,6,5,4,3,2,1\n";      // data row 10 / file line 11 — valid
    auto path = writeTempCsv(csv);

    auto result = CsvIngestor::ingest(path);

    REQUIRE(result.records.size() == 8);
    REQUIRE(result.errors.size() == 2);

    CHECK(result.errors[0].row == 5);
    CHECK(result.errors[0].message == "expected 6 numbers, found 5");
    CHECK(result.errors[1].row == 8);
    CHECK(result.errors[1].message == "number 60 outside valid pool range (1-53)");

    std::filesystem::remove(path);
}

TEST_CASE("CsvIngestor reports a non-numeric field as its own row error", "[DATA-IN-100]") {
    const std::string csv = "date,n1,n2,n3,n4,n5,n6\n"
                            "2020-01-01,5,12,twenty,33,40,53\n";
    auto path = writeTempCsv(csv);

    auto result = CsvIngestor::ingest(path);

    REQUIRE(result.records.empty());
    REQUIRE(result.errors.size() == 1);
    CHECK(result.errors[0].row == 2);
    CHECK(result.errors[0].message == "invalid number 'twenty'");

    std::filesystem::remove(path);
}

TEST_CASE("CsvIngestor rejects a number below the pool minimum", "[DATA-IN-100]") {
    const std::string csv = "date,n1,n2,n3,n4,n5,n6\n"
                            "2020-01-01,0,12,20,33,40,53\n";
    auto path = writeTempCsv(csv);

    auto result = CsvIngestor::ingest(path);

    REQUIRE(result.records.empty());
    REQUIRE(result.errors.size() == 1);
    CHECK(result.errors[0].row == 2);
    CHECK(result.errors[0].message == "number 0 outside valid pool range (1-53)");

    std::filesystem::remove(path);
}

TEST_CASE("CsvIngestor reports a duplicate number within a row", "[DATA-IN-100]") {
    const std::string csv = "date,n1,n2,n3,n4,n5,n6\n"
                            "2020-01-01,5,12,20,33,40,5\n";
    auto path = writeTempCsv(csv);

    auto result = CsvIngestor::ingest(path);

    REQUIRE(result.records.empty());
    REQUIRE(result.errors.size() == 1);
    CHECK(result.errors[0].row == 2);
    CHECK(result.errors[0].message == "duplicate number 5 within row");

    std::filesystem::remove(path);
}

TEST_CASE("CsvIngestor reports a malformed date", "[DATA-IN-100]") {
    const std::string csv = "date,n1,n2,n3,n4,n5,n6\n"
                            "01/01/2020,5,12,20,33,40,53\n";
    auto path = writeTempCsv(csv);

    auto result = CsvIngestor::ingest(path);

    REQUIRE(result.records.empty());
    REQUIRE(result.errors.size() == 1);
    CHECK(result.errors[0].row == 2);
    CHECK(result.errors[0].message == "invalid date '01/01/2020', expected YYYY-MM-DD");

    std::filesystem::remove(path);
}

TEST_CASE("CsvIngestor skips trailing blank lines without an error", "[DATA-IN-100]") {
    const std::string csv = "date,n1,n2,n3,n4,n5,n6\n"
                            "2020-01-01,5,12,20,33,40,53\n"
                            "\n";
    auto path = writeTempCsv(csv);

    auto result = CsvIngestor::ingest(path);

    REQUIRE(result.errors.empty());
    REQUIRE(result.records.size() == 1);

    std::filesystem::remove(path);
}

TEST_CASE("CsvIngestor throws CsvFormatError for a missing file", "[DATA-IN-100]") {
    auto path =
        std::filesystem::temp_directory_path() / "lottopicker_csv_ingestor_test_missing.csv";
    std::filesystem::remove(path);

    REQUIRE_THROWS_AS(CsvIngestor::ingest(path), CsvFormatError);
}

TEST_CASE("CsvIngestor throws CsvFormatError for a wrong header", "[DATA-IN-100]") {
    const std::string csv = "date,a,b,c,d,e,f\n"
                            "2020-01-01,5,12,20,33,40,53\n";
    auto path = writeTempCsv(csv);

    REQUIRE_THROWS_AS(CsvIngestor::ingest(path), CsvFormatError);

    std::filesystem::remove(path);
}
