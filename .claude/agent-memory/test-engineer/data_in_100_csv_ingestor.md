---
name: data-in-100-csv-ingestor
description: How DATA-IN-100 (CsvIngestor) was verified, and the open row-numbering ambiguity in TP-DATA-IN-100.
metadata:
  type: project
---

Issue #7, verified 2026-09-05, PASS. `lottopicker::CsvIngestor::ingest()`
(`include/lottopicker/CsvIngestor.h` / `src/CsvIngestor.cpp`) is a pure
library call — no CLI wiring yet (`main.cpp` is still the Generate Code
Base stub), so verification is via `ctest --preset default` (10/10,
`tests/CsvIngestorTest.cpp`) plus an independent standalone harness
compiling `CsvIngestor.cpp` directly against fresh fixtures (not reusing
the SE's own fixture data) to cross-check TP-DATA-IN-100 parts 1 and 2.
Both agree.

**Open, not a failure ground:** `docs/RTVM.md` TP-DATA-IN-100's wording
("row 4 containing only 5 numbers") is ambiguous about whether "row 4"
means the 4th data row or the 4th physical file line. Software Engineer
picked: 1-based physical line number, header counts as row 1 (so header
row 1, first data row 2). This is documented in code comments
(`CsvIngestor.h`'s `RowError`) and in the unit test comments side by
side with the data-row numbering. TP-DATA-IN-100 itself already flags
this as "open detail, not yet fixed" — don't treat it as a fail; if
Systems Engineer settles it later, expect a corresponding one-line
change in `CsvIngestor::ingest` (row number computed in exactly one
place, per Software Engineer's comment on #7).

**Reusable pattern for future ingestion/parsing issues:** when the
feature under test is a library call with no CLI entry point yet, write
a throwaway standalone `.cpp` that `#include`s the header and compiles
directly against the `.cpp` (e.g.
`g++ -std=c++17 -I include verify.cpp src/CsvIngestor.cpp -o verify`)
rather than only re-running the SE's own unit test binary — gives an
independent check with fixtures the SE never saw. Clean up the temp dir
afterward; nothing from it should land in git status.
