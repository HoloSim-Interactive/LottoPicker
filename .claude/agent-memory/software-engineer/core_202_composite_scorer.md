---
name: core-202-composite-scorer
description: CompositeScorer design for CORE-202 (issue #15), the nested-struct default-argument gotcha it surfaced, and the weight-exposure decision
metadata:
  type: project
---

Issue #15 (branch `issue-15`) implemented CORE-202:
`include/lottopicker/CompositeScorer.h` / `src/CompositeScorer.cpp`.
`CompositeScorer::score(normalized, combo, weights = CompositeWeights{})`
takes a [[core_206_pool_size_normalizer]] `Result` and one full
6-number combo (`std::array<int, kNumbersPerDraw>`, sorted ascending —
same invariant as `DrawRecord::numbers`), and returns
`w1·Σnorm_decay(k) + Σ_{g=2..6} w_g·Σnorm_cooc(group)` per `docs/SDD.md`'s
Algorithm Design. Subset enumeration reuses
[[core_201_cooccurrence_scoring]]'s 6-bit-mask/popcount technique,
applied to the combo's own 6 numbers instead of a draw's.

**C++ gotcha worth remembering:** originally nested a `Weights` struct
inside `CompositeScorer` with `score(..., const Weights &w = Weights{})`
as a default argument — GCC rejected this ("default member initializer
... required before the end of its enclosing class"). A nested class's
default member initializers aren't usable via aggregate
value-initialization from within a sibling member's declaration in the
*same* enclosing class body, even though the nested class itself looks
complete at that point. Fix: moved the struct to namespace scope
(`CompositeWeights`, renamed to avoid colliding with the class), same
pattern `CooccurrenceScorer` already used for `GroupKey`/
`CooccurrenceScores`. **How to apply:** don't nest a struct inside a
class if that class's own member function wants to default-construct
it as a default argument — hoist it to namespace scope instead, from
the start.

**Weight-exposure scope call:** the RTVM wording "expose them so
re-tuning doesn't require a code change" was satisfied by making
`CompositeWeights` a caller-supplied parameter (not literals inline in
the scoring loop) rather than by wiring new keys into `Config`/UI-002.
`main()` still doesn't call `CompositeScorer` at all (CORE-202/203
aren't wired into the pipeline yet, same gap noted in
[[ui_003_backtest_flag_parsing]]) — no config surface exists yet for
these weights to attach to. Flagged in the hand-off comment in case
Systems Engineer wants explicit `config` keys specified before
CORE-203/pipeline-wiring lands.

**Test fixture technique:** single-draw, small-pool (n=10) fixture,
halfLifeDraws=1, so every raw score is a clean 0 or 1 and only the
hypergeometric baseline needs exact-fraction computation (Python
`fractions.Fraction`) — same technique as
[[core_206_pool_size_normalizer]]'s test fixture, scaled down further
since CORE-202 composes two already-tested layers rather than deriving
new probability math.
