---
name: out-401-backtest-report-presenter
description: BacktestReportPresenter design for OUT-401 (issue #23) -- console table for DATA-OUT-302, and the single-"Rank column" reading of the not-found-vs-numeric-rank requirement.
metadata:
  type: project
---

Issue #23 (branch `issue-23`) implemented OUT-401:
`BacktestReportPresenter` (`include/lottopicker/BacktestReportPresenter.h`
/ `src/BacktestReportPresenter.cpp`) renders CORE-205/DATA-OUT-302's
`std::vector<BacktestResult>` (see [[core_205_backtest_engine]],
[[data_out_302_backtest_report_structure]]) as a console table,
following [[out_400_ranked_list_presenter]]'s exact pattern: pure
presentation (`print(std::ostream&, const std::vector<BacktestResult>&)`,
no dependency on `BacktestEngine` itself), `" | "`-delimited columns
each padded to their own widest value, one header row + exactly one
data row per entry in the given order.

**Column design choice:** rank and percentile are combined into a
single "Rank" column (`"<rank>/<topN> (<pct> pct)"`, e.g. `"42/84
(51.19 pct)"`) rather than two separate columns, specifically because
`docs/SDD.md` says "the backtest report renders 'not found in top-N'
as literal text **in the rank column**" (singular) -- a single column
is the more literal reading, and it also just mirrors the ad hoc
format `main.cpp` was already printing before this issue landed.
Other columns: Sample Date, Actual Draw (space-separated numbers, same
style as `RankedListPresenter`'s combo column), Containment (the four
`observedContainment` counts space-separated, levels 3-6 of 6 in
order) -- `chanceExpectedContainment` is deliberately NOT printed
(no RTVM/SDD wording asks for it in the console report; it exists in
`BacktestResult` for potential future analysis/export, not display).

**Wired into `main()`:** the backtest loop now collects successful
`BacktestResult`s into a `std::vector` and calls
`BacktestReportPresenter::print` once at the end; a per-date
`BacktestError` (e.g. no draw recorded on that date) is still printed
inline as it's caught, since it never becomes a report row (there's no
`BacktestResult` to add to the vector for that date).

**Testing approach:** per [[out_400_ranked_list_presenter]]'s already-
established precedent, tests hand-build `BacktestResult` fixtures
directly (3 years, 2 found + 1 not-found) rather than running a real
`BacktestEngine::run`/`RankingEngine::rank` pass, matching TP-OUT-401's
own wording ("given a DATA-OUT-302 fixture for 3 years") and avoiding
[[core_203_ranking_engine]]'s ~43-55s full-pool-53 scan cost per
sample date in the test suite. (A manual smoke-test run of the real
`lottopicker` binary in `--backtest` mode against a tiny synthetic CSV
was attempted for this issue but is slow for the same reason --
useful to remember if a future manual CLI smoke-check on `--backtest`
seems to hang: it isn't hung, a real pool-53 scan just takes tens of
seconds per requested date.)
