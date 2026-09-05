# Test Engineer — memory

**This file is an index, not a store.** It is loaded on every run you
ever do, so anything verbose here is re-read on every future hand-off
for the rest of the project. Keep each entry to one line: a link and a
one-sentence summary. Put the actual detail in its own file in this
folder.

    - [Short title](descriptive_slug.md) — one sentence on what it is.

A genuinely one-line fact can stay a plain line with no file of its
own. Split a lesson out when it needs a reproduction, a command
sequence, or real reasoning to be useful later. See "Memory structure"
in `.github/AGENT_LABELS.md`.

## Test harness notes

- [LottoPicker Generate Code Base](lottopicker_generate_code_base.md) — build/test verification commands, no-RTVM-item issues, and how Inspection-type RTVM items (checklist-only, no build) still route through the normal pass hand-off.
- [DATA-IN-100 CsvIngestor](data_in_100_csv_ingestor.md) — verified via standalone harness (no CLI yet); open row-numbering ambiguity in TP-DATA-IN-100 is pre-flagged, not a fail ground.
- [RTVM ahead of branch on main](rtvm_ahead_of_branch_on_main.md) — docs/RTVM.md edits can land on main mid-issue before the code merges; check main's copy, not just the branch's, before handing off for RTVM update.
- [Version.h clang-format nit](version_h_clang_format_nit.md) — pre-existing since scaffolding, not caused by any feature branch; don't count against whatever you're testing.

## Platform-specific test considerations

## Recurring failure patterns

## Flaky tests
