#include "lottopicker/BacktestReportPresenter.h"

#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <string>
#include <vector>

#include "lottopicker/BacktestEngine.h"

using lottopicker::BacktestReportPresenter;
using lottopicker::BacktestResult;

namespace {

// TP-OUT-401's own fixture size (3 sampled years) -- hand-built rather
// than run through BacktestEngine, since OUT-401 is pure presentation
// and the test procedure explicitly frames it as "given a
// DATA-OUT-302 fixture for 3 years" (same rationale RankedListPresenterTest
// uses for OUT-400 -- see docs/SDD.md "Console output"). Two of the
// three rows are "found" at different ranks; the third is the "not
// found in top-N" case TP-OUT-401 requires to render distinctly.
std::vector<BacktestResult> threeYearFixture() {
    BacktestResult year1;
    year1.sampleDate = "2017-01-01";
    year1.actualDraw = {1, 2, 3, 4, 5, 6};
    year1.poolMax = 9;
    year1.topN = 84;
    year1.found = true;
    year1.rank = 1;
    year1.percentile = 100.0;
    year1.observedContainment = {20, 45, 18, 1};

    BacktestResult year2;
    year2.sampleDate = "2018-01-01";
    year2.actualDraw = {2, 3, 4, 5, 6, 7};
    year2.poolMax = 9;
    year2.topN = 84;
    year2.found = true;
    year2.rank = 42;
    year2.percentile = 51.19;
    year2.observedContainment = {20, 45, 18, 1};

    BacktestResult year3;
    year3.sampleDate = "2019-01-01";
    year3.actualDraw = {7, 8, 9, 10, 11, 12};
    year3.poolMax = 12;
    year3.topN = 1;
    year3.found = false; // TP-CORE-205 part 2's "not found" case
    year3.observedContainment = {0, 0, 0, 0};

    return {year1, year2, year3};
}

std::vector<std::string> splitLines(const std::string &text) {
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    return lines;
}

} // namespace

// TP-OUT-401: "Given a DATA-OUT-302 fixture for 3 years. Expected:
// printed output shows exactly one correctly formatted row per year,
// with the 'not found in top-N' case rendered distinctly from a
// numeric rank."
TEST_CASE("BacktestReportPresenter prints a header row plus exactly one row per sampled year",
          "[OUT-401]") {
    const std::vector<BacktestResult> report = threeYearFixture();

    std::ostringstream out;
    BacktestReportPresenter::print(out, report);

    const std::vector<std::string> lines = splitLines(out.str());
    REQUIRE(lines.size() == report.size() + 1); // header + exactly 3 data rows

    CHECK(lines[0].find("Sample Date") != std::string::npos);
    CHECK(lines[0].find("Actual Draw") != std::string::npos);
    CHECK(lines[0].find("Rank") != std::string::npos);
    CHECK(lines[0].find("Containment") != std::string::npos);

    for (std::size_t i = 0; i < report.size(); ++i) {
        const std::string &line = lines[i + 1];
        CHECK(line.find(report[i].sampleDate) != std::string::npos);
        for (int number : report[i].actualDraw) {
            CHECK(line.find(std::to_string(number)) != std::string::npos);
        }
    }
}

// The found rows carry a genuine numeric rank; the not-found row must
// never render as a numeric placeholder like -1 or 0 (docs/SDD.md
// Interfaces & File Formats) -- it renders the literal text instead,
// and that text must not appear on the rows that were actually found.
TEST_CASE("BacktestReportPresenter renders \"not found in top-N\" distinctly from a numeric rank",
          "[OUT-401]") {
    const std::vector<BacktestResult> report = threeYearFixture();

    std::ostringstream out;
    BacktestReportPresenter::print(out, report);
    const std::vector<std::string> lines = splitLines(out.str());
    REQUIRE(lines.size() == 4);

    // Found rows (year 1, year 2): a numeric rank/topN is present, and
    // the "not found" literal text is absent.
    CHECK(lines[1].find("1/84") != std::string::npos);
    CHECK(lines[1].find("not found in top-N") == std::string::npos);
    CHECK(lines[2].find("42/84") != std::string::npos);
    CHECK(lines[2].find("not found in top-N") == std::string::npos);

    // Not-found row (year 3): the literal text is present, and no
    // numeric placeholder (-1 or bare 0 as a rank) is substituted.
    CHECK(lines[3].find("not found in top-N") != std::string::npos);
    CHECK(lines[3].find("-1") == std::string::npos);
}

// "Clearly delimited columns": every row (header included) uses the
// same " | " separator convention as OUT-400's RankedListPresenter, so
// columns are unambiguous regardless of how wide any value is.
TEST_CASE("BacktestReportPresenter delimits columns with a consistent separator", "[OUT-401]") {
    std::ostringstream out;
    BacktestReportPresenter::print(out, threeYearFixture());

    const std::vector<std::string> lines = splitLines(out.str());
    for (const std::string &line : lines) {
        const std::size_t firstBar = line.find('|');
        const std::size_t secondBar = line.find('|', firstBar + 1);
        const std::size_t thirdBar = line.find('|', secondBar + 1);
        REQUIRE(firstBar != std::string::npos);
        REQUIRE(secondBar != std::string::npos);
        REQUIRE(thirdBar != std::string::npos);
        CHECK(secondBar != firstBar);
        CHECK(thirdBar != secondBar);
    }
}

// An empty report (e.g. every requested sample date errored out before
// producing a BacktestResult) still prints a valid header row and no
// data rows, rather than an empty/garbled table.
TEST_CASE("BacktestReportPresenter prints only the header row for an empty report", "[OUT-401]") {
    std::ostringstream out;
    BacktestReportPresenter::print(out, {});

    const std::vector<std::string> lines = splitLines(out.str());
    REQUIRE(lines.size() == 1);
    CHECK(lines[0].find("Sample Date") != std::string::npos);
}
