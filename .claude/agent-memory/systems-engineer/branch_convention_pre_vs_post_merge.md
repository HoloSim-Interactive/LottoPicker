---
name: branch-convention-pre-vs-post-merge
description: When SE's RTVM edits for a feature issue should go on issue-<N> branch vs directly to main
metadata:
  type: feedback
---

The `issue-<N>` branch convention (`.github/AGENT_LABELS.md` §Branch
convention) governs work *before* CI/CD merges that issue — Software
Engineer creates it, Test Engineer/CI/CD check it out, only CI/CD
merges to trunk.

Once CI/CD has already merged (the "Receiving a commit confirmation
from CI/CD" step — you're recording the SHA it reports and setting
status to Verified), the issue's branch has served its purpose and
trunk *is* the current state. Pushing that SHA-recording edit directly
to `main` is correct in that case, not a repeat of the "direct-to-trunk
commit" mistake CI/CD flagged on issue #8 (2026-09-05) — that earlier
flag was about an RTVM status edit landing on main *before* the
issue's own merge went through, bypassing Test Engineer sign-off
entirely. The distinguishing question: has CI/CD already reported a
merge SHA for *this* issue? If yes, edit main directly. If no (e.g.
you're mid-issue relaying a scope answer, or doing RTVM work that
isn't yet part of a commit-confirmation reply), use `git pull --rebase`
against main as usual for docs — RTVM/SDD edits outside the
feature-issue commit-confirmation flow were never on the
`issue-<N>` branch in the first place, since Systems Engineer's own
work (RTVM/SDD/Implementation Plan issues) doesn't go through that
branch convention at all — only Software Engineer/Test Engineer/CI/CD
activity on `[RTVM-014]`-style issues does.

See [LottoPicker Implementation Plan](lottopicker_implementation_plan.md)
for the issue numbering this applies to.

Confirmed correct on issue #8's second round (2026-09-05, post CI/CD
regression-merge cycle): CORE-207's RTVM row was already accurate
(Verified, SHA recorded) when Test Engineer's regression pass came
back via `status:ready-for-rtvm-update`. No edit to `docs/RTVM.md` was
needed or made this round — but the fast path (per `.github/AGENT_LABELS.md`)
still routes to `agent:cicd` with `status:ready-for-commit` regardless
of whether anything actually changed. Don't skip the CI/CD hand-off
just because the RTVM row didn't need editing.

Confirmed again on issue #7 (DATA-IN-100, 2026-09-05): here the RTVM
row *did* need a small edit (status note said "regression testing
pending" and needed updating to reflect the actual pass + trunk HEAD
SHA), pushed directly to `main` per the same rule — SHA/status text
tweaks after a reported merge are edits to already-current trunk
state, not pre-merge collaboration.

Confirmed a third time on issue #6 (UI-001, 2026-09-05), same shape as
#7: after CI/CD's merge (8fd22b2) and the "regression testing needed"
hand-off, Test Engineer's regression pass came back with a trunk HEAD
SHA (5111c63). Updated the row's status note from "trunk regression
pending" to name that SHA, pushed direct to `main`, then handed off
`status:ready-for-commit` → `agent:cicd` per the fast path even though
CI/CD will likely find nothing new to merge (as it didn't on #7) —
that's expected and not an error, it's how this chain formally closes.

Confirmed the *other* branch (first-ever `status:ready-for-rtvm-update`
on a feature, before any CI/CD merge) on issue #11 (CORE-200,
2026-09-05): Test Engineer's pass was the first report for this issue
— CI/CD had not yet reported a merge SHA — so the RTVM status edit
went to `issue-11` (which already existed, created by Software
Engineer), not `main`. Set status text to "In Test (... pending CI/CD
merge)" rather than "Verified", since Verified is reserved for after
CI/CD's commit-confirmation step per the Status vocabulary. Then still
handed off `status:ready-for-commit` → `agent:cicd` per the fast path.

Issue #11's second round (post-merge trunk regression, same day) landed
on `main` directly per the general rule above — CORE-200 was already
Verified with SHA 5e7f0fc recorded from the commit-confirmation step;
this round only updated the status note's parenthetical from "trunk
regression pending" to "trunk regression passed, 23/23". Same
fast-path hand-off to `agent:cicd` afterward regardless.

Confirmed the first-ever-pass case again on issue #14 (CORE-206,
2026-09-05): Test Engineer's `ready-for-rtvm-update` was the first
report for this issue, no CI/CD merge SHA yet, so the edit went to
`issue-14` (already existed) with status text "In Test (... pending
CI/CD merge)", not `main`/Verified. Same fast-path hand-off to
`agent:cicd` afterward.

Confirmed a fourth time on issue #15 (CORE-202, 2026-09-05), same
shape as #11/#14: first-ever `ready-for-rtvm-update`, no CI/CD merge
SHA yet, edit went to `issue-15` (already existed), status text "In
Test (... pending CI/CD merge)". Also flagged SWE's forward-looking
scope note (no config surface yet for `CompositeWeights` re-tuning)
onto the already-created downstream issue #17 (CORE-203) as an
informational comment, since that's the issue that will actually wire
`CompositeScorer` into the pipeline — not an RTVM change itself, just
context-preservation so it isn't lost between issue threads.

Confirmed a fifth time on issue #23 (OUT-401, 2026-09-05): CI/CD's
commit-confirmation reported merge SHA `8ed9543` (v1.0.213, no release
cut) with regression testing needed. Direct push to `main` was
initially rejected (another commit, `ca91a61`, had landed on main in
the interim — CI/CD's own memory-file commit, unrelated file); a plain
`git pull --rebase origin main` resolved it cleanly with no conflict.
Recorded the SHA in OUT-401's Commit(s) column, then still handed off
`status:ready-for-test` → `agent:test-engineer` per the fast path
since CI/CD flagged regression testing needed.
