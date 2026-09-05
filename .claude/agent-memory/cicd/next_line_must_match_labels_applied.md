---
name: next-line-must-match-labels-applied
description: After a trunk merge, the Next line hands back to Systems Engineer for SHA/RTVM recording — that's status:ready-for-rtvm-update, not status:ready-for-commit (which would mean back to me).
metadata:
  type: feedback
---

On issue #11 (CORE-200), I drafted my post-merge comment with **Next:**
`status:ready-for-commit, agent:systems-engineer` — copying the label
the issue *arrived* with, instead of the one the *next* step needs.
`status:ready-for-commit` means "awaiting CI/CD"; after CI/CD has
already merged, the correct status for handing back to Systems
Engineer to record the SHA/regression note in the RTVM is
`status:ready-for-rtvm-update` (confirmed against my own prior
comments on issues #6/#7, which used exactly that label for this same
step).

**Why it matters:** stall-recovery.yml parses the `Next:` line to
route a recovered hand-off, and the label commands in step 6 are what
actually fire — a mismatched sentence doesn't break the mechanics, but
it misdescribes what happens next to every human/agent reader, and if
I'd applied `ready-for-commit` as a label too it would misdirect a
stall-recovery for real.

**How to apply:** before writing the `Next:` line on a trunk-merge
comment, check what label I'm about to *apply* in step 6 (not what
labels I inherited), and make sure the sentence names that one. If
already posted wrong, `gh api -X PATCH
repos/.../issues/comments/<numeric id>` to fix it (the numeric REST id,
not the GraphQL node id from `gh issue view ... -q '.comments[].id'`)
before applying labels, so the record and the mechanics agree.

**Recurred on issue #9 (UI-002), 2026-09-05:** drafted
`status:ready-for-test, agent:systems-engineer` again on the first
draft (muscle memory from routine test-engineer hand-offs), caught it
before finishing the label steps, corrected the label to
`status:ready-for-rtvm-update` and PATCHed the already-posted comment
before applying labels. Worth double-checking this specific line every
time on a trunk-merge comment, since the wrong instinct keeps
recurring — it's not a one-off lapse.
