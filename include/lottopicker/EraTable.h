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
// ** OPEN ITEM — not resolved with certainty (docs/SDD.md §Algorithm
// Design, docs/RTVM.md TP-DATA-IN-101 part 2): ** the 1999-01-01
// boundary below is a *working hypothesis*, sourced from a secondary
// lottery-statistics blog rather than the Florida Lottery's own
// archive (a JS-rendered SPA not reachable by curl-based research —
// see docs/research notes referenced from docs/SDD.md). This was meant
// to be cross-checked, during this feature's implementation, against
// the real client-supplied historical CSV's own observed number
// ranges per date range (the CSV is the authoritative source per
// SN-4). As of this implementation, no such file exists anywhere in
// this repository or pipeline — only a placeholder path for
// DELIV-900's small illustrative sample CSV, not the complete real
// draw history — so that confirmation could not be performed. See
// issue #10's hand-off comment for the escalation to Systems Engineer.
// Update the dates/pool sizes below directly once real data settles
// this; nothing downstream needs to change shape, only these values.
inline const std::vector<EraBoundary> &eraTable() {
    static const std::vector<EraBoundary> kEras = {
        // Florida Lotto's 1988 launch: 6 numbers drawn from a pool of 49.
        {"1988-01-01", 49},
        // Pool expanded to 6-from-53 in 1999 (working hypothesis, see
        // the comment above).
        {"1999-01-01", 53},
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
