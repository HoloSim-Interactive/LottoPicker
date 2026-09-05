---
name: rtvm-id-scheme-lottopicker
description: LottoPicker's RTVM ID scheme and category assignments — check before adding/renumbering line items
metadata:
  type: project
---

`docs/RTVM.md`'s skeleton fixes these category prefixes/ranges for
this project: `UI` 001–099, `DATA-IN` 100–199, `CORE` 200–299,
`DATA-OUT` 300–399, `OUT` 400–499, `NFR` 500–599, `DELIV` 900–999.
Req IDs are the prefix itself (e.g. `CORE-205`), not a generic
`RTVM-###` — feature issue titles should read `[CORE-205] Short desc`,
matching whichever category the item belongs to.

19 items populated as of 2026-09-05 (issue #2): UI-001..003,
DATA-IN-100..101, CORE-200..207, DATA-OUT-300..302, OUT-400..401,
NFR-500, DELIV-900..901. Two are approved as requirements with open
design questions explicitly deferred to SDD: CORE-206 (historical
pool-size normalization method — SN-6) and CORE-207 (comparative
statistical-strategy research — SN-5, itself a documentation
deliverable verified by Inspection, not a runtime test).

**Why:** MVP output is console-only — file/CSV export was explicitly
*not* requested and is *not* required, per client's confirmed MVP
Definition. Don't add an OUT/DATA-OUT item for file export without a
new client confirmation; it was deliberately left out, not missed.

**How to apply:** when creating `[CORE-014]`-style feature issues
during the Implementation Plan step, reuse these exact IDs — don't
renumber. CORE-206 and CORE-207 need their test procedures finalized
during/after SDD before their feature issues can include a concrete
`TP-` reference.
