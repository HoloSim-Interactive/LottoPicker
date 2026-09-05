---
name: core-204-model-store
description: ModelStore/ModelSerializer/Sha256 design for CORE-204 (issue #16), the DATA-OUT-301 scope-overlap call, and the model-path-naming convention I picked
metadata:
  type: project
---

Issue #16 (branch `issue-16`) implemented CORE-204: `ModelStore`
(`include/lottopicker/ModelStore.h` / `src/ModelStore.cpp`) is the
hash-check/build-or-load orchestrator docs/SDD.md's Top-level control
flow diagram specifies. `ModelStore::loadOrBuild(dataFile, modelPath)`
computes `sha256(dataFile bytes)` (self-rolled `Sha256` class, FIPS
180-4, no external crypto dep -- matches the Dependency policy's
"standard library only"), compares it against a persisted artifact's
`sourceHash` via `ModelSerializer::tryRead`, and either returns the
loaded artifact (`wasRebuilt=false`) or re-runs
ingest -> [[data_in_101_era_tagging|era-tag]] ->
[[core_200_decay_scorer|decay]]/[[core_201_cooccurrence_scoring|cooc]]
scoring -> [[core_206_pool_size_normalizer|normalization]], persists,
and returns that (`wasRebuilt=true`).

**Scope call: I implemented DATA-OUT-301's serializer here too, not
just CORE-204's orchestration.** Issue #16's text said the artifact's
*file format* is DATA-OUT-301, "a separate, dependent issue" -- but
#18 (DATA-OUT-301) is Finish-Start dependent *on* #16, and
TP-CORE-204 ("run the tool twice... second run reused, not
recomputed") is only checkable if a real load/persist mechanism
exists. Since docs/SDD.md's Interfaces & File Formats already fully
specifies the exact plain-text format (down to `std::to_chars`
shortest-round-trip mode), implementing it wasn't filling an
undefined gap -- it was following an already-fixed spec. `#18`'s job
becomes formally exercising `TP-DATA-OUT-301` against this
already-shipped code (similar to how CORE-207's inspection-type item
worked, see [[inspection_type_rtvm_items]]).

**Model artifact path convention (NOT fixed in docs -- I picked it):**
`ModelStore::defaultModelPath(dataFile)` = `dataFile` with `.model`
appended (`history.csv` -> `history.csv.model`), colocated in the same
directory. Nothing in docs/SDD.md or docs/RTVM.md specifies where the
persisted artifact lives (only DATA-OUT-301's *contents* format is
fixed) -- cheap to change in this one function if a config key or
different convention is wanted later.

**PoolSizeNormalizer::Result doesn't expose which groups were
observed** (only `normCooc(key)` lookups) -- rather than modifying
CORE-206's already-Verified `Result` class to add an accessor,
`ModelStore::buildArtifact` calls `CooccurrenceScorer::score()` a
second time just to enumerate the sparse observed-group keys, then
looks each one up via `PoolSizeNormalizer::Result::normCooc()`. Slight
duplicate computation, but avoids touching a Verified/merged item's
internals for a build-time-only (not hot-path) operation.

**Persisted values are normalized (CORE-206 output), not raw
CORE-200/201 scores** -- the model is meant to be ranking-ready per
CORE-202's composite formula, which consumes `norm_decay`/`norm_cooc`
directly. Raw intermediates are never persisted.

**ModelSerializer::read() vs tryRead():** `read()` throws
`ModelStoreError` on any problem (missing file, bad header, malformed
row) -- used directly by tests and by anything that wants the strict
contract. `tryRead()` wraps it in one try/catch, returning
`std::nullopt` instead of throwing -- this is `ModelStore`'s "does a
valid model exist" check, framed as a single sanctioned exception
boundary (mirrors main()'s catch of `LottoPickerError`) rather than
scattering error handling through the parser, keeping "no exceptions
for control flow in internal algorithmic code" intact.

**Wired ingestion into main() for the first time** (previous issues,
see [[ui_003_backtest_flag_parsing]], left it a placeholder). `main()`
now prints `model: reused (source unchanged) <- <path>` or
`model: rebuilt (N draw(s)) -> <path>` plus any per-row ingest errors
-- this is what TP-CORE-204 part 1's "the second run's log/output
indicates... reused" wording checks against.

**How to apply:** CORE-202 (composite scorer, not yet implemented)
should consume `ModelArtifact::perNumber`/`groupScores` directly
rather than recomputing scores from raw history -- that's the whole
point of persisting them. If DATA-OUT-301 (issue #18) needs to modify
the wire format, `ModelSerializer` is the only place that needs to
change; `ModelStore`/`ModelArtifact` don't know about the on-disk
text shape.
