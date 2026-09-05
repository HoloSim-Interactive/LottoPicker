---
name: out-400-ranked-list-presenter
description: OUT-400 RankedListPresenter pass (issue #21) — verification approach for a pure-presentation console formatter.
metadata:
  type: project
---

Issue #21, 2026-09-05, passed on first hand-off (no fail cycle).

**What it is:** `RankedListPresenter::print(ostream&, vector<RankedCombo>)` —
pure formatting, no dependency on `RankingEngine`/`ModelArtifact`. Renders
header (`Rank | Combination | Score`) + one data row per entry, columns
padded to widest value, `" | "` delimited.

**Verification:** standard build/ctest sufficed (78/78, 3 new `[OUT-400]`
tests map directly onto TP-OUT-400's fixture wording — 5-entry order
check, delimiter-count check, empty-list header-only check). Additionally
ran the real CLI against a synthetic 3-draw CSV/config (`top_n=5`) to
confirm real-run output matches the hand-built-fixture test's shape —
worth doing for presentation-layer items since the unit tests use a
synthetic `RankedCombo` vector, not the real `RankingEngine::rank` output
path through `main()`.

No new gotchas — a clean, low-risk pass. See [CORE-203 RankingEngine](core_203_ranking_engine.md)
for why a full-pool real run wasn't needed here (SWE and TE both used a
tiny synthetic CSV instead, per TP-OUT-400's own "given a ... fixture"
framing).
