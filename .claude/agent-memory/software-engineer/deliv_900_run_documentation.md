---
name: deliv-900-run-documentation
description: DELIV-900 (issue #25) — root README.md fill-in and the examples/ sample CSV+config, including the real ~1min-per-run timing gotcha.
metadata:
  type: project
---

Issue #25 (branch `issue-25`) filled in root `README.md`'s
Config-file-format/Launch-command/Sample-output sections (previously
`TODO (DELIV-900)` placeholders from Generate Code Base) and added the
two files `examples/README.md` promised: `examples/sample_history.csv`
(15 synthetic 2024-dated draws, current 6/53 era) and
`examples/sample_config.cfg` (`top_n=10`, kept small so the console
table stays short).

**Every console-output block in the README is a real captured run**,
not hand-typed — built the project (`cmake --preset default && cmake
--build --preset default`), ran the actual binary against the
committed example twice (once for `model: rebuilt`, once more for
`model: reused`) and once with `--backtest 2024-03-13`, pasted the
literal stdout. Re-ran the exact documented command sequence from a
clean model state afterward and diffed against the captured text to
confirm it reproduces exactly, since TP-DELIV-900 explicitly checks
"output matching the documented example."

**Timing gotcha worth knowing before writing example docs for this
project again:** [[core_203_ranking_engine]]'s ~43-55s full-scan cost
applies to *every* invocation regardless of how tiny the sample CSV
is — `RankingEngine::rank`'s default `poolMax` is always the *current*
6/53 pool (`kCurrentPoolMax`), not something derived from the sample
data's own date range, and a `--backtest` run does one such full scan
*per requested sample date* too (`BacktestEngine::run` looks up the
era-appropriate `poolMax` for the sample date, which is still 53 for
any 2024 date). Documented this explicitly in the README's "Launch
command" section ("Heads up on timing... about a minute") so a first-
time reader doesn't mistake the wait for a hang — this is exactly the
kind of thing TP-DELIV-900's "no undocumented steps" wording is meant
to catch.

**Added `*.model` to `.gitignore`.** A documented run leaves
`examples/sample_history.csv.model` on disk (`CORE-204`'s persisted
model artifact, next to the CSV since `ModelStore::defaultModelPath`
appends `.model` to `dataFile`'s path) — this is derived/regenerable
data, deliberately not committed, and nothing had ignored it before
now since no example run had ever been committed to this repo.

**Picked `--backtest 2024-03-13` deliberately for the shipped
example** because it's an ISO date that (a) exists exactly in
`sample_history.csv`, (b) has earlier rows to train on, and (c)
happens to produce the "not found in top-N" (`OUT-401`) case rather
than a found one with this tiny 15-row/`top_n=10` fixture — mentioned
this is expected/intentional in the README so a reader doesn't mistake
it for a bug in their own run.

**Out of scope, left for a human/real data:** `DELIV-900`'s RTVM row
also tracks the deferred `DATA-IN-101` part 2 check (confirm/correct
the 1999 era boundary against the client's real historical CSV) — see
[[data_in_101_era_tagging]]. Still blocked on the client actually
supplying real data; this issue's synthetic `examples/sample_history.csv`
does not and should not stand in for it. Not re-escalated here since
it's already tracked from issue #10 and doesn't gate this issue's own
completion per the RTVM row's own wording ("not a gate on this item").
