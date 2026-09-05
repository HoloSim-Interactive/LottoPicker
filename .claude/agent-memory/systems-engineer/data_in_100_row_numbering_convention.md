---
name: data-in-100-row-numbering-convention
description: Resolved row-numbering convention for DATA-IN-100 CSV ingestion error messages — check before touching TP-DATA-IN-100 or any future CSV row-reporting requirement
metadata:
  type: project
---

TP-DATA-IN-100 originally left ambiguous whether a reported error's
"row <n>" meant the nth data row or the nth physical file line.
Software Engineer picked, implemented, and documented in code
(`CsvIngestor::ingest`, computed in exactly one place): **1-based
physical file line number, header included** — header is row 1, first
data row is row 2. Test Engineer's independent fixture verification
(issue #7) confirmed this matches actual behavior. `docs/RTVM.md`'s
TP-DATA-IN-100 text now states this explicitly, closing the ambiguity
without any code change.

**Why:** matches what a user sees opening the CSV in a text
editor/spreadsheet — the more intuitive convention for a client-facing
error message, and cheaper to keep than to relitigate.

**How to apply:** any future requirement that reports row-specific
errors from a client-supplied file (not just DATA-IN-100/101) should
default to this same convention for consistency, unless a stakeholder
need says otherwise.
