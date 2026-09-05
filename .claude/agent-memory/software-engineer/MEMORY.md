# Software Engineer — memory

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

## Architecture patterns

- [LottoPicker scaffold](lottopicker_scaffold.md) — CMake layout, INTERFACE lib until first feature source, and the Windows-workflow hand-off note from Generate Code Base (issue #5).
- [DATA-IN-100 CSV ingestion](data_in_100_csv_ingestion.md) — CsvIngestor/DrawRecord design, the row-numbering convention I picked (not fixed in docs), fatal-vs-per-row error split (issue #7).
- [LottoPicker error handling](lottopicker_error_handling.md) — typed-exception pattern (UI-001, issue #6) to extend for future validation errors, plus a clang-format pointer-alignment gotcha.

## Platform-specific notes

## Reusable solutions

- [Inspection-type RTVM items](inspection_type_rtvm_items.md) — CORE-207 (issue #8): no branch/commit needed when the item's test method is Inspection and the artifact was already delivered elsewhere; just verify against the TP-* checklist and hand off.

## Coding standards
