---
name: client-context
description: Who the client is and the core product framing for LottoPicker
metadata:
  type: user
---

Client is Kyle at HoloSim Entertainment. Project: "Lotto Picker" — a
C++ CLI tool that statistically ranks Florida Lotto 6-number
combinations, validated via historical backtesting. Kyle is explicit
and consistent that this is a **statistics research exercise, not a
gambling/winner-picking tool** — frame all scope and comments that way,
don't slip into "predicting the winning numbers" language.

Budget directive (2026-09-05, Kickoff Runbook): "let's do this cheap."
Kyle even joked "it's not like I've won the lottery." This should bias
scope decisions toward one well-justified approach over broad
comparative research, and toward simple/free tooling over heavier
infrastructure (e.g. a client-provided data file over building a
scraper, unless he says otherwise).

User documentation: client's Kickoff Runbook answer ("No") turned out to
mean "no full user manual," not "nothing at all" — when the PM interview
reached output/config details (2026-09-05), Kyle clarified he does want
minimal run instructions (build from clone, config file format + one
working example, launch command, expected output). Lesson: a quick
"no docs needed" answer given before the client has seen what the config
file / CLI invocation will actually look like may not survive contact
with those details — worth a light re-check once the shape of the
config/CLI is concrete, rather than treating the very first answer as
permanently settled.

Client also reversed the PM's "keep it cheap → skip comparative research"
proposal explicitly: he wants real comparative research (published
strategies, papers, open-source repos) as the actual point of the
exercise — "cheap" applies to build/infra choices (e.g. accepting a
client-supplied CSV instead of building a scraper), not to research
depth. See [[scope_decisions]].
