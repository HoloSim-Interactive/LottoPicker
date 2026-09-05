---
name: data-out-300-ranked-list-structure
description: DATA-OUT-300 (issue #19) -- structure was already delivered by CORE-203; only a dedicated tagged test was needed, same pattern as DATA-OUT-301/issue #18.
metadata:
  type: project
---

Issue #19 (branch `issue-19`) is the second case (after DATA-OUT-301 /
issue #18) of an RTVM item whose production code was already delivered
as a side effect of implementing a different, earlier item.
[[core_203_ranking_engine]]'s `RankedCombo` struct and
`RankingEngine::rank()` *are* the DATA-OUT-300 structure -- the
`RankingEngine.h` header comment even says so explicitly (added
proactively during issue #17). No new production code was needed.

**What was actually missing:** a test tagged `[DATA-OUT-300]` that
Test Engineer can filter on directly, matching TP-DATA-OUT-300's exact
stated fixture (`top_n=5`). CORE-203's own tests exist but use
`topN=100` (part 1) or don't explicitly assert the "6 distinct numbers
within valid pool range" clause (they rely on it being structurally
guaranteed by construction, which is true but not itself asserted).
Added one new `TEST_CASE` in `tests/RankingEngineTest.cpp` tagged
`[DATA-OUT-300][CORE-203]`, topN=5 exactly, with explicit
`std::set`-based distinctness + range checks -- so this RTVM item has
its own standalone test evidence rather than requiring a reader to
infer coverage from CORE-203's differently-parameterized case.

**Reusable lesson:** when an RTVM item's requirement is already
satisfied by existing code from a prior issue, don't assume "nothing
to do" -- check whether a *dedicated, correctly-tagged test matching
the test procedure's exact fixture parameters* exists yet. If not,
that's the real, scoped-down deliverable for the issue (a few lines of
test code, not a "no code diff" hand-off like DATA-OUT-301 got). Only
skip to the no-new-commit hand-off (see
[[inspection_type_rtvm_items]]'s CORE-207 precedent and the
DATA-OUT-301/issue-18 thread) when a tagged test *already* exists and
genuinely already exercises the stated procedure.
