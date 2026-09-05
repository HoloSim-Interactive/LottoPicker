---
name: data-in-100-csv-ingestion
description: CsvIngestor/DrawRecord design and the row-numbering convention chosen for DATA-IN-100
metadata:
  type: project
---

Issue #7 (branch `issue-7`) implemented DATA-IN-100: `include/lottopicker/DrawRecord.h`
(date string + sorted 6-number array) and `include/lottopicker/CsvIngestor.h` /
`src/CsvIngestor.cpp`. This was the first real source file, so
`lottopicker_lib` in `src/CMakeLists.txt` moved from `INTERFACE` to
`STATIC` per [[lottopicker_scaffold]]'s note to do this the moment one
lands.

**Row-numbering convention (not fixed anywhere in docs/SDD.md or
docs/RTVM.md — I picked it):** `row <n>` in a `RowError` is the
1-based *physical line number in the file*, header included (header =
row 1, first data row = row 2) — matches what a user sees opening the
file in a text editor/spreadsheet. `docs/RTVM.md`'s TP-DATA-IN-100
wording ("row 4 containing only 5 numbers") is ambiguous about whether
it means the 4th data row or the 4th file line; Test Engineer needs to
write the actual test fixture consistent with this convention (or flag
it back if a different convention is wanted — it's cheap to change,
row number is computed in exactly one place in `CsvIngestor::ingest`).

**Fatal vs. per-row error split:** whole-file structural problems
(file won't open, header line doesn't match
`date,n1,n2,n3,n4,n5,n6` exactly) throw `CsvFormatError`, caught at
main()'s boundary per the SDD's Coding Standards. Everything else
(wrong field count, non-numeric field, out-of-pool-range number,
duplicate number within a row, malformed date) is collected into
`IngestResult::errors` and does NOT throw — DATA-IN-100 explicitly
requires partial-file tolerance (remaining rows still ingest), which
an exception-per-row model can't give you if you want the file scan to
continue.

**Pool range for row validation:** validates against the *widest*
documented pool (1-53, `kPoolMin`/`kCurrentPoolMax` in CsvIngestor.h),
not an era-specific range — DATA-IN-101 (era tagging, not yet
implemented) is a separate downstream stage per the SDD pipeline
diagram; ingestion only needs to catch numbers that are invalid under
any era.

**How to apply:** when DATA-IN-101 lands, it consumes
`IngestResult::records` and tags each with an era/pool size — it
should NOT need to re-validate or re-parse anything CsvIngestor already
validated.
