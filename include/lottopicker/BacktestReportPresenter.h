#pragma once

#include <ostream>
#include <vector>

#include "lottopicker/BacktestEngine.h" // BacktestResult

namespace lottopicker {

// OUT-401: renders CORE-205/DATA-OUT-302's backtest report
// (`std::vector<BacktestResult>`, one entry per requested `--backtest`
// sample date -- see BacktestEngine.h's DATA-OUT-302 comment) as a
// human-readable console table, following the same pure-presentation
// pattern as OUT-400's RankedListPresenter: one header row followed by
// exactly `report.size()` data rows, in the same order given (no
// re-sorting -- `BacktestEngine::run` is called once per requested
// date by the caller, so the report's order is already the caller's
// requested date order). Pure presentation: takes an already-computed
// std::vector<BacktestResult> and any std::ostream, no dependency on
// BacktestEngine's computation itself -- unit-testable against a
// hand-built fixture and an std::ostringstream, matching TP-OUT-401's
// wording ("given a DATA-OUT-302 fixture for 3 years").
class BacktestReportPresenter {
public:
    // Writes one header row ("Sample Date | Actual Draw | Rank |
    // Containment (3/4/5/6 of 6)") followed by one data row per entry
    // of `report`, each column padded to the widest value in that
    // column (header included) and separated by " | " (same delimiter
    // convention as RankedListPresenter). The combined "Rank" column
    // renders `<rank>/<topN> (<percentile> pct)` when `entry.found` is
    // true, or the literal text "not found in top-N" when it is
    // false -- per docs/SDD.md's Interfaces & File Formats ("the
    // backtest report renders 'not found in top-N' as literal text in
    // the rank column, never a numeric placeholder like -1 or 0") and
    // TP-OUT-401 ("the 'not found in top-N' case rendered distinctly
    // from a numeric rank"). Containment is printed as the four
    // observed counts (levels 3/4/5/6 of 6) space-separated, in level
    // order.
    static void print(std::ostream &out, const std::vector<BacktestResult> &report);
};

} // namespace lottopicker
