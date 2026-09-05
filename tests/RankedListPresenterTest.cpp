#include "lottopicker/RankedListPresenter.h"

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include <string>
#include <vector>

using lottopicker::RankedCombo;
using lottopicker::RankedListPresenter;

namespace {

// TP-OUT-400's own fixture size (5 entries) -- hand-built rather than
// run through RankingEngine, since OUT-400 is pure presentation and
// the test procedure explicitly frames it as "given a DATA-OUT-300
// fixture", not "given a fresh ranking run".
std::vector<RankedCombo> fiveEntryFixture() {
    return {
        RankedCombo{1, {1, 2, 3, 4, 5, 6}, 45.0},  RankedCombo{2, {2, 3, 4, 5, 6, 7}, 44.5},
        RankedCombo{3, {3, 4, 5, 6, 7, 8}, 40.25}, RankedCombo{4, {4, 5, 6, 7, 8, 9}, 12.0},
        RankedCombo{5, {5, 6, 7, 8, 9, 10}, 1.5},
    };
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

// TP-OUT-400: "Given a DATA-OUT-300 fixture with 5 entries. Expected:
// printed output shows exactly 5 rows in the same order, with a
// header row and clearly delimited columns."
TEST_CASE("RankedListPresenter prints a header row plus exactly one row per entry, in order",
          "[OUT-400]") {
    const std::vector<RankedCombo> ranked = fiveEntryFixture();

    std::ostringstream out;
    RankedListPresenter::print(out, ranked);

    const std::vector<std::string> lines = splitLines(out.str());
    REQUIRE(lines.size() == ranked.size() + 1); // header + exactly 5 data rows

    // Header row names the three columns.
    CHECK(lines[0].find("Rank") != std::string::npos);
    CHECK(lines[0].find("Combination") != std::string::npos);
    CHECK(lines[0].find("Score") != std::string::npos);

    // Data rows, in the same order as the input, each carrying its own
    // rank number and combination numbers.
    for (std::size_t i = 0; i < ranked.size(); ++i) {
        const std::string &line = lines[i + 1];
        CHECK(line.find(std::to_string(ranked[i].rank)) != std::string::npos);
        for (int number : ranked[i].combo) {
            CHECK(line.find(std::to_string(number)) != std::string::npos);
        }
    }
}

// "Clearly delimited columns": every row (header included) uses the
// same " | " column separator, so columns are unambiguous regardless
// of how wide any individual value is.
TEST_CASE("RankedListPresenter delimits columns with a consistent separator", "[OUT-400]") {
    std::ostringstream out;
    RankedListPresenter::print(out, fiveEntryFixture());

    const std::vector<std::string> lines = splitLines(out.str());
    for (const std::string &line : lines) {
        const std::size_t firstBar = line.find('|');
        const std::size_t secondBar = line.find('|', firstBar + 1);
        REQUIRE(firstBar != std::string::npos);
        REQUIRE(secondBar != std::string::npos);
        CHECK(secondBar != firstBar); // exactly two separators -> three columns
    }
}

// An empty ranked list (topN produced zero results) still prints a
// valid header row and no data rows, rather than an empty/garbled
// table.
TEST_CASE("RankedListPresenter prints only the header row for an empty ranked list", "[OUT-400]") {
    std::ostringstream out;
    RankedListPresenter::print(out, {});

    const std::vector<std::string> lines = splitLines(out.str());
    REQUIRE(lines.size() == 1);
    CHECK(lines[0].find("Rank") != std::string::npos);
}
