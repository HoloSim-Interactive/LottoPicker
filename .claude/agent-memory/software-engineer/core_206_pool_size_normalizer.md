---
name: core-206-pool-size-normalizer
description: PoolSizeNormalizer design for CORE-206, the algebraic shortcut used, and the kDefaultHalfLifeDraws duplication that finally had to be fixed (issue #14)
metadata:
  type: project
---

Issue #14 (branch `issue-14`) implemented CORE-206:
`include/lottopicker/PoolSizeNormalizer.h` / `src/PoolSizeNormalizer.cpp`.
`PoolSizeNormalizer::normalize(history, poolMin, poolMax, halfLifeDraws)`
returns a `Result` with `normDecay(number)` / `normCooc(sortedGroup)` —
observed-minus-chance-expected, per `docs/SDD.md`'s Algorithm design
(hypergeometric baseline `p(g,n)=C(n-g,6-g)/C(n,6)`).

**Algebraic shortcut (the key design decision):** the chance-expected
term in both `norm_decay`/`norm_cooc` formulas (`6/n_era(d)` and
`p(|grp|,n_era(d))`) doesn't depend on the specific number/group, only
its group size and the draw's era. So instead of recomputing a fresh
per-number/per-group sum over history, `normalize()` reuses
[[core_200_decay_scorer]]'s and [[core_201_cooccurrence_scoring]]'s
already-computed *raw* scores directly, and does one extra pass over
`history` to accumulate a *scalar baseline per group size* (6 values:
one for decay, 5 for cooc group sizes 2-6) via
`Σ_d w(age(d))·p(g,n_era(d))`. `norm_x = raw_x - baseline[groupSize]`.
This also means an unobserved group's `norm_cooc` is still well-defined
(`0.0 - baseline[g]`, typically negative) without needing to store
every possible group — CORE-201's sparse-storage contract survives
normalization untouched.

**Untagged-record fallback:** a `DrawRecord` with `poolSize <= 0`
(EraTagger's untagged sentinel, or invalid) defensively uses
`kCurrentPoolMax` as `n_era` for that single draw's baseline
contribution, to avoid dividing by zero — should never trigger on a
properly-tagged pipeline run (EraTagger always runs before scoring per
the pipeline diagram).

**The kDefaultHalfLifeDraws duplication finally broke the build:**
[[core_201_cooccurrence_scoring]]'s memory noted `CooccurrenceScorer.h`
re-declared `kDefaultHalfLifeDraws` separately from `DecayScorer.h`
because CORE-200 wasn't merged yet when CORE-201 was implemented, and
flagged it for later consolidation. That flag turned into a hard
"redefinition of inline constexpr" **compile error** the moment
`PoolSizeNormalizer.h` needed both headers in the same translation
unit (`inline constexpr` doesn't deduplicate across two independent
declarations of the same name, only across multiple includes of the
*same* declaration). Fixed by having `CooccurrenceScorer.h` `#include`
`DecayScorer.h` and drop its own copy — a one-line-declaration change,
exactly as anticipated, done as a build-fix within this issue's scope
rather than a separate issue.

**How to apply:** when a memory/comment says "duplicated for now,
consolidate when both land" — treat that as a live TODO to actually
resolve the *first* time a new class needs to include both headers,
not just a historical note. Don't let a documented-but-deferred
duplication linger past the point it becomes a real compile hazard.

**Test fixture technique:** used `halfLifeDraws=1` in
`PoolSizeNormalizerTest.cpp` so decay weights are clean powers of 2
(`(1/2)^age`), then computed exact hand-checkable expected values via
Python's `fractions.Fraction` + `math.comb` (see issue #14's
implementation notes) rather than approximating by hand — avoids
arithmetic transcription errors when the RTVM test procedure demands
"hand-computed expected value within 1e-6 tolerance."
