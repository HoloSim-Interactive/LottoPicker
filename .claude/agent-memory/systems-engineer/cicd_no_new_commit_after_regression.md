---
name: cicd-no-new-commit-after-regression
description: CI/CD can hand back with no new SHA when regression re-verifies an already-merged state — just close the loop, don't wait for a second merge
metadata:
  type: project
---

On issue #13 (UI-003), the flow ran: SWE implements → TE passes →
SE records Verified + routes to CI/CD → CI/CD merges (e020769/v1.0.114)
and flags "needs regression testing" → SE routes to TE → TE regression
passes on already-merged trunk (9f743b5, no new commit — TE just
re-ran tests on `main`) → CI/CD hands back to SE explicitly saying "no
new commit/merge action needed, issue-13 was already an ancestor of
main, this just re-verified it."

**Why this matters:** the standard "Receiving a commit confirmation
from CI/CD" instructions assume every CI/CD comment carries a fresh
SHA to record. When CI/CD's follow-up comment instead says nothing new
landed (regression only re-verified prior state), there's no new SHA
to record — the existing Commit(s)/status entry from the first
merge is still correct. The right action is to append the regression
result to the existing status text (not overwrite the merge info) and
close the issue, since no release cut occurred and no further routing
is needed.

**How to apply:** if CI/CD's comment says "no new commit" / "already
merged, no diff" after a regression pass, don't treat it as a fresh
commit-confirmation requiring a new hand-off cycle — update the RTVM
status text to note the regression pass alongside the existing
commit/tag, comment, and close the issue directly.

**Inspection-only variant (issue #24, NFR-500):** when the RTVM item
is Inspection (no code change at all), the "commit" is the RTVM-edit
commit itself, not a feature commit. CI/CD's comment confirming "no
diff pending on main" is the same signal — just close. Keep the
Commit(s) column holding the actual SHA and the Status column holding
the narrative, matching how CORE-207 (#8) was recorded, rather than
inverting the two.
