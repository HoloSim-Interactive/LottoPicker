# Requirements Traceability & Verification Matrix (RTVM)

<!--
Owned by the Systems Engineer. Don't enter line items against a
[PROPOSED] item in docs/PROJECT_DEFINITION.md — wait for it to become
[CONFIRMED]. See systems-engineer.md for the escalation and handoff
rules this document participates in.
-->

## ID scheme

The category blocks below are a starting point — adjust them to fit
the project, not a fixed requirement:

| Category | Prefix | Range |
| --- | --- | --- |
| UI | UI | 001–099 |
| Data in | DATA-IN | 100–199 |
| Core algorithm / processing | CORE | 200–299 |
| Data out | DATA-OUT | 300–399 |
| Output | OUT | 400–499 |
| Non-functional | NFR | 500–599 |
| Deliverable | DELIV | 900–999 |

Companion schemes: `SN-<n>` for stakeholder needs (defined in
`docs/PROJECT_DEFINITION.md`), `TP-<nnn>` for test procedures.

## Verification vocabulary

Test / Demonstration / Analysis / Inspection. `DELIV` items are
typically verified by inspection and specified in `docs/SDD.md`'s
build/toolchain conventions rather than by a runtime test.

## Status vocabulary

Draft → Approved → In Implementation → In Test → Verified, plus
Blocked / Withdrawn.

## Requirements

