---
name: ui-003-backtest-flag-parsing
description: CliArgs --backtest flag parsing design and main.cpp mode-branch stub for UI-003 (issue #13)
metadata:
  type: project
---

Issue #13 (branch `issue-13`) implemented UI-003's CLI surface only —
flag parsing, not the backtest engine itself (that's CORE-205, a
later, dependent issue). `CliArgs` (see [[ui_002_config_parsing]] for
the sibling UI-002 pattern) gained:

- `backtestDates` (`std::vector<std::string>`, empty = normal ranking
  run) and `isBacktest()` accessor.
- `CliArgs::parse` now accepts exactly 1 or exactly 3 positional argv
  tokens — `<config>` alone, or `<config> --backtest <dates>` — per
  `docs/SDD.md`'s fixed syntax (flag and value are always two argv
  tokens, `--backtest=x` is not supported since SDD didn't specify
  it). Any other shape, an unrecognized second token, or a
  comma-separated date that fails a coarse `YYYY-MM-DD` shape check
  all throw `CliUsageError` — no new exception type needed.

**Decision:** duplicated a small `looksLikeIsoDate` helper into
`CliArgs.cpp` rather than extracting `CsvIngestor.cpp`'s copy into a
shared header. Same logic, but the two call sites belong to different
features (UI-003 CLI parsing vs. DATA-IN-100 CSV ingestion) and it's a
few lines — didn't want to couple two otherwise-independent modules
for issue-#13's scope. Revisit if a third caller ever needs it.

**main() stub:** since CSV ingestion still isn't wired into `main()`
at all (flagged in [[ui_002_config_parsing]]), the backtest branch
can't do real work yet — it just prints one placeholder line per
requested date, clearly commented as standing in for CORE-205/
DATA-OUT-302 until that issue lands. Don't mistake this for real
backtest output when reviewing later diffs.
