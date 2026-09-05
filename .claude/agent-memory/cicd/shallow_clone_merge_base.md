---
name: shallow-clone-merge-base
description: git merge-base / rev-list on origin/main look wrong (2 commits, unrelated history) when the runner's checkout is shallow — unshallow before merging.
metadata:
  type: project
---

On issue #5's merge, `git rev-list --count origin/main` reported only
2 commits and `git merge-base origin/main issue-5` returned nothing,
as if main and the feature branch had unrelated histories — even
though `gh api repos/.../commits?sha=main` showed 25 commits on
GitHub. `git rev-parse --is-shallow-repository` confirmed the
checkout was shallow; the missing history was just not fetched
locally, not an actual rewrite of main.

**Why it matters:** trusting the shallow view would have led to
either a bogus "history diverged, escalate" conclusion or, worse, a
`--allow-unrelated-histories` merge that actually orphans main's real
history.

**How to apply:** if `git merge-base <base> <branch>` ever comes back
empty right before a trunk merge, check
`git rev-parse --is-shallow-repository` first. If true, run
`git fetch --unshallow origin` and re-check before concluding
anything about the branch relationship. Don't escalate or force a
merge based on a shallow view.
