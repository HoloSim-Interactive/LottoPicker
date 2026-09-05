---
name: data-in-101-missing-real-csv
description: DATA-IN-101 — real CSV missing (issue #10), SA ruled Verified gate is part 1 only, part 2 deferred to DELIV-900; closed on issue #27 once real CSV confirmed boundary as 1999-10-24. Reusable pattern for client-artifact-gated test clauses.
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

**Resolution (Solutions Architect ruling, 2026-09-05, issue #10):**
DATA-IN-101 reaches Verified on part 1 (synthetic-fixture era-tagging
mechanism) alone — a real-data cross-check clause was a design-time
sequencing mistake (assumed the CSV would exist by implementation
time), not a defect, and RTVM's own philosophy is synthetic/
deterministic fixtures precisely so verification doesn't depend on
real data's arrival. I updated `docs/RTVM.md`: dropped the "before
Verified" gate from TP-DATA-IN-101 part 2, reworded it as a **deferred
pre-delivery check** tracked alongside DELIV-900 (confirm/correct the
era boundary against the real CSV once the client supplies it, before
final MVP sign-off), and handed part 1 to Test Engineer normally.
**General pattern to reuse:** if a test procedure's "before Verified"
clause turns out to depend on an artifact the client controls the
timing of with no ETA, that clause almost certainly belongs decoupled
from Verified and reframed as a deferred pre-delivery/DELIV-900-style
check instead of blocking the feature — don't re-escalate to SA for
the same shape of problem next time; apply this directly.

**Closed out (2026-09-05, issue #27):** the real CSV arrived
(`data/florida_lotto_full.csv`, 4018 records, commit `be8ef7e`) and
disagreed with the working-hypothesis boundary — confirmed boundary is
**1999-10-24**, not 1999-01-01. Software Engineer corrected
`EraTable.h::eraTable()`, Test Engineer independently re-derived the
same date from raw CSV data and confirmed. I updated `docs/RTVM.md`'s
TP-DATA-IN-101 (added a "Resolved" note under the prior ruling, don't
delete history) and DELIV-900 (deferred check now closed, not
open-ended). Two flagged-but-out-of-scope design questions remain for
whoever scopes real-CSV wiring into an actual run: Double Play draws
in `_full.csv` put two draws on one date (breaks even-spacing
assumptions), and `X2`-`X5` columns are XTRA-multiplier prize data, not
drawn numbers — don't let a scorer ingest them.
