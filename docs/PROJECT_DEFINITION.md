# Project Definition

<!--
Owned by the Product Manager. Every item below is tagged
[CONFIRMED] (stated directly by the client) or [PROPOSED] (a
recommended default, not yet a decision). Nothing may be built
against a [PROPOSED] item — flip it to [CONFIRMED] once the client
has actually responded, before handing off to the Systems Engineer.
-->

## Mission Statement

[CONFIRMED] Build a C++ command-line tool that analyzes the complete
historical drawing record of the Florida Lotto and produces a ranked
list of 6-number combinations, using a composite statistical model, so
that the actual winning numbers land near the top of that ranking more
often than chance alone would predict. This is a statistics research
tool, not a numbers-picking or gambling product — it does not claim to
predict winners, and the client has been explicit about that framing.

## Value

[CONFIRMED] Gives the client (Kyle) a rigorous, backtestable way to
evaluate whether a frequency/co-occurrence-based statistical model —
selected and tuned using real published research on this class of
problem — can measurably outperform random chance at anticipating
lottery draw outcomes, using historical regression testing as the
yardstick rather than anecdote.

## Stakeholders and Needs

| Need ID | Stakeholder | Description & Rationale |
| --- | --- | --- |
| SN-1 | Kyle (client) | [CONFIRMED] Wants a CLI tool that ranks all possible Florida Lotto 6-number combinations by a composite probability/frequency metric derived from full draw history, on the premise that the sequential-repeat probability of a number (or group of numbers) declines the more recently it was drawn, and that this should produce a measurable skew when applied across history. |
| SN-2 | Kyle (client) | [CONFIRMED] Needs the ranking's accuracy measured against partial-match outcomes (matching 3, 4, 5, or 6 of the 6 drawn numbers), not just exact 6/6 matches — mirrors Florida Lotto's own prize tiers (SN-1/SN-2 apply only to Florida Lotto's own 6-number draw; not Florida's separate Pick 3/4/5 draw games — confirmed 2026-09-05). |
| SN-3 | Kyle (client) | [CONFIRMED] Needs a retroactive backtest mode: for a sample date in each year of Florida Lotto history, rank combinations using only data available up to that date, then report how close the ranking came to that draw's actual winning numbers (top-N containment / percentile rank, not just a single top guess). |
| SN-4 | Kyle (client) | [CONFIRMED] Needs the complete historical Florida Lotto draw history available to the tool as an input, supplied by the client as a CSV file he will gather himself (no scraper to be built for MVP). If research surfaces an existing free public scraper or free published dataset specifically for this data, use it instead — paid tools/data sources are explicitly out. |
| SN-5 | Kyle (client) | [CONFIRMED] Wants real comparative research into published statistical/predictive strategies for this exact class of problem (frequency/recency/co-occurrence-based prediction of a bounded random draw) — white papers, prior studies, and open-source projects — to select and justify the model actually implemented. This is explicitly "the whole point of the exercise" (client, 2026-09-05) and supersedes the earlier PM proposal to skip comparative research; see [[client_context]]. |
| SN-6 | Kyle (client) | [CONFIRMED] Florida Lotto's 6-number pool size has changed at least once over its history. The model must make a deliberate, justified choice about how to use pre-change historical draws — normalize them into the current model somehow, rather than silently discarding them — and this choice should be treated as a real research/design question (the client frames getting this right as a possible differentiator), not an assumption. Flagged for algorithmic design attention downstream (Systems Engineer / Solutions Architect). |

## MVP Definition

- **Target platform:** [CONFIRMED] CLI application, C++, cross-platform
  (no OS-specific APIs). Client will run it on Windows and, whatever
  the build environment produces now is acceptable for MVP.
- **Language / stack:** [CONFIRMED] C++
- **Output format and delivery:** [CONFIRMED] Two-stage pipeline:
  (1) compute and persist the derived statistical model (per-number and
  per-group frequency/decay/co-occurrence metrics) from historical
  draws as its own stored data artifact — this changes only slightly
  week to week and should not be recomputed from scratch as a side
  effect of ranking; (2) using that stored model, generate a ranked
  list of 6-number combinations, keeping only the top-N (default top
  100) and discarding the rest rather than persisting the full ~22.9M
  combination ranking. N (the "top envelope") must be configurable via
  a config file, so it can be widened for backtest/regression work.
  Output goes to the console; file export was not explicitly requested
  and is not required for MVP.

