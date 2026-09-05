# Software Engineer — memory

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

## Architecture patterns

- [LottoPicker scaffold](lottopicker_scaffold.md) — CMake layout, INTERFACE lib until first feature source, and the Windows-workflow hand-off note from Generate Code Base (issue #5).
- [DATA-IN-100 CSV ingestion](data_in_100_csv_ingestion.md) — CsvIngestor/DrawRecord design, the row-numbering convention I picked (not fixed in docs), fatal-vs-per-row error split (issue #7).
- [LottoPicker error handling](lottopicker_error_handling.md) — typed-exception pattern (UI-001, issue #6) to extend for future validation errors, plus a clang-format pointer-alignment gotcha.
- [CORE-201 co-occurrence scoring](core_201_cooccurrence_scoring.md) — CooccurrenceScorer design (issue #12) and the call to re-derive CORE-200's decay formula rather than block on its not-yet-merged parallel issue.
- [CORE-200 DecayScorer](core_200_decay_scorer.md) — sorts history by date internally rather than trusting caller order; scope boundary vs. CORE-206 normalization (issue #11).
- [UI-002 config parsing](ui_002_config_parsing.md) — Config::parse design (issue #9), key-check-order/malformed-line choices I made, and a pre-existing CsvFormatError/LottoPickerError inheritance gap flagged for whoever wires ingestion into main().
- [DATA-IN-101 era tagging](data_in_101_era_tagging.md) — EraTable/EraTagger design (issue #10) and the missing-real-CSV gap escalated to Systems Engineer.
- [UI-003 backtest flag parsing](ui_003_backtest_flag_parsing.md) — CliArgs --backtest parsing (issue #13), duplicated-vs-shared date-validation call, and the main.cpp placeholder stub since ingestion isn't wired into main() yet.
- [CORE-206 PoolSizeNormalizer](core_206_pool_size_normalizer.md) — normalization via raw-score-minus-per-group-size-baseline shortcut (issue #14), and the kDefaultHalfLifeDraws duplication that became a real compile error and got consolidated.
- [CORE-202 CompositeScorer](core_202_composite_scorer.md) — linear-combination composite score (issue #15), a nested-struct-default-argument C++ gotcha, and the weight-exposure-vs-Config scope call.
- [CORE-204 ModelStore](core_204_model_store.md) — hash-check/build-or-load orchestration + DATA-OUT-301 serializer scope call, model-path-naming convention, main.cpp finally wired to ingestion (issue #16).
- [CORE-203 RankingEngine](core_203_ranking_engine.md) — min-heap top-N over the full combo space (issue #17), the baselineCooc/DATA-OUT-301-v2 correctness gap it surfaced and fixed, ~43s real-run timing note.
- [DATA-OUT-300 ranked-list structure](data_out_300_ranked_list_structure.md) — issue #19: struct already existed from CORE-203; only a dedicated `[DATA-OUT-300]`-tagged test (topN=5, per TP text) was the real gap.
- [OUT-400 RankedListPresenter](out_400_ranked_list_presenter.md) — issue #21: pure-formatting console table class (header + rows, `|`-delimited, column-width padding), tested via hand-built fixture not a real ranking run.

## Platform-specific notes

## Reusable solutions

- [Inspection-type RTVM items](inspection_type_rtvm_items.md) — CORE-207 (issue #8): no branch/commit needed when the item's test method is Inspection and the artifact was already delivered elsewhere; just verify against the TP-* checklist and hand off.

## Coding standards
