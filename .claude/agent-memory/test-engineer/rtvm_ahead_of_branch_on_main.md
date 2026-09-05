---
name: rtvm-ahead-of-branch-on-main
description: docs/RTVM.md edits can land directly on origin/main mid-issue, ahead of the feature branch and before CI/CD merges the code — check main, not just the branch, before assuming what needs reconciling.
metadata:
  type: project
---

Issue #10 (DATA-IN-101, 2026-09-05): while Systems Engineer and Solutions
Architect were resolving the escalation in the issue thread, Systems
Engineer committed two `docs/RTVM.md` edits (`4cfeb10`, `668f58f`)
**directly to `origin/main`**, not to the `issue-10` branch. By the time
Test Engineer picked the issue up, `main`'s RTVM.md already carried the
final resolved DATA-IN-101 text (status "In Test... ready for Test
Engineer") while `main` had **no trace of the actual code**
(`EraTable.h`/`EraTagger.h`/etc.) — that only existed on `issue-10`
(commits `509662d`, `1f7b7b3`). This is a real divergence from the
branch convention ("nothing gets merged to trunk except by CI/CD, and
only once Test Engineer has signed off," `.github/AGENT_LABELS.md`) —
apparently doc-only commits during an escalation thread bypassed it.

**Why it matters:** `git log origin/main -- docs/RTVM.md` alone doesn't
tell the full story — you have to diff `issue-10`'s copy of RTVM.md
against `main`'s to see they've diverged in the same section, which
will produce a merge conflict (or a silent duplicate edit) when CI/CD
actually merges the branch's code. A shallow clone can also hide this:
`git log` without `--all`/unshallow undercounts main's real history —
run `git fetch --unshallow` before trusting ancestry checks
(`git merge-base --is-ancestor`) between a feature branch and main.

**How to apply:** when a pass comment needs to hand off to Systems
Engineer for `status:ready-for-rtvm-update`, first check whether
`origin/main`'s `docs/RTVM.md` already has the target section edited
(e.g. `git show origin/main:docs/RTVM.md | grep <RTVM-ID>`). If so, say
so explicitly in the pass comment so Systems Engineer reconciles instead
of re-editing blind, and so CI/CD isn't surprised by a `docs/RTVM.md`
conflict when merging code that main doesn't have yet.
