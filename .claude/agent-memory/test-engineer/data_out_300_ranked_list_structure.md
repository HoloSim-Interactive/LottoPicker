---
name: data-out-300-ranked-list-structure
description: How TP-DATA-OUT-300 was verified (issue #19) — a dedicated tagged test added on top of already-shipped CORE-203 code, distinct from the DATA-OUT-301 no-diff precedent.
metadata:
  type: project
---

DATA-OUT-300 (issue #19, 2026-09-05, passed): unlike DATA-OUT-301
(issue #18, pure no-diff hand-off), this one *did* get a small new
commit — a dedicated `[DATA-OUT-300][CORE-203]`-tagged `TEST_CASE` in
`tests/RankingEngineTest.cpp` using `top_n=5` exactly as the TP text
specifies (CORE-203's own tests use `topN=100` or don't explicitly
assert the distinctness/range clause). See
[[core_203_ranking_engine]] for the underlying `RankingEngine::rank()`
being tested.

Verified: `./build/default/tests/lottopicker_tests "[DATA-OUT-300]"` →
75/1 pass; full ctest 75/75 (up from 74/74); `clang-format-18
--dry-run -Werror` clean (note: must run from repo root — a relative
path from elsewhere gives a misleading "No such file or directory"
that looks like clang-format itself is missing, when it's actually a
`.clang-format` discovery issue).

**Reusable lesson:** "structure/behavior already exists from a prior
issue" is not automatically a no-diff hand-off like DATA-OUT-301 got —
check whether a test tagged for *this specific* RTVM item, using the
test procedure's *exact* stated fixture parameters, already exists.
If the only coverage is an existing test using different parameters
(here: topN=100 vs the TP's topN=5) or not asserting every TP clause,
that's a legitimate small scoped commit for the Software Engineer, not
a rubber-stamp pass.
