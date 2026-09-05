---
name: data-in-101-era-tagging
description: EraTable/EraTagger design for DATA-IN-101 and the unresolved real-CSV cross-check gap (issue #10)
metadata:
  type: project
---

Issue #10 (branch `issue-10`) implemented DATA-IN-101:
`include/lottopicker/EraTable.h` (+ `src/EraTable.cpp`) holds one
documented, ascending `std::vector<EraBoundary>` (date + pool size) and
`poolSizeForDate()`; `include/lottopicker/EraTagger.h` (+
`src/EraTagger.cpp`) is a separate pipeline stage (per
[[data_in_100_csv_ingestion]]'s note that it would consume
`IngestResult::records` without re-validating) that tags each
already-ingested `DrawRecord` in place. `DrawRecord` gained a
`poolSize` field, default `0` meaning "untagged" — `CsvIngestor` never
sets it, keeping ingestion decoupled from era logic exactly as planned.

**Pattern reused from CsvIngestor:** both `EraTagger::tag()` and
`poolSizeForDate()` take the era table as a defaulted parameter
(default = the real `eraTable()`), the same shape as CsvIngestor's
`poolMin`/`poolMax` defaults — lets tests exercise the logic against a
synthetic boundary date without depending on real-world Florida Lotto
history, per TP-DATA-IN-101 part 1's own fixture description.

**Open item that could NOT be resolved (escalated to Systems
Engineer):** `docs/SDD.md`/`docs/RTVM.md` both instruct Software
Engineer to cross-check the 1999 6/49→6/53 boundary hypothesis against
"the real client-supplied CSV's own observed number ranges" during
implementation. No such file exists anywhere in this repo, in any
issue, or referenced from any doc — only a *placeholder path* for
DELIV-900's small illustrative sample CSV (10-20 synthetic rows, not
delivered yet either). This is a missing-input-artifact gap, not a
design ambiguity — flagged back to Systems Engineer via issue #10
rather than guessed at. If a future issue reports the real CSV has
arrived, correcting `eraTable()`'s dates/pool sizes is a one-line
change (that's the whole point of centralizing it there).

**How to apply:** if this recurs on a future project — an RTVM item
that gates "Verified" on cross-checking a real artifact that was never
actually delivered into the repo/pipeline — implement everything that
*is* fully specified (the design, the synthetic-fixture test), and
escalate specifically the missing artifact rather than blocking the
whole feature or silently shipping the unconfirmed hypothesis as if
confirmed.
