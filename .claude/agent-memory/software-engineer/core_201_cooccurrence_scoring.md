---
name: core-201-cooccurrence-scoring
description: CooccurrenceScorer design for CORE-201, and the parallel-issue dependency-duplication call made on issue #12
metadata:
  type: project
---

Issue #12 (branch `issue-12`) implemented CORE-201:
`include/lottopicker/CooccurrenceScorer.h` / `src/CooccurrenceScorer.cpp`.
`CooccurrenceScorer::score(history, halfLifeDraws=104)` returns a
sparse `std::array<std::map<GroupKey,double>, 5>` (index 0 = pairs ...
index 4 = full 6-groups) — only groups observed at least once are
keyed, per `docs/SDD.md`'s Algorithm design sparse-storage constraint
(shared with CORE-203/CORE-204/DATA-OUT-301, [[data_in_100_csv_ingestion]]
established the same "never materialize the dense space" instinct for
CSV row handling). Age is computed by sorting a local copy of
`history` by `date` string — the class does NOT assume the caller's
vector is chronologically ordered.

**Parallel-issue dependency call:** CORE-201's RTVM Dependency is only
DATA-IN-100 (#7), not CORE-200 (#11), even though the algorithm text
says "same decay weighting as CORE-200." When I started #12, issue #11
(CORE-200) was *also* freshly released to `agent:software-engineer` in
parallel and had no merged code yet. Rather than block on an
undeclared dependency, I re-declared `kDefaultHalfLifeDraws` (104) and
the `w(age) = exp(-ln(2)·age/halfLife)` formula directly from
`docs/SDD.md`'s Algorithm design section inside `CooccurrenceScorer`,
and flagged in the hand-off comment that Systems Engineer may want it
consolidated onto a shared decay utility once both land.

**How to apply:** when two RTVM items reference "the same X" but only
one is a declared Dependency, and the other's issue is concurrently
`agent:software-engineer` with no code on trunk yet, don't wait —
re-derive the shared piece from the SDD text (cite the section) and
flag the duplication explicitly for later consolidation rather than
silently coupling to a branch that may not merge first. Consider doing
the same for CORE-202 if it lands before CORE-200's own decay-score
class does.

**Combinatorics note:** enumerating all size-2..6 subsets of a draw's
fixed 6 numbers is just a 6-bit mask loop (`1u << kNumbersPerDraw`,
`std::bitset<kNumbersPerDraw>(mask).count()` for popcount — portable
across MSVC/GCC/Clang, unlike `__builtin_popcount`). No combinatorics
library needed; `kMaxGroupSize == kNumbersPerDraw` by construction so
there's no risk of a group exceeding one draw's numbers.
