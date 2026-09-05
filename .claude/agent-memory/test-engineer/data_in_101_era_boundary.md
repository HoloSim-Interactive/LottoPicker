---
name: data-in-101-era-boundary
description: How TP-DATA-IN-101 part 2 (real-CSV boundary cross-check, deferred since issue #10) was finally verified in issue #27.
metadata:
  type: project
---

Issue #27, verified 2026-09-05, PASS. `EraTable.h::eraTable()`'s second
boundary corrected from `{"1999-01-01", 53}` to `{"1999-10-24", 53}`.

**Independent verification, not just trusting SE's/README's claim:**
ran my own `awk` pass directly over `data/florida_lotto_full.csv`
(4018 rows) computing max(n1..n6) per row, sorted by date, restricted
to `draw_type==LOTTO` (main draws only, excluding Double Play and
XTRA-code rows). Confirmed independently: max value stays ≤49 through
1999-10-23, first draw with a value >49 is 1999-10-27 (max 53), zero
draws >49 before 1999-10-24. Matches both the issue's evidence and
`data/README.md`'s own analysis — two independently-computed sources
agree, not one trusting the other.

`tests/EraTaggerTest.cpp`'s `[DATA-IN-101]`-tagged case now uses fixture
dates 1999-10-23 (→49) / 1999-10-24 (→53) and passes, 8 assertions/3
cases. Full suite 88/88, `clang-format --dry-run --Werror` clean on
both changed files.

**Scope note:** real CLI wiring to `data/florida_lotto_history.csv`
(vs. the synthetic sample) is explicitly out of scope for this issue
(flagged to Solutions Architect if it changes scope) — Double Play
same-date-two-draws and X2-X5 non-ball columns are open design
questions, not defects. Did not run a full CLI pass against the real
CSV since it's not required by TP-DATA-IN-101 and not this issue's
scope; unit-level + independent raw-CSV cross-check was the right
verification depth here.

RTVM's TP-DATA-IN-101 status-note update (recording the confirmed date,
replacing the "deferred" language) and DELIV-900's deferred-check
tracking are Systems Engineer's to make on the rtvm-update hand-off,
per issue #10's original division of labor — not touched by me.

**Trunk regression (same issue #27 thread, post-merge):** SE recorded
the merge SHA (`e578223`, tag v1.0.249) into RTVM with "trunk regression
pending" noted; CI/CD then asked for it explicitly. Re-ran clean build +
full suite on `main` @ `b4a8adb` — 88/88, clean tree, no new
clang-format violations beyond the known `Version.h` nit. PASS, handed
back to Systems Engineer to update the RTVM note from "pending" to
"passed." Confirms the DATA-IN-101/DELIV-900 rows already on trunk
matched the actual merge SHA before I signed off — worth spot-checking
that alignment on any post-merge regression ask, not just re-running
tests.
