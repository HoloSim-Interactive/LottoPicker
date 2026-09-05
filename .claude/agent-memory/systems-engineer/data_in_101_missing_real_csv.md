---
name: data-in-101-missing-real-csv
description: DATA-IN-101 (issue #10) escalated — real client Florida Lotto CSV doesn't exist in the pipeline yet, so TP-DATA-IN-101 part 2 can't run
metadata:
  type: project
---

`docs/SDD.md` flagged the 1988/1999 6/49→6/53 Florida Lotto era
boundary as an unconfirmed working hypothesis (secondary blog source),
to be cross-checked by Software Engineer against "the real
client-supplied CSV" during DATA-IN-101 implementation (see
[[lottopicker_sdd_decisions.md]]). On issue #10 (2026-09-05), Software
Engineer reported that check is impossible right now: **no real
historical CSV exists anywhere in the repo or pipeline.** Per
`docs/PROJECT_DEFINITION.md` SN-4, the client (Kyle) gathers and
supplies that file himself — it hasn't arrived. DELIV-900's planned
example CSV is a small *synthetic* illustrative sample, not real
history, and isn't due yet regardless.

**Why this matters:** this is a missing-input problem, not a design
ambiguity or a code defect — don't reopen the SDD's normalization
design or second-guess Software Engineer's due diligence (they also
tried public-source research and found nothing more authoritative).

**What I did:** updated `docs/RTVM.md` (DATA-IN-101 status +
TP-DATA-IN-101 part 2) to record the gap explicitly, and escalated to
Solutions Architect (`status:blocked`) rather than deciding myself
whether DATA-IN-101 can reach Verified on part 1 (synthetic-fixture
tagging logic) alone for MVP, or must wait on the client's real CSV.
That's a release/scope call bound up with the client relationship
(arguably Product Manager's territory too), not something Systems
Engineer should resolve unilaterally.

**How to apply:** if this resurfaces (SA/PM decision comes back, or
any other RTVM item ever depends on "the real client-supplied CSV"
existing in-repo before it can be verified), check whether the file
has actually landed before assuming Software Engineer can just look
harder for it — it may genuinely not exist yet. Same logic would apply
to CORE-206 normalization behavior validation if that ever wants real
multi-era data too.
