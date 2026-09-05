---
name: inspection-type-rtvm-items
description: How to handle an RTVM line item whose test method is Inspection rather than Test, when the artifact already exists — no branch/commit needed
metadata:
  type: project
---

CORE-207 (issue #8) is an RTVM item with test method **Inspection**
(`docs/RTVM.md`), not **Test**. The Systems Engineer had already
written `docs/research/CORE-207-comparative-research.md` during the
SDD issue (per the client's SN-5 ask), and this issue existed purely
as the formal tracking/hand-off record so Test Engineer could perform
sign-off inspection against `docs/RTVM.md`'s TP-CORE-207 checklist.

**Why:** not every RTVM item produces code. Some (research summaries,
design documents, written recommendations) are verified by reading the
artifact against an explicit checklist (Inspection), not by running a
test suite. When the issue body says "already delivered ... no new
research or code is required," take that at face value rather than
inventing work.

**How to apply:** on any future Inspection-type item —
1. Read the artifact named in the issue.
2. Check it against the exact TP-* criteria in `docs/RTVM.md` line by
   line, and quote which criterion each part of the artifact satisfies
   in the hand-off comment (Test Engineer needs this to do a fast
   sign-off, since there's no test run to point to instead).
3. Skip branch/commit entirely if no file needs to change — "always
   work on your own branch" applies to actual changes; verifying an
   existing doc isn't a change.
4. Hand off straight to Test Engineer (`status:ready-for-test`) the
   same as a coded feature — Inspection sign-off still flows through
   Test Engineer, not directly to Systems Engineer/RTVM update.
