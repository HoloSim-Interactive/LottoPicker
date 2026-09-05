# Project Definition

<!--
Owned by the Product Manager. Every item below is tagged
[CONFIRMED] (stated directly by the client) or [PROPOSED] (a
recommended default, not yet a decision). Nothing may be built
against a [PROPOSED] item — flip it to [CONFIRMED] once the client
has actually responded, before handing off to the Systems Engineer.
-->

## Mission Statement

[PROPOSED] Build a C++ command-line tool that analyzes the complete
historical drawing record of the Florida Lotto and produces a ranked
list of 6-number combinations, using a composite statistical model, so
that the actual winning numbers land near the top of that ranking more
often than chance alone would predict. This is a statistics research
tool, not a numbers-picking or gambling product — it does not claim to
predict winners, and the client has been explicit about that framing.

## Value

[PROPOSED] Gives the client (Kyle) a rigorous, backtestable way to
evaluate whether a frequency/co-occurrence-based statistical model can
measurably outperform random chance at anticipating lottery draw
outcomes, using historical regression testing as the yardstick rather
than anecdote.

## Stakeholders and Needs

| Need ID | Stakeholder | Description & Rationale |
| --- | --- | --- |
| SN-1 | Kyle (client) | [CONFIRMED] Wants a CLI tool that ranks all possible Florida Lotto 6-number combinations by a composite probability/frequency metric derived from full draw history, on the premise that the sequential-repeat probability of a number (or group of numbers) declines the more recently it was drawn, and that this should produce a measurable skew when applied across history. |
| SN-2 | Kyle (client) | [CONFIRMED] Needs the ranking's accuracy measured against partial-match outcomes (matching 3, 4, 5, or 6 of the 6 drawn numbers), not just exact 6/6 matches — mirrors Florida Lotto's own prize tiers. |
| SN-3 | Kyle (client) | [CONFIRMED] Needs a retroactive backtest mode: for a sample date in each year of Florida Lotto history, rank combinations using only data available up to that date, then report how close the ranking came to that draw's actual winning numbers (top-N containment / percentile rank, not just a single top guess). |
| SN-4 | Kyle (client) | [PROPOSED] Needs the complete historical Florida Lotto draw history available to the tool as an input. Acquisition mechanism (scrape floridalottery.com vs. a client-supplied data file) not yet confirmed — see open questions on issue #1. |

## MVP Definition

- **Target platform:** [PROPOSED] CLI application, C++, cross-platform
  (no OS-specific APIs) — specific OS(es) the client will actually run
  it on not yet confirmed.
- **Language / stack:** [CONFIRMED] C++
- **Output format and delivery:** [PROPOSED] Ranked list of
  combinations printed to the console, with an optional export to a
  text/CSV file. Exact output size (e.g. top 100 vs. a configurable N)
  not yet confirmed — ranking all ~22.9 million possible combinations
  in full is impractical to display.

## Scope

### In scope for MVP

- [CONFIRMED] Statistical ranking engine that scores all possible
  6-number Florida Lotto combinations using a composite
  frequency/co-occurrence model built from historical draws: per-number
  frequency decay, extended to 2-, 3-, 4-, 5-, and 6-number
  co-occurrence groups, combined into one ranking metric per
  combination (baseline approach as described by the client in the
  Kickoff Runbook).
- [CONFIRMED] A backtest/regression-test mode: run the ranking model
  against historical data truncated to a chosen past date, and report
  where the actual drawn numbers fell in that ranking, at the
  pick-3/4/5/6 partial-match level, across a sample date from each year
  of Florida Lotto history.
- [PROPOSED] Historical data ingestion for the complete Florida Lotto
  draw history. Mechanism TBD — see open questions on issue #1.

### Explicitly out of scope

- [PROPOSED] Any lottery game other than Florida Lotto (e.g. Florida's
  own Pick 3/4/5 draw games, Powerball, Mega Millions) — this MVP is
  Florida Lotto (6-number draw) only.
- [PROPOSED] Any ticket-purchasing or gambling-facilitation feature —
  this is a statistics/research tool only, per the client's explicit
  framing in the Kickoff Runbook.
- [PROPOSED] A GUI — CLI only, per the Kickoff Runbook.
- [CONFIRMED] Comparative research across many distinct statistical
  algorithms — the client's "keep it cheap" budget directive points
  toward implementing and validating the one composite model described
  in SN-1, not an open-ended survey of alternative methods.

## Deliverable Requirements

- [CONFIRMED] No end-user documentation required (client confirmed
  directly in the Kickoff Runbook, 2026-09-05: "User documentation
  needed? — No").
- [CONFIRMED] Budget directive from client ("let's do this cheap"):
  favor a single, well-justified statistical approach validated by
  backtest, over an open-ended comparative research effort across many
  strategies.
