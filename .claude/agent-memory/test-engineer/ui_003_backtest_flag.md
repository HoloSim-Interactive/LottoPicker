---
name: ui-003-backtest-flag
description: How TP-UI-003 was verified for the --backtest CLI flag when only flag-parsing (not CORE-205) is in scope.
metadata:
  type: project
---

Issue #13, 2026-09-05: UI-003 (`--backtest <date1>[,<date2>,...]`) has no
dedicated Catch2 test file (no `CliArgsTest.cpp` exists even for UI-001) —
verification is CLI-binary-level only, same pattern as UI-001/UI-002.
Built a throwaway fixture config + CSV under `/tmp` and invoked the built
`lottopicker` binary directly with single-date, comma-separated
multi-date, malformed-date, empty-value, trailing-comma, and
unrecognized-flag cases, plus a no-flag regression check.

**Key judgment call, likely to recur for CORE-205's issue:** TP-UI-003
step 2 says "exactly 3 report rows... per DATA-OUT-302" but this issue's
scope is flag-parsing only (CORE-205, the actual backtest engine, is a
separate later issue this one's issue text explicitly defers to).
Judged the row-count/mode-selection level of output (3 placeholder lines,
one per date, clearly marked "not yet implemented (CORE-205)") sufficient
to pass *this* item — real rank/percentile/containment content is
CORE-205's gate, not UI-003's. Re-apply this same split when CORE-205
lands: at that point the placeholder text should be real backtest output,
and TP-UI-003/TP-DATA-OUT-302 should be re-checked at full content level,
not just row-count.

**Trunk regression (same issue #13, after CI/CD merge e020769/v1.0.114):**
re-ran the identical CLI probes against the merged binary (main @
9f743b5) — output byte-identical to the pre-merge check. 35/35 ctest
still green. This confirms the merge-then-regress hand-off pattern:
CI/CD merges → Systems Engineer records SHA in RTVM and routes
`status:ready-for-test`/`agent:test-engineer` for regression → Test
Engineer re-verifies the same feature on trunk (not just "did other
things break") before releasing `status:ready-for-commit` back to
CI/CD. No new RTVM edit needed on a pure-regression pass if RTVM
already has the merge SHA recorded.
