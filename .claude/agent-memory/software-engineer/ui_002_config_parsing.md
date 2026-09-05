---
name: ui-002-config-parsing
description: Config::parse design and validation-ordering choice for UI-002 (issue #9)
metadata:
  type: project
---

Issue #9 (branch `issue-9`) implemented UI-002:
`include/lottopicker/Config.h` / `src/Config.cpp`, following the same
typed-exception pattern as [[lottopicker_error_handling]] — added
`ConfigFormatError` (whole-file open/read failure) and
`ConfigValidationError` (missing key or invalid `top_n`, message names
the specific key) to `Errors.h`, both deriving from `LottoPickerError`
so `main()`'s existing single catch block handles them with no new
wiring needed there beyond calling `Config::parse` right after
`CliArgs::parse`.

**Gaps not fixed by docs, decisions I made:**
- **Key-check order:** `data_file` is checked for presence before
  `top_n`. Not specified anywhere; matters only when *both* are
  missing (which key gets named). TP-UI-002's fixtures only test one
  missing key at a time, so this didn't block anything, but flag it if
  Test Engineer's fixture ever has both missing and expects a specific
  one named.
- **Lines with no `=`:** treated as ignorable (skipped silently), same
  as an unrecognized key — RTVM only defines "missing key" and
  "invalid top_n" as validation errors, not "malformed line", so
  treating a stray line as forward-compatible noise seemed more
  consistent with the "unrecognized keys ignored" rule than inventing
  a new error case.
- **No existence check on `data_file` itself** — UI-002 is scoped to
  the config's own two keys; whether the CSV it points to actually
  exists is DATA-IN-100's `CsvIngestor::ingest`'s problem (it already
  throws `CsvFormatError` on open failure), not Config's.

**Pre-existing inconsistency noticed, not touched:** `CsvFormatError`
(DATA-IN-100) derives from `std::runtime_error` directly, not
`LottoPickerError` — harmless today only because `CsvIngestor::ingest`
isn't called from `main()` yet. Whichever issue first wires ingestion
into `main()` needs to either catch `CsvFormatError` separately or
(cleaner) reparent it onto `LottoPickerError` — flag this then if it's
not already fixed.
