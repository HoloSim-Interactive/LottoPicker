---
name: rtvm-fast-path-reconcile-against-main
description: On the ready-for-rtvm-update fast path, edit main's current docs/RTVM.md, not the feature branch's stale copy, when Test Engineer flags main is ahead
metadata:
  type: feedback
---

On issue #10 (DATA-IN-101), Test Engineer's pass comment flagged that
`origin/main` already carried a prior Systems Engineer RTVM edit (the
Solutions Architect ruling text dropping the part-2 Verified gate),
but the `issue-10` feature branch's own `docs/RTVM.md` was still the
older pre-ruling text — normal, since the feature branch was cut
before that ruling landed on trunk via a separate issue-flow commit.

**Why this matters:** the fast-path instruction ("update the relevant
RTVM item's status... then hand off to CI/CD") doesn't say which
copy to edit. Editing the stale branch copy would either silently
regress the ruling text or hand CI/CD a merge conflict on a doc that's
already ahead of the branch. See related
[[lock_pull_resolves_stale_read.md]] and
[[branch_convention_pre_vs_post_merge.md]] — same underlying pattern
(concurrent trunk edits vs. a first read), different trigger (an
explicit Test Engineer heads-up rather than my own stale-read
surprise).

**How to apply:** whenever a handoff comment explicitly notes main/a
sibling branch is ahead on a shared doc, `git fetch`, diff the branch
copy against `origin/main`, and base the new edit on `main`'s current
content (working directly on `main`, per
[[branch_convention_pre_vs_post_merge.md]]'s post-merge-edit-on-main
rule) rather than the feature branch's copy — even mid-flow, before
CI/CD has formally merged the code changes.
