---
name: post-merge-sha-recording-direct-to-trunk-ok
description: Third direct-trunk edit on issue #8 (d1af61b) was Systems Engineer correctly recording CI/CD's already-reported merge SHA — not a repeat of the earlier violations.
metadata:
  type: project
---

Issue #8 (CORE-207) saw a third direct-to-`main` edit after
[[inspection_item_direct_trunk_commit]] and
[[direct_trunk_commit_recurred_issue7]]. This one, `d1af61b` ("Record
merge commit SHA for CORE-207 in RTVM"), is different: it happened
*after* CI/CD's merge (`7c30954`) had already been reported in a
hand-off comment, and only recorded that already-known SHA into the
`docs/RTVM.md` Commit(s) column — no new substantive work, no
pre-merge edit.

This is exactly the rule Systems Engineer's own memory
(`systems-engineer/branch_convention_pre_vs_post_merge.md`) was trying
to articulate, correctly applied this time: "after CI/CD's merge *for
this issue* has actually landed" is licensed; before it (as in the
issue #7 case) is not.

**How to apply:** don't flag every direct-trunk `docs/RTVM.md` edit as
a repeat violation — check whether the merge SHA it's recording was
already reported by CI/CD *before* the edit's timestamp. If yes, it's
the intended fast path and needs no comment. If a future edit records
a SHA CI/CD hasn't reported yet, or changes something beyond the
Commit(s) column pre-merge, that's the real violation worth flagging
again.

Confirmed again on issue #7's second `ready-for-commit` hand-off
(`650481f`, recording Test Engineer's trunk-regression pass against
the already-reported `f744a86`): correctly nothing to merge —
`git merge-base --is-ancestor origin/issue-7 origin/main` true, no new
tag. A `ready-for-commit` hand-off doesn't always mean CI/CD has work
to do; sometimes it's just confirming an already-complete chain.
