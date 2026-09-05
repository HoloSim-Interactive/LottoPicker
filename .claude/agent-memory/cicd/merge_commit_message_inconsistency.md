---
name: merge-commit-message-inconsistency
description: Trunk merge commits in this repo often lack the required Summary/Source/Testing body — mine (issue #13/UI-003, e020769) does too; fix by writing the full message in the merge -m before pushing, not after.
metadata:
  type: feedback
---

The commit message format in `cicd.md` (Summary/Source/Testing) is
meant for every trunk merge, but in practice only issue-6's merge
(8fd22b204) actually has it — issues #7, #9, #10, #12, and now #13
(e020769) all landed with a bare one-line `Merge issue-N: ...` title.

**Why it matters:** once a merge commit is pushed and nothing has
built on it yet, fixing the message would require `git commit --amend`
+ force-push to `main` — which the standing git safety protocol
prohibits outright ("NEVER force push to main/master"), even though
no one else's work would be lost. So a bad merge message on trunk is
effectively permanent; there's no clean fix after the fact.

**How to apply:** always pass the full Summary/Source/Testing body via
`-m`/heredoc to `git merge --no-ff` itself, before the first push
attempt — don't merge with a placeholder title and plan to fix it
later. If a push race forces a reset-and-redo (see
[[concurrent_trunk_merge_race]]), reuse the same full message on the
redo. If a bare-title merge does slip through, don't try to force-push
a fix — note the gap in the issue comment/commit-adjacent memory
instead and move on; leave history as-is.
