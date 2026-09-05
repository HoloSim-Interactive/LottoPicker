---
name: rebase-before-push-gotcha
description: git push rejected with a workflows-permission error even though my branch never touched .github/workflows — caused by a stale local main, fixed by rebasing.
metadata:
  type: reference
---

Pushing a freshly branched `issue-N` branch was rejected by GitHub with:

```
! [remote rejected] issue-N -> issue-N (refusing to allow a GitHub App
  to create or update workflow `.github/workflows/windows-verification.yml`
  without `workflows` permission)
```

even though the branch's own commit never touched that file. Cause:
local `main` was stale — `git status` had reported "up to date with
origin/main" at the *start* of the session, but another agent run
(CI/CD, via a separate issue) had pushed a commit to `origin/main`
that itself modified `.github/workflows/windows-verification.yml`
*after* my local clone's last fetch and *before* my push. Since my
branch was forked from the old `main`, pushing it tried to carry that
workflow-file diff along as part of the branch's own commit range from
GitHub's point of view.

**Fix:** `git fetch origin && git rebase origin/main` on the feature
branch before pushing. After rebase the branch no longer carries any
workflow-file diff and pushes cleanly. Re-run the build/test suite
after rebasing even if the rebase reports no conflicts — confirm
nothing shifted before handing off.

**How to apply:** this can happen on *any* issue, not just ones near
Windows-CI work, any time enough wall-clock time passes between
`git checkout -b` and `git push` for another concurrent agent run to
land a commit on trunk. Cheap insurance: fetch + rebase onto
`origin/main` right before the final push, not just at branch
creation.
