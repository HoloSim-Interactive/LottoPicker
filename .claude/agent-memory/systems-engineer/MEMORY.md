# Systems Engineer — memory

**This file is an index, not a store.** It is loaded on every run you
ever do, so anything verbose here is re-read on every future hand-off
for the rest of the project. Keep each entry to one line: a link and a
one-sentence summary. Put the actual detail in its own file in this
folder.

    - [Short title](descriptive_slug.md) — one sentence on what it is.

A genuinely one-line fact can stay a plain line with no file of its
own. Split a lesson out when it needs a reproduction, a command
sequence, or real reasoning to be useful later. See "Memory structure"
in `.github/AGENT_LABELS.md`.

## RTVM conventions

- [LottoPicker RTVM ID scheme](rtvm_id_scheme_lottopicker.md) — category prefixes/ranges and the 19 items populated 2026-09-05; console-only MVP output is deliberate, not an oversight.
- [LottoPicker Implementation Plan](lottopicker_implementation_plan.md) — issues #5-#26 map to RTVM IDs; full dependency graph/rationale, check before touching any feature issue's dependencies.
- [DATA-IN-100 row-numbering convention](data_in_100_row_numbering_convention.md) — resolved: 1-based physical file line, header=row 1; applies to any future row-specific CSV error too.

## Cross-product interface standards

## Requirements patterns and traps

- [Lock-acquire pull resolves a stale-looking doc](lock_pull_resolves_stale_read.md) — before rewriting a shared doc that looks out of sync with another role's claim, pull (lock-acquire does this) and re-read before assuming the edit was lost.
- [Genuine research via public APIs](genuine_research_via_public_apis.md) — Crossref/GitHub-search/Wikipedia REST calls for real, checkable sources when a stakeholder need demands actual comparative research, not restated client description.
- [Branch convention: pre- vs post-merge](branch_convention_pre_vs_post_merge.md) — issue-<N> branch is for pre-merge collaboration; once CI/CD reports a merge SHA, editing main directly for the RTVM SHA/status update is correct, not a repeat of the direct-to-trunk flag.
- [DATA-IN-101 missing real CSV](data_in_101_missing_real_csv.md) — resolved: SA ruled Verified gate is part 1 only, part 2 deferred to DELIV-900; reuse this pattern directly if a test clause ever depends on a client-controlled artifact with no ETA.
- [RTVM fast-path: reconcile against main](rtvm_fast_path_reconcile_against_main.md) — if Test Engineer flags main is ahead of the feature branch's RTVM copy, edit main directly, not the stale branch copy.

## Documentation index

- [LottoPicker SDD decisions](lottopicker_sdd_decisions.md) — CORE-206 normalization method, unconfirmed FL rule-change date, CORE-207 summary location, UI-003 CLI flag, target-platform strategy, sparse-model decision. Check before revisiting these RTVM items.
