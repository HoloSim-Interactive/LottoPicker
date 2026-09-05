---
name: real-data-gate-pattern
description: How to handle RTVM test procedures that ask to cross-check against the real client-supplied CSV, which doesn't exist yet
metadata:
  type: project
---

The real historical Florida Lotto CSV (SN-4, `docs/PROJECT_DEFINITION.md`)
does not exist anywhere in the repo/pipeline as of 2026-09-05, and has no
committed delivery date — client (Kyle) gathers and supplies it himself,
on his own schedule, no scraper being built. `docs/RTVM.md`'s own stated
test philosophy is to use synthetic/hand-constructed fixtures specifically
so verification is deterministic and doesn't depend on real-world data
(see the note above the Test Procedures section).

**Decision (issue #10, DATA-IN-101, 2026-09-05):** any test-procedure
clause that asks an agent to cross-check a design assumption against "the
actual client-supplied CSV" during implementation should NOT block that
RTVM item's Verified status if the CSV isn't available yet. Instead:
- Verify what's testable now against synthetic fixtures per the
  established RTVM philosophy.
- Track the real-data cross-check as a standing action to execute once
  the client actually supplies the CSV (realistically: at final
  integration / client acceptance, not mid-build) — note it under
  DELIV-900 or wherever Systems Engineer tracks pre-delivery checklist
  items, so it isn't lost.
- Don't re-escalate this pattern to Product Manager / ask the client to
  expedite the CSV each time it recurs — SN-4 already covers the fact
  that it's client-supplied with no fixed timeline; escalating to PM to
  chase it isn't warranted per single feature.

**Why:** blocking implementation-phase RTVM items on an artifact the
client controls the timing of, with no ETA, would stall unrelated
downstream work for no benefit — the actual code (e.g. `EraTable.h`'s
one-line era table) is already architected so a later correction is
cheap and doesn't require reopening the design.

**How to apply:** if this pattern recurs on another requirement (any
future item that says "cross-check against the real CSV" or similar),
apply the same treatment by default: verify the deterministic/synthetic
part now, defer the real-data check to the pre-delivery checklist. Only
escalate to Product Manager if the *deferred* check itself turns out to
gate something the client needs sooner (e.g. final acceptance can't
proceed at all without real data — which is a separate, larger,
already-known dependency, not this ticket's problem).
