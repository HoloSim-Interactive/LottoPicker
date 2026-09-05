---
name: scope-decisions
description: Kickoff scope decisions for LottoPicker (issue #1), confirmed by client 2026-09-05
metadata:
  type: project
---

Scope fully confirmed by client on issue #1 (2026-09-05), all five
open questions from the PM interview answered. `docs/PROJECT_DEFINITION.md`
is now all [CONFIRMED], no [PROPOSED] tags remain. Key resolutions:

1. **Game scope:** Florida Lotto's own 6-number draw only. "Pick-3/4/5/6"
   in the original ask refers to partial-match prize tiers within that
   one draw, not Florida's separate Pick 3/4/5 games — confirmed, not
   in scope.
2. **Data source:** Client supplies historical draw data as a CSV
   himself; no scraper to be built. Exception: if research turns up an
   existing free scraper or free published dataset built specifically
   for this purpose, use it instead. Paid tools/data are out regardless.
3. **Historical pool-size change:** Confirmed real (Florida Lotto's
   6-number pool size changed at least once historically). Client
   explicitly wants pre-change draws normalized into the model somehow
   rather than discarded, and frames solving this well as a possible
   differentiator ("our edge") — treat as a real open algorithmic
   question for Systems Engineer/Solutions Architect, not a detail to
   quietly resolve by exclusion.
4. **Output:** Two-stage pipeline — (a) compute & persist the derived
   statistical model as data (changes little week to week), (b) use it
   to rank all combinations but keep only top-N (default top 100,
   configurable via config file) for backtest/regression tuning.
   Console output; no file export requested.
5. **Target OS:** Whatever the build environment produces is fine now;
   client runs on Windows and **explicitly wants to open this project in
   Visual Studio himself later to keep enhancing it** — this is a
   deliverable-format requirement (IDE-openable, maintainable codebase),
   not just a functional platform note. Captured in
   PROJECT_DEFINITION.md's Deliverable Requirements and flagged to
   Systems Engineer for the "Generate Code Base" step.

**Reversed from initial PM proposal:** client wants genuine comparative
research into published statistical strategies (papers, prior work,
open-source repos) before finalizing the model — this was explicitly
called out as "the whole point of the exercise," overriding the PM's
"keep it cheap → skip comparative research" default. The "cheap" budget
directive still applies to build/infra choices, just not to research
depth. See [[client_context]].

**Also newly confirmed:** client wants minimal run documentation (build
from clone, config file format + one working example, launch command,
expected output) — a refinement of the earlier "no docs needed" answer,
not a contradiction of it (he still doesn't want a full user manual).

**How to apply:** This scope is locked in for RTVM/SDD/Implementation
Plan. If any future feature work seems to reopen one of these five
points, treat the answers above as binding unless the client says
otherwise — don't re-ask the same questions.
