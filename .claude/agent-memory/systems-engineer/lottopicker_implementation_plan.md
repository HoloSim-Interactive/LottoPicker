---
name: lottopicker-implementation-plan
description: LottoPicker's Implementation Plan structure — issue numbers 5-26, dependency graph, why the order is what it is. Check before touching any [RTVM-xxx] feature issue's dependencies on this project.
metadata:
  type: project
---

`docs/IMPLEMENTATION_PLAN.md` (issue #4, 2026-09-05) sequenced all 21
RTVM items into a single linear order (no multi-phase split — small
single-implementer console CLI). Issue numbers, in creation order,
matched the planned sequence exactly (no interleaving from other
concurrent issues during creation): #5 Generate Code Base, #6 UI-001,
#7 DATA-IN-100, #8 CORE-207, #9 UI-002, #10 DATA-IN-101, #11 CORE-200,
#12 CORE-201, #13 UI-003, #14 CORE-206, #15 CORE-202, #16 CORE-204,
#17 CORE-203, #18 DATA-OUT-301, #19 DATA-OUT-300, #20 CORE-205,
#21 OUT-400, #22 DATA-OUT-302, #23 OUT-401, #24 NFR-500,
#25 DELIV-900, #26 DELIV-901. Full dependency graph and rationale is
in the plan doc's Mermaid diagram — don't re-derive it from scratch if
a later issue asks "what depends on me" or "am I unblocked yet."

**Key structural decisions, in case a later query second-guesses them:**
- CORE-206 (normalization) sits *after* CORE-200/201 (raw scores) and
  *before* CORE-202/204 (composite/persistence) — every downstream
  consumer uses normalized scores, never raw ones directly.
- CORE-204 (model persistence) and CORE-202/203 (composite/top-N) are
  siblings, both gated only on CORE-206 — they don't depend on each
  other, so Software Engineer can build them concurrently.
- CORE-205 (backtest) needs three things closed first: CORE-204
  (persisted model to truncate/reload), CORE-203 (scoring/top-N logic
  it reuses against truncated data), and UI-003 (the flag that
  invokes it) — all three are real prerequisites, not conservative
  padding.
- NFR-500 and DELIV-900 are both gated on *both* OUT-400 and OUT-401
  finishing, since each inspects/documents the whole finished
  codebase, not just the ranking half.
- DELIV-901 (Windows/VS consolidation check) is last on purpose per
  [[lottopicker_sdd_decisions]]'s target-platform decision — not a
  per-feature gate.

All 21 feature issues got `status:on-hold` + `type:requirement` (no
`Owner:` line — every one is Software Engineer's, since this project
has no UI/scene-developer split; CLI-only per confirmed Scope).
`dependency-check.yml` releases each to `agent:software-engineer` once
its Finish-Start dependencies close.

**#5 Generate Code Base carries no RTVM ID** (it's plan step 1, not a
feature) — when Test Engineer's PASS arrives via the normal
`status:ready-for-rtvm-update` path, there's no RTVM row to touch;
just confirm that against this file and `docs/RTVM.md`, comment, and
hand straight to `agent:cicd` with `status:ready-for-commit`.
