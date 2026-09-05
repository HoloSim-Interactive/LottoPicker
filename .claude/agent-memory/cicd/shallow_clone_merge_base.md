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

**Recurred on issue #14 (CORE-206) merging 2026-09-05**: this time it
showed up as `git merge --no-ff` outright refusing with "fatal:
refusing to merge unrelated histories" (not just an empty
`merge-base`) after a routine `git pull origin main` fast-forward —
the checkout was shallow from the start of the session, unrelated to
that pull. `git fetch --unshallow origin` immediately fixed it and the
merge proceeded normally (one real conflict after that, see
[[concurrent_trunk_merge_race]]). Treat "unrelated histories" on a
feature branch that Systems/Test Engineer have been actively working
off of main as this same shallow-clone artifact, not a real divergence
— check shallow-ness before treating it as a genuine history problem.

**Recurs almost every session** — checkout starts shallow by default.
On issue #27 (2026-09-05), `git diff origin/main..origin/issue-27
--stat` while still shallow showed a big, misleading pile of unrelated
file diffs (deleted workflow files, CMakePresets.json, etc.) that
looked like real divergence; `is-shallow-repository` was true,
`fetch --unshallow` fixed it, and `git log --oneline
origin/main..origin/issue-27` came back genuinely empty (branch fully
merged already). Make `git rev-parse --is-shallow-repository` a
standing first check before trusting *any* diff/log between
`origin/main` and a feature branch, not just before a merge.