## Scope

### In scope for MVP

- [CONFIRMED] Statistical ranking engine that scores all possible
  6-number Florida Lotto combinations using a composite
  frequency/co-occurrence model built from historical draws: per-number
  frequency decay, extended to 2-, 3-, 4-, 5-, and 6-number
  co-occurrence groups, combined into one ranking metric per
  combination. The client's Kickoff Runbook description is the working
  baseline hypothesis, but per SN-5 it must be validated (and may be
  refined) against real published research before being finalized as
  "the" model — see the research task below.
- [CONFIRMED] A research task, undertaken before/alongside SDD, that
  surveys existing statistical/algorithmic approaches to ranking
  bounded random draws by recurrence likelihood (published papers,
  write-ups, open-source repos) and produces a short justified
  recommendation feeding into the model design. This is not a mandate
  to build multiple competing implementations — one final model still
  gets implemented and validated via backtest — but the choice of model
  must be traceable to this research, not just to the client's initial
  description.
- [CONFIRMED] A backtest/regression-test mode: run the ranking model
  against historical data truncated to a chosen past date, and report
  where the actual drawn numbers fell in that ranking, at the
  pick-3/4/5/6 partial-match level, across a sample date from each year
  of Florida Lotto history.
- [CONFIRMED] Historical data ingestion for the complete Florida Lotto
  draw history, read from a client-supplied CSV file referenced by
  path in the tool's config file. A prior era of Florida Lotto draws
  used a different pool size than today's 6-from-53; the ingestion/
  model design must account for this explicitly (see SN-6) rather than
  assume a constant pool size across all historical rows.
- [CONFIRMED] A config file (format TBD in SDD) that at minimum
  specifies: the path to the historical draw data CSV, and the top-N
  envelope size for the ranked output. The CLI takes this config file
  as its input parameter.

### Explicitly out of scope

- [CONFIRMED] Any lottery game other than Florida Lotto's own 6-number
  draw (e.g. Florida's own separate Pick 3/4/5 draw games, Powerball,
  Mega Millions) — confirmed by client 2026-09-05.
- [CONFIRMED] Any ticket-purchasing or gambling-facilitation feature —
  this is a statistics/research tool only, per the client's explicit
  framing in the Kickoff Runbook.
- [CONFIRMED] A GUI — CLI only, per the Kickoff Runbook.
- [CONFIRMED] Building a web scraper for historical data acquisition —
  client is supplying a CSV. (Using an existing free scraper/dataset
  found during research, if one exists specifically for this purpose,
  is in scope; building one from scratch is not.)
- [CONFIRMED] Persisting the full ranked list of all possible
  combinations — only the derived statistical model and the top-N
  output are kept; everything below the configured envelope is
  discarded, per the MVP Output definition above.

## Deliverable Requirements

- [CONFIRMED] Minimal run documentation is required — this is a
  narrower ask than full user docs and supersedes the earlier "no
  documentation needed" note (client clarified 2026-09-05, see
  [[client_context]]). Must cover: how to build from a fresh clone, the
  config file's format with one complete working example (pointing at
  a small sample historical-data CSV and a top-N value), the exact
  command to launch the tool with that config file, and what correct
  console output looks like. Does not need to cover extending/
  maintaining the code (see next item, which is separate).
- [CONFIRMED] The client intends to open and continue enhancing this
  project himself in Visual Studio on Windows after MVP delivery. The
  codebase/build setup must therefore be left in a state he can open
  as a real IDE project and build on — not just "whatever compiles in
  the pipeline's VM." This is a non-functional, build-tooling/
  documentation requirement for Systems Engineer and Software Engineer
  to plan for explicitly (e.g. at the "Generate Code Base" step),
  separate from the run-instructions item above.
- [CONFIRMED] Budget directive from client ("let's do this cheap") still
  applies to implementation effort and infrastructure choices (e.g.
  client-supplied CSV over building a scraper), but no longer to the
  research step — client explicitly wants real comparative research
  into published statistical strategies (SN-5); "cheap" should not be
  read as skipping that.
