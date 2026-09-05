#pragma once

#include <string>
#include <vector>

namespace lottopicker {

// One documented Florida Lotto rule-change boundary: the number-pool
// size in effect for every draw dated on or after `effectiveFrom`
// (inclusive), until the next boundary's `effectiveFrom` (exclusive).
struct EraBoundary {
    std::string effectiveFrom; // YYYY-MM-DD, inclusive lower bound.
    int poolSize = 0;
};

// Documented table of Florida Lotto rule-change dates (DATA-IN-101),
// ascending by `effectiveFrom`. Kept as one table in one place (not
// scattered assumptions) so a correction is a one-line change, not a
// design change, per docs/SDD.md's Algorithm Design section.
//
// ** CONFIRMED against real client data (docs/RTVM.md TP-DATA-IN-101
// part 2, issue #27): ** the boundary below was originally a *working
// hypothesis* of `{"1999-01-01", 53}`, sourced from a secondary
// lottery-statistics blog rather than the Florida Lottery's own
// archive. That has now been corrected using the client-supplied
// `data/florida_lotto_full.csv` (4018 records, 1988-05-07 …
// 2026-09-02; see `data/README.md` for full provenance/evidence):
// every draw from 1988-05-07 through 1999-10-23 contains only numbers
// 1–49, and every draw from 1999-10-27 onward contains numbers up to
// 53 — the actual boundary is ~10 months later than the original
// hypothesis. `1999-10-24` (the Florida Lottery's documented matrix-
// change date, falling in the unobserved gap between the last 6/49
// draw and the first 6/53 draw) is used as `effectiveFrom` below.
inline const std::vector<EraBoundary> &eraTable() {
    static const std::vector<EraBoundary> kEras = {
        // Florida Lotto's 1988 launch: 6 numbers drawn from a pool of 49.
        {"1988-01-01", 49},
        // Pool expanded to 6-from-53 on 1999-10-24, confirmed against
        // real draw history — see the comment above.
        {"1999-10-24", 53},
    };
    return kEras;
}

// Returns the number-pool size in effect for `date` (`YYYY-MM-DD`),
// per `eras` (ascending by `effectiveFrom`; defaults to the real
// documented `eraTable()`). Dates are compared lexicographically,
// which is correct for zero-padded ISO-8601 dates (see DrawRecord.h).
// A `date` earlier than `eras`'s first boundary still returns that
// boundary's pool size — the first entry is expected to be the game's
// own launch, so no valid draw date should ever precede it.
int poolSizeForDate(const std::string &date, const std::vector<EraBoundary> &eras = eraTable());

} // namespace lottopicker
