#include "lottopicker/EraTagger.h"

#include <catch2/catch_test_macros.hpp>

#include "lottopicker/DrawRecord.h"
#include "lottopicker/EraTable.h"

using lottopicker::DrawRecord;
using lottopicker::EraBoundary;
using lottopicker::EraTagger;

// TP-DATA-IN-101 part 1: fixture history spanning a synthetic
// rule-change date -- draws dated before 2001-01-01 belong to era A
// (pool 49), on/after belong to era B (pool 53). A hand-picked
// synthetic table (not the real documented one) is used so this test
// doesn't depend on real-world Florida Lotto history ever changing --
// see EraTagger.h's `eras` parameter.
TEST_CASE("EraTagger tags each record with its era's pool size", "[DATA-IN-101]") {
    const std::vector<EraBoundary> syntheticEras = {
        {"1970-01-01", 49},
        {"2001-01-01", 53},
    };

    std::vector<DrawRecord> records = {
        DrawRecord{"1999-06-15", {1, 2, 3, 4, 5, 6}},  // era A
        DrawRecord{"2000-12-31", {1, 2, 3, 4, 5, 6}},  // era A, day before boundary
        DrawRecord{"2001-01-01", {1, 2, 3, 4, 5, 53}}, // era B, boundary date itself
        DrawRecord{"2010-03-01", {1, 2, 3, 4, 5, 53}}, // era B
    };

    EraTagger::tag(records, syntheticEras);

    CHECK(records[0].poolSize == 49);
    CHECK(records[1].poolSize == 49);
    CHECK(records[2].poolSize == 53);
    CHECK(records[3].poolSize == 53);
}

// A record dated before the earliest documented boundary still gets a
// pool size (the first table entry's), never left untagged (0).
TEST_CASE("EraTagger falls back to the earliest era for a date before any boundary",
          "[DATA-IN-101]") {
    const std::vector<EraBoundary> syntheticEras = {
        {"1988-01-01", 49},
        {"1999-01-01", 53},
    };

    std::vector<DrawRecord> records = {DrawRecord{"1950-01-01", {1, 2, 3, 4, 5, 6}}};

    EraTagger::tag(records, syntheticEras);

    CHECK(records[0].poolSize == 49);
}

// Tagging with no explicit table uses the real documented one
// (EraTable.h's eraTable()), exercised here against its two known
// boundaries so a future correction to that table is caught by this
// test failing, not silently. The 1999-10-24 boundary itself is the
// real-data-confirmed value (issue #27, see data/README.md), not a
// hypothesis.
TEST_CASE("EraTagger defaults to the real documented era table", "[DATA-IN-101]") {
    std::vector<DrawRecord> records = {
        DrawRecord{"1988-01-01", {1, 2, 3, 4, 5, 6}},
        DrawRecord{"1999-10-23", {1, 2, 3, 4, 5, 6}},
        DrawRecord{"1999-10-24", {1, 2, 3, 4, 5, 53}},
    };

    EraTagger::tag(records);

    CHECK(records[0].poolSize == 49);
    CHECK(records[1].poolSize == 49);
    CHECK(records[2].poolSize == 53);
}
