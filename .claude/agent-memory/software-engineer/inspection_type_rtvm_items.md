---
name: inspection-type-rtvm-items
description: How to handle an RTVM item (Inspection method, or Test method already satisfied by a prior scope call) when the artifact/code already exists — no branch/commit needed
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

**Second occurrence, different flavor — DATA-OUT-301 (issue #18, a
Test-method item, not Inspection):** [[core_204_model_store]] recorded
a deliberate scope call in issue #16 to implement DATA-OUT-301's
`ModelSerializer` (and its dedicated `ModelSerializerTest.cpp`) early,
because CORE-204's own test procedure needed a real load/persist
mechanism to be checkable. When #18 was later released (Finish-Start
dependency on #16 satisfied), there was nothing left to build: I
rebuilt from a clean tree, ran `lottopicker_tests "[DATA-OUT-301]"`
(6 cases / 8 assertions, all pass) plus the full suite as a regression
check, and handed off with no new branch/commit — same shape as the
Inspection case above, just arrived at via an earlier scope decision
instead of the RTVM's test-method column. General lesson: before
assuming a Test-method RTVM item needs new code, check whether a prior
issue's memory already claims it as a scope-overlap delivery.