| Req ID | Requirement | Stakeholder Need(s) | Verification Method | Status | Commit(s) |
| --- | --- | --- | --- | --- | --- |
| UI-001 | CLI accepts exactly one required argument: the path to a config file. Missing or non-existent config path produces a clear usage/error message on stderr and a non-zero exit code; a valid path proceeds. | SN-1, SN-3 | Test | Verified (TP-UI-001 passed by Test Engineer, issue #6; merged to main by CI/CD, no release cut; trunk regression passed by Test Engineer, HEAD 5111c63) | 8fd22b2 |
| UI-002 | Config file is parsed into, at minimum: (a) path to the historical draw CSV, (b) the top-N envelope size. Either key missing, or top-N not a positive integer, is a validation error naming the offending key. | SN-1, SN-4 | Test | Verified (TP-UI-002 passed by Test Engineer, issue #9; merged to main by CI/CD, HEAD 302c87a; trunk regression passed by Test Engineer, 35/35, no regressions, HEAD c44ed31) | 302c87a |
| UI-003 | The tool supports invoking a backtest run in addition to the normal ranking run, parameterized by one or more sample dates used to truncate historical data before ranking. Exact invocation mechanism (CLI flag vs. config key) is an SDD design decision; this item fixes only that the capability is required and user-invocable. | SN-3 | Test | Verified (TP-UI-003 passed by Test Engineer, issue #13 — flag-parsing/mode-selection scope; DATA-OUT-302 row content is CORE-205 placeholder, deferred to that item's own gate; merged to trunk e020769/v1.0.114; trunk regression passed by Test Engineer, 35/35, no regressions, HEAD 9f743b5) | e020769 (tag v1.0.114) |
| DATA-IN-100 | Ingest the client-supplied historical draw CSV (referenced by the config file) into one in-memory record per draw (date + the 6 numbers drawn). A malformed row (wrong count of numbers, out-of-pool-range number) is rejected with an error identifying the specific row, not a silent skip or a whole-run crash. | SN-4 | Test | Verified (TP-DATA-IN-100 passed by Test Engineer, issue #7; merged to main by CI/CD; trunk regression passed by Test Engineer, HEAD 21c3234) | f744a86 |
| DATA-IN-101 | Each ingested draw record is tagged with the number-pool size in effect on its draw date, derived from a documented table of Florida Lotto rule-change dates, so downstream scoring can normalize eras instead of assuming a constant pool size across all history. Verified on part 1 (synthetic-fixture era-tagging mechanism) alone; part 2 (real-CSV boundary cross-check) is a deferred pre-delivery check, not a gate on this item — see TP-DATA-IN-101 note and DELIV-900. | SN-6 | Test | Verified (TP-DATA-IN-101 part 1 passed by Test Engineer, issue #10, 13/13; merged to main by CI/CD, HEAD 4a56564; trunk regression passed by Test Engineer, 35/35, no regressions from concurrent issue-9/issue-10 merge, HEAD c44ed31) | 4a56564 |
| CORE-200 | Per-number frequency-decay score: a recency-weighted score per pool number, via a documented decay function, so more recent occurrences weigh more heavily than older ones. | SN-1 | Test | Verified (TP-CORE-200 passed by Test Engineer, issue #11; merged to main by CI/CD, HEAD 5e7f0fc; trunk regression passed, 23/23) | 5e7f0fc |
| CORE-201 | Co-occurrence scoring for 2-, 3-, 4-, 5-, and 6-number groups: a frequency/recency score per group size for how often that specific set of numbers co-occurred historically. | SN-1 | Test | Verified (TP-CORE-201 passed by Test Engineer, issue #12; merged to main by CI/CD) | ed954a6 |
| CORE-202 | Composite ranking metric: combine the per-number decay score (CORE-200) and all applicable co-occurrence group scores (CORE-201) into one score per full 6-number combination, per the weighting approach selected and documented via the CORE-207 research task. | SN-1, SN-5 | Test | Approved | |
| CORE-203 | Score the full ~22.9M-combination space and retain only the configured top-N (default 100) without materializing the full ranked list in memory. | SN-1 | Test + Analysis | Approved | |
| CORE-204 | Persist the derived statistical model (per-number decay + per-group co-occurrence scores) as its own data artifact, computed once and reused across ranking runs unless the underlying historical data has changed. | SN-1 | Test | Approved | |
| CORE-205 | Backtest engine: for a chosen sample date, run the model using only data available up to that date, then report the actual draw's rank/percentile (or "not found in top-N") and partial-match containment counts at the 3/4/5/6 level. | SN-2, SN-3 | Test | Approved | |
| CORE-206 | Historical pool-size normalization: draws recorded under a different historical pool size are incorporated into the current-era model via a documented, justified normalization method, not discarded. The requirement (normalize, don't discard) is confirmed; normalization method resolved in `docs/SDD.md` §Architecture → Algorithm Design (observed-minus-chance-expected via each era's hypergeometric baseline), informed by CORE-207's research. | SN-6 | Test | Verified (TP-CORE-206 all 3 parts passed by Test Engineer, issue #14, 42/42 incl. 8 new `PoolSizeNormalizerTest.cpp`; merged to main by CI/CD; trunk regression pending) | d235ced |
| CORE-207 | Comparative research task: produce a written research summary surveying published statistical/algorithmic approaches to ranking bounded random draws by recurrence likelihood (papers, prior write-ups, open-source repos), citing at least 3 distinct sources, with a documented recommendation that feeds CORE-202's composite formula and CORE-206's normalization approach. Delivered: `docs/research/CORE-207-comparative-research.md` (5 sources: 1 peer-reviewed paper, 2 open-source repos, 2 Wikipedia references). | SN-5 | Inspection | Verified (formal Inspection passed per TP-CORE-207, issue #8) | 7c30954 |
| DATA-OUT-300 | Ranked-list output structure: ordered (rank, 6-number combination, composite score) records for the retained top-N. | SN-1 | Test | Approved | |
| DATA-OUT-301 | Persisted statistical model schema (pairs with CORE-204): documented file format holding per-number decay scores, per-group co-occurrence scores, and metadata (source data identity/hash, date range covered) sufficient to detect staleness. | SN-1 | Test | Approved | |
| DATA-OUT-302 | Backtest report structure (pairs with CORE-205): per sampled year, the actual draw's numbers, its rank/percentile (or "not found"), and partial-match containment counts for 3/4/5/6. | SN-2, SN-3 | Test | Approved | |
| OUT-400 | Console presentation of the top-N ranked list (rank, combination, score), human-readable. Console is the only required output destination for MVP — file export was not requested and is not required. | SN-1 | Test | Approved | |
| OUT-401 | Console presentation of the backtest report, one row per sampled year, rendering "not found in top-N" distinctly from a numeric rank. | SN-2, SN-3 | Test | Approved | |
| NFR-500 | The tool operates entirely offline against locally supplied data (client-supplied CSV, local config, local persisted model artifact) — no network calls during a normal ranking or backtest run. | SN-4 | Inspection | Approved | |
| DELIV-900 | Minimal run documentation: build-from-clone steps, the config file format with one complete working example (pointing at a small sample CSV and a top-N value), the exact launch command, and what correct console output looks like. Also tracks the deferred DATA-IN-101 part 2 pre-delivery check: confirm/correct the 1988/1999 era boundary against the client's real historical CSV once supplied, before final MVP delivery sign-off (see TP-DATA-IN-101). | Deliverable Requirements | Inspection + Demonstration | Approved | |
| DELIV-901 | Delivered codebase is a real, IDE-openable, maintainable project: actual Visual Studio project/solution structure (or an equivalent that opens cleanly in Visual Studio, e.g. CMake with a Visual Studio generator), so the client can open and keep building on it in Visual Studio on Windows after MVP delivery. | Deliverable Requirements | Inspection | Approved | |

## Test Procedures

<!-- One per verifiable requirement, with concrete test input values
     and expected output — not just "it works." Fixture data below is
     synthetic/hand-constructed specifically so tests are deterministic
     and don't depend on real-world Florida Lotto history changing. -->

**TP-UI-001** — Objective: config-path argument handling.
1. Run the tool with zero arguments. Expected: usage message on stderr, exit code ≠ 0.
2. Run the tool with one argument pointing at a path that does not exist (`./nope.cfg`). Expected: error naming the missing path, exit code ≠ 0.
3. Run with a valid config path pointing at a minimal fixture config (`data_file=fixture_5draws.csv`, `top_n=10`). Expected: tool proceeds to ranking without error.

**TP-UI-002** — Objective: config validation.
1. Fixture config missing `top_n`. Expected: error message names `top_n` specifically, exit code ≠ 0.
2. Fixture config with `top_n=0` and, separately, `top_n=-5`. Expected: validation error stating top-N must be a positive integer, for both.
3. Well-formed fixture config (`data_file=fixture_5draws.csv`, `top_n=10`). Expected: internal Config representation's `data_file` and `top_n` fields match the fixture values exactly.

**TP-UI-003** — Objective: backtest mode is invocable via `--backtest <dates>` (finalized in `docs/SDD.md` §Interfaces & File Formats).
1. Invoke `lottopicker <config> --backtest 2015-06-15` against fixture history `fixture_10years.csv`. Expected: the tool runs the CORE-205 backtest path (not the normal ranking path) and completes without crashing or silently ignoring the mode selection.
2. Invoke with a comma-separated list, `--backtest 2010-03-01,2015-06-15,2020-11-20`. Expected: exactly 3 report rows, one per listed date, per DATA-OUT-302.

**TP-DATA-IN-100** — Objective: CSV ingestion and malformed-row handling.
1. Fixture CSV with 10 well-formed rows (date + 6 numbers each). Expected: exactly 10 in-memory draw records, each matching the fixture's date and numbers.
2. Fixture CSV with row 4 containing only 5 numbers and row 7 containing a number of 60 (outside the 1–53 current pool). Expected: both rows reported as row-specific errors (e.g. "row 4: expected 6 numbers, found 5"; "row 7: number 60 outside valid pool range"); the other 8 valid rows still ingest successfully. Row-numbering convention (resolved, issue #7): `row <n>` is the 1-based **physical file line number, header included** — header is row 1, the first data row is row 2. So "row 4" here means the 3rd data row. `CsvIngestor::ingest` computes the row number in exactly one place; a different convention would be a one-line change if ever needed.

**TP-DATA-IN-101** — Objective: pool-size era tagging.
1. Fixture history spanning a synthetic rule-change date (e.g. draws dated before 2001-01-01 belong to era A, on/after belong to era B). Expected: every record's tagged pool size matches its era.
2. Real-data check (deferred, not a Verified gate — see status note): era boundary is 1988 (6/49 launch) → 1999 (6/53) per `docs/SDD.md`'s Algorithm Design section — flagged there as a working hypothesis from a secondary source, not the Florida Lottery's own archive. Expected, once the client's real CSV exists: cross-check this boundary against the actual client-supplied CSV's observed number ranges per date range, and correct the one-line era table in code (`EraTable.h::eraTable()`) if the real data disagrees.
   **Status note (2026-09-05, issue #10):** part 2 could not be executed — no real client-supplied historical CSV exists anywhere in the repo or pipeline yet (SN-4: client gathers and supplies it himself; the only planned example artifact, DELIV-900's small illustrative sample, is synthetic and not due yet). This is a missing input, not a defect or a design ambiguity — Software Engineer confirmed via repo search and independent public-source research that no more authoritative date is reachable either.
   **Ruling (Solutions Architect, 2026-09-05, issue #10):** DATA-IN-101 reaches Verified on part 1 alone — RTVM's own test philosophy is synthetic/deterministic fixtures precisely so verification doesn't depend on real-world data arriving on a client-controlled timeline; part 2's original "before Verified" clause was a sequencing mismatch in the initial design assumption (that the real CSV would already exist), not a defect in implementation. Part 2 is retained here only as a **deferred pre-delivery check**: confirm/correct the era boundary against the real CSV once the client supplies it, before final MVP delivery sign-off (tracked alongside DELIV-900, since both concern what ships in the final delivered package). No design impact either way — the era table is a one-line change to correct whenever this resolves.

**TP-CORE-200** — Objective: decay scoring favors recency.
1. Synthetic 5-draw fixture history where number 7 appears only in the most recent draw and number 12 appears only in the oldest draw. Expected: `score(7) > score(12)`.
2. A number (e.g. 40) appearing in zero draws in the fixture. Expected: score equals the documented floor value (e.g. 0), not an error.

**TP-CORE-201** — Objective: co-occurrence scoring favors higher-frequency groups.
1. Fixture history where pair {4,17} co-occurs in 3 draws and pair {4,22} co-occurs in 1 draw. Expected: `pair_score({4,17}) > pair_score({4,22})`.
2. Extend the same fixture with a 3-number group and a 4-number group, each with a known higher-vs-lower co-occurrence count. Expected: the same ordering holds at those group sizes.

**TP-CORE-202** — Objective: composite formula correctness.
1. Fixed, hand-computed per-number and co-occurrence fixture inputs, run through the documented composite formula (once finalized via CORE-207). Expected: composite score for one specific 6-number combination matches the hand-calculated expected value within 1e-6 tolerance.

**TP-CORE-203** — Objective: top-N retention at full scale, bounded memory.
1. Run against fixture data with `top_n=100`. Expected: exactly 100 results, sorted strictly descending by score, no duplicate combinations.
2. (Analysis/Inspection, not a runtime assertion) Code review confirms the implementation never materializes all ~22.9M scored combinations simultaneously (e.g. streaming top-N retention, not sort-then-slice).

**TP-CORE-204** — Objective: model persistence and reuse.
1. Run the tool twice against an unchanged `data_file`. Expected: the second run's log/output indicates the persisted model artifact was reused (via a source-data hash/timestamp check), not recomputed from raw history.
2. Add one new draw to the fixture CSV and rerun. Expected: the model artifact is regenerated.

**TP-CORE-205** — Objective: backtest containment reporting.
1. Fixture history through synthetic year Y with one held-out draw known in advance, constructed so it lands inside the top-100. Expected: reported containment counts (e.g. "2 combinations in top-100 match exactly 4 of 6 numbers") match a hand-verified expected result for that fixture.
2. Repeat with a held-out draw constructed so none of its partial-match variants land in the top-N. Expected: report states "not found in top-N" rather than a false rank.

**TP-CORE-206** — Objective: pool-size normalization, observed-minus-chance-expected method (`docs/SDD.md` §Algorithm Design).
1. Synthetic fixture with two eras: era A (pool size 49, 3 draws) and era B (pool size 53, 5 draws), with a hand-pickable number `k` appearing in 2 of the 3 era-A draws and 2 of the 5 era-B draws. Expected: `norm_decay(k)` reflects `k`'s occurrences relative to each era's own chance rate (`6/49` vs `6/53`) rather than a flat raw count of 4 — hand-computed expected value within 1e-6 tolerance.
2. A group fully contained within only pre-change-era draws is not discarded: its `norm_cooc(group)` is nonzero and contributes to the final composite score for any combination containing it (confirms "normalize, don't discard" against an actual scoring run, not just the era-tagging step alone).
3. `expected_count(group, era)` matches the hand-computed hypergeometric value `C(n_era - g, 6 - g) / C(n_era, 6) × draws_in_era` for at least one group size `g` and one era in the fixture.

**TP-CORE-207** — Objective: research summary completeness (Inspection, not a runtime test).
1. Confirm the summary document cites at least 3 distinct sources (a published paper, a prior study/write-up, or an open-source repo).
2. Confirm each cited source states explicitly whether/why it does or doesn't apply to this problem.
3. Confirm the summary ends in an explicit recommendation, referenced by name from CORE-202 (composite formula) and CORE-206 (normalization approach).

**TP-DATA-OUT-300** — Objective: ranked-list structure integrity.
1. Fixture run with `top_n=5`. Expected: structure has exactly 5 entries, strictly descending score order, each combination 6 distinct numbers within the valid pool range.

**TP-DATA-OUT-301** — Objective: model artifact round-trip.
1. Write the model artifact for a fixture dataset, then read it back. Expected: every score and the metadata (source hash, date range) round-trip exactly (bit-for-bit, or within floating-point tolerance for scores).

**TP-DATA-OUT-302** — Objective: backtest report structure integrity.
1. Fixture backtest run across 3 synthetic years. Expected: structure contains exactly one row per year, all fields populated, consistent with TP-CORE-205's fixture results.

**TP-OUT-400** — Objective: console ranked-list formatting.
1. Given a DATA-OUT-300 fixture with 5 entries. Expected: printed output shows exactly 5 rows in the same order, with a header row and clearly delimited columns.

**TP-OUT-401** — Objective: console backtest-report formatting.
1. Given a DATA-OUT-302 fixture for 3 years. Expected: printed output shows exactly one correctly formatted row per year, with the "not found in top-N" case rendered distinctly from a numeric rank.

**TP-NFR-500** — Objective: offline operation (Inspection, not a runtime test).
1. Code/dependency review of the ranking and backtest code paths. Expected: no network client libraries or network calls present (the only sanctioned exception is an optional, explicitly-invoked future scraper utility, itself out of scope for MVP).

**TP-DELIV-900** — Objective: run documentation is sufficient and accurate.
1. A person with no prior context follows the documentation from a clean clone, on the pipeline's build environment. Expected: successful build, successful run using the documented example config/command, output matching the documented example — no undocumented steps required.

**TP-DELIV-901** — Objective: IDE-openable deliverable.
1. Confirm the repository contains a Visual Studio solution (or a CMake configuration that generates one) at the path documented in `docs/SDD.md`'s build/toolchain conventions.
2. Full open-and-build-in-actual-Visual-Studio-on-Windows verification is a one-time consolidation-phase check per `docs/SDD.md`'s target-platform decision (agents run on Linux) — not gated per individual feature.
