---
name: regression-ready-for-rtvm-update-still-goes-to-cicd
description: Test Engineer's post-merge regression pass (not a first-time test) still uses status:ready-for-rtvm-update, and the fast-path instruction to hand off to agent:cicd afterward still applies even though there's nothing new to merge
metadata:
  type: project
---

On issue #14 (CORE-206), after the item was already Verified with a
recorded commit SHA (d235ced), CI/CD's trunk-merge comment flagged
"regression testing needed." Test Engineer ran the regression pass on
`main` and handed back to Systems Engineer with the *same*
`status:ready-for-rtvm-update` label used for first-time test passes
— there's no separate "regression confirmed" label in
`.github/AGENT_LABELS.md`.

**Why this matters:** it's tempting to treat a regression pass as
"nothing to do, just close the issue" since the RTVM already shows
Verified. But the role instructions' fast-path section says
literally: on `status:ready-for-rtvm-update`, update RTVM and hand off
to `agent:cicd` with `status:ready-for-commit` — no carve-out for
regression vs. first-time passes. CI/CD has its own matching pattern
(see software-engineer/cicd memory: "post-regression-no-new-commit")
for recognizing there's no diff to merge and bouncing back to Systems
Engineer with no new status label, which is the actual closing step.

**How to apply:** when a regression-pass hand-off arrives on an
already-Verified item, still: (1) edit the RTVM note to reflect
regression completed (e.g. replace "trunk regression pending" with
"trunk regression confirmed clean, N/N, no concurrent changes"),
keeping the same Commit(s) SHA since no new commit was produced; (2)
comment and hand off to `agent:cicd` / `status:ready-for-commit`
exactly as the fast path says, don't skip straight to closing the
issue yourself. CI/CD will recognize the no-diff case and route it
back for the actual close.
