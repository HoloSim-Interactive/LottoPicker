---
name: core-200-decay-scorer
description: DecayScorer design for CORE-200 and the chronological-ordering convention I picked (not fixed in docs)
metadata:
  type: project
---

Issue #11 (branch `issue-11`) implemented CORE-200: `include/lottopicker/DecayScorer.h` /
`src/DecayScorer.cpp`, added to `lottopicker_lib`. `DecayScorer::score(history, poolMin,
poolMax, halfLifeDraws)` returns a `Result` with `at(number)` — raw
`decay_score(k) = Σ_d w(age(d))·[k∈draw(d)]`, `w(age)=exp(-ln(2)·age/HALF_LIFE_DRAWS)`
per `docs/SDD.md`'s Algorithm Design. `kDefaultHalfLifeDraws = 104`. Unseen numbers
floor at 0.0, not an error.

**Chronological-ordering convention (not fixed anywhere in docs — I picked it):**
`DrawRecord` history is NOT assumed to arrive pre-sorted by date. [[data_in_100_csv_ingestion]]
notes `CsvIngestor` only preserves file order; it never sorts by date. Since CORE-200's
`age(d)` is defined relative to "the most recent draw in the data set," `DecayScorer`
sorts a *local copy* (via pointers, `std::stable_sort` on `DrawRecord::date`,
lexicographic — safe because dates are zero-padded ISO-8601, same assumption
DATA-IN-101's era tagging will rely on) before computing ages, rather than trusting
caller order. This makes the function robust to whatever order a future caller
(model-build stage, backtest truncation) hands it a subset of history in. There's a
dedicated test proving output is order-independent
(`DecayScorer is independent of input vector order` in `tests/DecayScorerTest.cpp`).

**Scope boundary respected:** CORE-206 (chance-expected normalization across pool-size
eras) is explicitly a separate, later issue per this issue's own text — `DecayScorer`
only computes the raw score, no era/pool-size awareness at all. When CORE-206 lands, it
likely wraps/post-processes `DecayScorer::Result` rather than modifying this class,
since DATA-IN-101 (era tagging) is a separate upstream stage per the SDD pipeline
diagram.

**How to apply:** CORE-201 (co-occurrence scoring) will want the same
sort-by-date-internally pattern for consistency — don't let it assume pre-sorted input
either.
