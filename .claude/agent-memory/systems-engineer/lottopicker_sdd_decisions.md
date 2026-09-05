---
name: lottopicker-sdd-decisions
description: Key SDD architecture decisions for LottoPicker (issue #3) — CORE-206/207 resolutions, CLI invocation, target-platform strategy. Check before revisiting these RTVM items.
metadata:
  type: project
---

`docs/SDD.md` (written 2026-09-05) resolved several things future issues
on this project should treat as settled, not reopen from scratch:

- **CORE-206 normalization method**: observed-minus-chance-expected,
  using each era's own hypergeometric baseline `p(g,n) = C(n-g,6-g)/C(n,n... `
  (see SDD for the real formula) — not raw-count pooling, not discarding
  pre-change draws. Requires DATA-IN-101's era tag to look up `n_era`.
- **Florida Lotto's actual 6/49→6/53 rule-change date is an unconfirmed
  working hypothesis (1999), sourced from a secondary blog**, not the
  official Florida Lottery archive (a JS-rendered SPA, not scrapable
  with curl). Flagged explicitly in SDD and RTVM's TP-DATA-IN-101 for
  Software Engineer to cross-check against the real client-supplied CSV
  during implementation. If a later issue reports the real data
  disagrees, that's expected — update the one-line era table in code,
  not a design change, and note it back to RTVM.
- **CORE-207 research summary already exists**: `docs/research/CORE-207-comparative-research.md`,
  5 sources (1 peer-reviewed paper via Crossref, 2 GitHub repos, 2
  Wikipedia references), with an explicit recommendation feeding
  CORE-202 and CORE-206. A CORE-207 feature issue during
  implementation should point Test Engineer at this file for
  Inspection, not regenerate it.
- **UI-003 invocation**: `--backtest <date1>[,<date2>,...]` CLI flag,
  not a config key — config file stays limited to `data_file`/`top_n`.
- **Target-platform verification**: build/iterate on Linux per feature;
  one Windows/Visual-Studio consolidation check at the end (gated to
  DELIV-901 only), not per-feature. See [[windows_verification_gating]]
  if that file exists, or `docs/SDD.md`'s own "Target-platform
  verification strategy" section for the full rationale — worth reusing
  verbatim on any future project with the same Linux-agent/Windows-
  client split and no OS-specific API surface.
- **Model artifact and co-occurrence tables are sparse** (only groups
  with nonzero historical occurrence), never the full 22.9M-combination
  space — applies to CORE-201, CORE-203, CORE-204, DATA-OUT-301 alike.
  If a future feature issue proposes a dense representation, that's a
  regression against this decision, not a new idea.
- **Model file format bumped v1 -> v2 during CORE-203 (issue #17)**:
  added `baseline_cooc=<b2>,<b3>,<b4>,<b5>,<b6>` (one chance-expected
  baseline per group size 2-6), because ranking evaluates `norm_cooc`
  for unobserved groups too, and per CORE-206 those aren't `0.0` —
  they're `0.0 - baseline`. A stale v1 file is rejected by
  `ModelSerializer::read()` and self-heals via the existing
  rebuild-on-invalid-artifact path (no migration code). If a future
  requirement needs another persisted scalar, follow this same
  pattern: bump the version line, let the old format fail closed into
  a rebuild, don't write a migrator. Documented in `docs/SDD.md`'s
  Interfaces & File Formats section.
