---
name: inspection-item-direct-trunk-commit
description: On CORE-207 (issue #8), Systems Engineer committed the RTVM status change straight to main instead of onto issue-8 — CI/CD just merged the leftover branch commits.
metadata:
  type: project
---

On issue #8 (CORE-207, an Inspection-type RTVM item — see Software
Engineer's [[inspection_type_rtvm_items]] memory), the Systems
Engineer's RTVM status update (Approved -> Verified) landed as commit
c39bd99 directly on `main`, not on `issue-8`. The `issue-8` branch only
ever held Software Engineer's and Test Engineer's memory-note commits
(no RTVM.md change). Per `.github/AGENT_LABELS.md`'s "Branch
convention," nothing should merge to trunk except via CI/CD — this
was a deviation, but harmless here: no code changed, no conflicting
edits, and Test Engineer's sign-off predated the commit.

**Why it matters:** don't assume "issue-N branch has the work" — check
`git merge-base main issue-N` and diff both directions first,
especially on documentation-only Inspection issues. On this one, the
substantive change was already on trunk and the branch only had
memory notes.

**How to apply:** if a feature branch's diff against main is empty or
unrelated to the issue's actual deliverable, look for the real change
already on main (e.g. via `git log --oneline -- docs/RTVM.md`) before
concluding something is missing. Merge whatever's left on the branch
(here: memory commits) with a normal `--no-ff` merge — don't try to
redo work that's already on trunk. Worth a note to Systems Engineer
next time this recurs, since the standing rule is CI/CD merges to
trunk, not Systems Engineer directly — but not worth blocking a
correct, tested, conflict-free result over.
