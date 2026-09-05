---
name: scope-decisions
description: Kickoff scope decisions and open questions for LottoPicker (issue #1)
metadata:
  type: project
---

Decisions/proposals made in PROJECT_DEFINITION.md during kickoff
(2026-09-05), pending client confirmation:

- MVP is Florida Lotto (6-number draw) only — explicitly excludes
  Florida's separate Pick 3/4/5 games and other multi-state lotteries,
  despite the client's issue #1 wording mentioning "pick-3/4/5/6"
  (that phrase refers to partial-match prize tiers within one 6-number
  draw, not separate games — needs client confirmation, see below).
- The statistical strategy is the single composite frequency/decay/
  co-occurrence model the client already described in detail in
  KICKOFF_RUNBOOK.md, not a comparative survey of algorithms — driven
  by the "keep it cheap" budget note. If the client wants real
  comparative research across strategies, this changes both scope and
  cost meaningfully and should come back through Systems Engineer/
  Solutions Architect as a bigger design question.
- Backtest/regression-test is in scope as a first-class feature (SN-3),
  not an afterthought: sample one date per year of Florida Lotto
  history, rank using only data available up to that date, score
  against pick-3/4/5/6 partial-match containment.

**Why:** Client (Kyle) framed this explicitly as a statistics exercise,
not a gambling tool, and gave a detailed baseline algorithm in the
Kickoff Runbook rather than asking for open-ended research — treat that
description as the working hypothesis to implement and validate, unless
told otherwise.

Open questions sent back to client on issue #1 (2026-09-05), unresolved
as of last commit — check the issue thread for answers before RTVM/SDD
work proceeds:
1. Is "pick-3/4/5/6" partial-match tiers within Florida Lotto's 6-number
   draw, or does he actually want Florida's separate Pick 3/4/5 games
   included too?
2. How should historical data get into the tool — scrape
   floridalottery.com, or does the client provide an exported data
   file? (Cost/complexity tradeoff — scraping is a nontrivial technical
   lift for a "keep it cheap" project.)
3. Florida Lotto's number pool size has changed at least once over its
   history — should older draws under a different pool size be
   excluded from the model, or normalized in somehow?
4. Output size/format: full sort of ~22.9M combinations isn't
   displayable — what's the top-N the client actually wants to see, and
   does he want a file export in addition to console output?
5. Target OS(es) to actually run the CLI on.

**How to apply:** Once client answers land on issue #1, update
PROJECT_DEFINITION.md's [PROPOSED] tags to [CONFIRMED] (or revise) and
notify Systems Engineer of anything that changes their read on scope,
per role instructions — don't let a refinement sit only in this memory
file.
