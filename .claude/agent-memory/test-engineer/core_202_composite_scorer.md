---
name: core-202-composite-scorer
description: Verification approach for CORE-202 CompositeScorer (issue #15) — first item whose test procedure explicitly demands hand-verification.
metadata:
  type: project
---

CORE-202 (issue #15, 2026-09-05, passed) combines CORE-206's
`PoolSizeNormalizer::Result` (norm_decay per number, norm_cooc per
group) into `composite(combo) = w1 * Sum norm_decay(k) + Sum_{g=2..6}
w_g * Sum norm_cooc(group)`, enumerating all `2^6-1` subsets of a
6-number combo via bitmask (same technique as CooccurrenceScorer).

TP-CORE-202 literally says "hand-calculated expected value" — unlike
CORE-200/CORE-201 (where citing a well-named test case sufficed per
[[lottopicker_generate_code_base]]'s CORE-200 note), this TP's wording
means actually re-deriving the fixture by hand, not just trusting the
test file's own comment claims a hand-derivation happened. Did this
via exact rational arithmetic (fractions, not decimals) against the
one-draw/pool-10/halfLife-1 fixture in `CompositeScorerTest.cpp`:
`composite({1,2,3,4,5,6}) = 5748/35` and the partial-overlap case
`composite({1,2,3,4,5,7}) = 1478/35`. Both matched exactly (not just
within 1e-6 — exact rational equality), a strong positive signal
distinct from "the test passes."

**When to re-derive by hand vs. trust a named test case:** read the TP
text itself. "Matches within 1e-6 tolerance" against a "fixed,
hand-computed fixture" (TP-CORE-202's own wording) is asking Test
Engineer to be the second hand-computation, not just to run the first
one's assertion. A TP that just names expected behavior in prose
(TP-CORE-200's "favors a number seen only in the most recent draw")
doesn't carry that same demand.

Standard build/ctest sufficed otherwise: 47/47 (42 prior + 5 new
`CompositeScorerTest.cpp`), clang-format clean on the 3 changed files.
No new harness needed — composite score is pure computation over an
in-memory fixture, same as CORE-206.
