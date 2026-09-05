---
name: core-203-ranking-engine
description: RankingEngine design for CORE-203 (issue #17), the baselineCooc/DATA-OUT-301-v2 correctness gap it surfaced, and the min-heap/test-fixture techniques used
metadata:
  type: project
---

Issue #17 (branch `issue-17`) implemented CORE-203:
`RankingEngine::rank(model, topN, weights, poolMin, poolMax)`
(`include/lottopicker/RankingEngine.h` / `src/RankingEngine.cpp`)
iterates the full `C(poolMax-poolMin+1, 6)` space via 6 literal nested
index loops (not recursion -- matches docs/SDD.md's "nested index
loops" wording literally, and `kNumbersPerDraw` is a fixed domain
constant so hardcoding 6 loop levels with a `static_assert` guard is
fine), scoring each via `CompositeScorer::score(ModelArtifact, ...)`
and retaining only the top N in a `std::priority_queue` with a
`>`-flipped comparator (the standard min-heap-via-priority_queue
idiom). Wired into `main()`'s `mode: rank` branch for the first time
(previously a bare `std::cout << "mode: rank\n"` placeholder).

**Real correctness gap found and fixed, not just an implementation
detail:** [[core_204_model_store]]'s `ModelArtifact` only ever
persisted *observed* group scores (CORE-201's sparse-map contract).
But CORE-203 evaluates `norm_cooc` for every one of a combo's `2^6-1`
subsets, the vast majority never historically observed -- and per
CORE-206's own Verified behavior (TP-CORE-206 part 2), an unobserved
group's `norm_cooc` is `0.0 - baselineCooc[size]`, not `0.0`. Without
persisting that baseline scalar, a *reused* (not rebuilt) model would
silently score every unobserved subset as exactly 0 instead --
systematically wrong rankings, not a missing nice-to-have. Fixed by
adding `ModelArtifact::baselineCooc` (5 doubles) and bumping
`ModelSerializer`'s on-disk format from `LOTTOPICKER_MODEL v1` to `v2`
(new `baseline_cooc=<c2>,<c3>,<c4>,<c5>,<c6>` line, right after
`draw_count`). A stale v1 file fails to parse and `ModelStore`'s
existing tryRead()-fails-so-rebuild fallback self-heals it -- no
migration path needed. **Flagged to Systems Engineer: docs/SDD.md's
Interfaces & File Formats section still only documents v1 and needs
updating.**

**How I computed baselineCooc without touching CORE-206's Verified
internals:** `PoolSizeNormalizer::Result` has no accessor for its
private baseline scalars (deliberately, per [[core_204_model_store]]'s
same-pattern note about not modifying Verified code for a build-time-
only need). Instead, duplicated the ~15-line era-aware
weighted-hypergeometric-sum loop directly in
`ModelStore::buildArtifact`, using two *already-public* static
functions built exactly for this kind of reuse:
`CooccurrenceScorer::decayWeight` and
`PoolSizeNormalizer::hypergeometricProbability`. Small duplication,
zero risk to already-tested code.

**CompositeScorer got a second overload**, `score(ModelArtifact, ...)`
alongside CORE-202's original `score(PoolSizeNormalizer::Result, ...)`
-- CORE-203 ranks off the *persisted* model, not a freshly-normalized
`Result`. Both share one internal template helper
(`computeComposite(normDecayFn, normCoocFn, combo, weights)`) taking
the two lookups as generic callables, so the subset-enumeration bitmask
loop exists exactly once, with no `std::function`/virtual-dispatch
overhead in a loop CORE-203 runs ~22.9M times.

**Test fixture technique specific to this issue:** for the
"strictly descending, no duplicates" TP-CORE-203 check, a plain
`norm_decay(k) = k` fixture (like CORE-202's) produces genuine score
ties among lower-ranked combinations once you ask for the top 100 of a
210-combination space -- ties are expected/correct behavior (RTVM only
requires unique *combinations*, not unique *scores*), but they broke my
first attempt at asserting strict inequality. Fixed by using
`norm_decay(k) = 2^k` instead (pool [1,20]): distinct powers of two sum
to a distinct total per distinct subset (uniqueness of binary
representation), guaranteeing no two combinations can ever tie -- a
clean way to test "no ties" specifically without relying on floating-
point noise or a huge real-scale fixture.

**Performance note (not a defect, just a heads-up for whoever next
touches this):** a full real run (pool 1-53, `main()`'s ranking path)
took ~43s wall-clock in a `RelWithDebInfo` build on the CI-equivalent
Ubuntu runner -- expected, given `O(C(53,6) log top_n)` with a
`std::map<vector<int>,double>` lookup (`GroupKey`/`GroupScoreMap`, from
CORE-201) inside the innermost loop, ~57 subset lookups per combo. This
is the complexity class docs/RTVM.md's CORE-203 entry explicitly
commits to; I did not optimize beyond it (would mean touching
CORE-201/206's already-Verified data structures, out of this issue's
scope) but flagged the number in the hand-off in case Test
Engineer/Systems Engineer wants to weigh in on it as a future
perf-tuning item.
