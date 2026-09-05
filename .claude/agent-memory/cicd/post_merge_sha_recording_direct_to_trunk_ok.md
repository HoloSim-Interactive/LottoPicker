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

Confirmed a third time on issue #6's (UI-001) second `ready-for-commit`
round, this time following a full trunk merge + regression cycle
(`8fd22b2`, tag `v1.0.54`) rather than an Inspection item: `cbed585`
recorded Test Engineer's already-reported regression pass, `origin/main..origin/issue-6`
was empty. Pattern generalizes across item types — check
"is this SHA already reported, and is the branch diff empty" before
assuming there's a merge to do.

Confirmed a fourth time on issue #11 (CORE-200), 2026-09-05, after a
full trunk-merge + regression cycle (`5e7f0fc`, tag `v1.0.78`): SE's
two follow-up edits (`d7b870a` marking Verified with the merge SHA,
`15e036c` recording the regression pass) both landed on `main` after
CI/CD's merge comment, both licensed. `origin/main..origin/issue-11`
empty. Hand-off comment in this case, and in the issue #8 precedent,
omitted a new status label entirely and went straight to
`agent:systems-engineer` with no `status:*` add (step 6c is skippable
when there's genuinely nothing to report beyond "already done").

Confirmed a fifth time on issue #18 (DATA-OUT-301), 2026-09-05 — first
occurrence on a *Test*-method RTVM item satisfied by an earlier scope
call (issue #16/CORE-204's `ModelSerializer`), not an Inspection item:
SE's RTVM edit `cd5d297` recorded commit `86ca3ed` (already tagged
v1.0.145 from #16), landing directly on `main` before this issue #18
ever had a CI/CD merge of its own — but licensed because the
substantive commit predates issue #18's whole cycle, not because it
came after a CI/CD report on *this* issue. `origin/main..origin/issue-18`
was memory-notes only. Pattern generalizes beyond "SHA already
reported by CI/CD on this issue" to "SHA already exists on trunk from
a prior issue, nothing this cycle needed to commit." Same handling:
no merge, no tag, hand off to `agent:systems-engineer` with no new
status label.

Confirmed a sixth time on issue #24 (NFR-500, 2026-09-05) — a pure
Inspection item with no `issue-24` branch ever created at all (not
even a memory-notes-only one, unlike #8/#11/#18). SE committed the
Verified status straight to `main` (`f217b45`) before handing off
`status:ready-for-commit`, then a follow-up memory-only commit
(`6d8853b`) landed after. Nothing for CI/CD to merge or tag. Checked
the full RTVM table anyway (DELIV-900/901 still Approved, so no
release regardless). First mid-task correction of my own hand-off
comment: I initially wrote `Next: status:ready-for-rtvm-update` out of
habit before catching that the no-new-work pattern calls for no status
label at all — posted a correcting follow-up comment rather than
leaving the wrong Next line for stall-recovery to parse.

Confirmed a seventh time on issue #27 (DATA-IN-101 part 2, 2026-09-05)
after a full trunk-merge (`e578223`, tag v1.0.249) + regression cycle:
SE's `22666b8` recorded TE's already-reported trunk regression pass
(`b4a8adb`) direct to `main`. `origin/main..origin/issue-27` empty
after unshallowing (see [[shallow_clone_merge_base]] — checkout was
shallow going in, `git diff origin/main..origin/issue-27` showed a
misleading pile of unrelated file diffs before unshallowing/checking
merge-base properly; always confirm shallow-ness first on this repo,
it recurs almost every session). DELIV-901 still Approved, so no
release. Hand-off: no new status label, straight to
`agent:systems-engineer`.
