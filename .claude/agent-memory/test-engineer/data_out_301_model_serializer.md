---
name: data-out-301-model-serializer
description: How TP-DATA-OUT-301 (model artifact round-trip) was verified, and confirmation of the "already-shipped via a prior issue" pattern for a second RTVM item.
metadata:
  type: project
---

DATA-OUT-301 (issue #18, 2026-09-05, passed): implementation was already
delivered in issue #16 (CORE-204) as a scope call by the Software
Engineer — `ModelSerializer::write`/`read`/`tryRead` plus
`tests/ModelSerializerTest.cpp` (6 cases tagged `[DATA-OUT-301]`) were
built there because docs/SDD.md's Interfaces & File Formats section
already fully specified the wire format and TP-CORE-204 needed a real
persist mechanism anyway. `issue-18` branch has no code diff vs
`main` (only prior memory-file commits) — confirms this is the same
"already-shipped, formally exercise the TP" pattern as CORE-207 (#8)
and the CORE-204 note in [[core_204_model_store]].

Verified: `./build/default/tests/lottopicker_tests "[DATA-OUT-301]"` →
8/8 assertions, 6/6 cases. Full suite 64/64 ctest. Read the test file
directly against TP-DATA-OUT-301's text (write-then-read-back, exact
round-trip of every score/metadata field) — fixture deliberately uses
values not exactly representable in short decimal form
(0.938026448787062, 0.01727460702622849, 1.0/3.0) to actually exercise
the bit-for-bit clause, not just equality after truncation. Also
covers empty-history artifact and missing/corrupt-file error paths
(not required by the TP text but a reasonable superset).
`clang-format --dry-run -Werror` clean on all three files.

**Pattern now confirmed twice**: an RTVM item whose file format was
fully pinned down in the SDD ahead of time is a good candidate for the
Software Engineer to implement early as part of a *different* issue's
"needs a real mechanism to be testable" requirement, then hand off the
paperwork-only issue straight to Test Engineer with no new branch
commit. Don't treat an empty `git diff main` as a red flag by itself —
check the cited prior issue/commit first (see [[core_204_model_store]]).
