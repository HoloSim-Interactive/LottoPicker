---
name: out-401-backtest-report-presenter
description: OUT-401 BacktestReportPresenter pass (issue #23) — verification approach for the backtest console formatter, mirrors OUT-400's pattern.
metadata:
  type: project
---

Issue #23, 2026-09-05, passed on first hand-off (no fail cycle).

**What it is:** `BacktestReportPresenter::print(ostream&, vector<BacktestResult>)`
— pure formatting like OUT-400's `RankedListPresenter`. Header (`Sample Date |
Actual Draw | Rank | Containment (3/4/5/6 of 6)`) + one row per entry. Rank
column renders `<rank>/<topN> (<pct> pct)` when `found`, else the literal
`"not found in top-N"` — never a numeric placeholder, per docs/SDD.md.

**Verification:** standard build/ctest sufficed (88/88, 776 assertions,
matching SE's stated counts exactly this time — no off-by-one like
[[data_out_302_backtest_report_structure]]). 4 new `[OUT-401]` tests map
directly onto TP-OUT-401's fixture wording. Also ran the real CLI
`--backtest` path against a synthetic 5-draw fixture — confirms a
per-date `BacktestError` (e.g. no draws before that sample date) is
printed inline and correctly *excluded* from the report table (not
faked as a row), and the remaining rows render with proper column
alignment and the literal not-found text. CSV header must be
`date,n1..n6` (not `draw_date`) — matches [[data_out_302_backtest_report_structure]]'s
config-format gotcha (`key = value` lines, not JSON).

No new gotchas. Same low-risk profile as OUT-400 — pure presentation
layer over an already-tested engine (CORE-205/DATA-OUT-302).

**Post-merge trunk regression (same issue #23, main @ 46342d6):** PASS,
88/88, 776 assertions, matches CI/CD's merge report exactly. Only
clang-format nit is the pre-existing [[version_h_clang_format_nit]];
all other files clean.
