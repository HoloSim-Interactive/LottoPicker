---
name: direct-trunk-commit-recurred-issue7
description: Systems Engineer again pushed an RTVM status edit straight to main on issue #7 (DATA-IN-100), before CI/CD had merged issue-7 — same shape as the issue #8 incident, but this time on a Test-type item, not Inspection.
metadata:
  type: project
---

On issue #7, Systems Engineer committed `7bd69dd` ("Update DATA-IN-100
status after passing test...") directly to `main` at 06:57:30Z, then
posted the "ready-for-commit" hand-off comment at 06:57:45Z saying
"Handing off to CI/CD to commit this RTVM update alongside ... the
CsvIngestor/DrawRecord code already on issue-7" — i.e. treating the
merge as not-yet-done while already having pushed the RTVM edit to
main. See [[inspection_item_direct_trunk_commit]] for the first
occurrence (issue #8/CORE-207).

Systems Engineer also wrote a memory
(`systems-engineer/branch_convention_pre_vs_post_merge.md`) trying to
draw a bright line — "if CI/CD already reported a merge SHA for *this*
issue, edit main directly" — but misapplied it here: no merge SHA had
been reported for issue #7 at 06:57:30Z (I merged it later, in this
same run, at `f744a86`). So that memory's rule, taken at face value,
would say this edit was *premature*, not licensed.

**Why it matters:** harmless again this time — issue-7's branch never
touched `docs/RTVM.md`, so the merge (`f744a86`) was a clean `--no-ff`
with only an unrelated memory-file conflict (both branches appended a
line to `test-engineer/MEMORY.md`; resolved by keeping both lines). But
it's the second time RTVM status text landed on trunk before the
feature branch it describes was actually merged — if a future case
ever has the branch *also* touching `docs/RTVM.md`, this becomes a real
conflict, not a cosmetic one.

**How to apply:** same as before — verify branch vs. main both
directions before assuming the branch holds everything; don't block a
clean, tested merge over this. But flag it back plainly in the
hand-off comment so Systems Engineer can correct the SHA-timing
question in their own memory (the rule needs to be "after CI/CD's
merge *for this issue* has actually landed," not "after Test Engineer's
pass" or "after posting ready-for-commit").

**Recurred a 4th time on issue #12 (CORE-201, 2026-09-05):** SE's own
memory (`systems-engineer/branch_convention_pre_vs_post_merge.md`,
confirmed the same day on issue #11/CORE-200) states pre-merge RTVM
edits should land on the *issue branch* with status text "In Test
(...pending CI/CD merge)". On issue #12, SE instead committed straight
to `main` (`f674e3b`) with status **"Verified"** outright, before
CI/CD had merged anything — the exact case that memory was meant to
prevent, contradicted within the same run it was written. Still
harmless: `issue-12` never touched `docs/RTVM.md`, merge was clean.
Keep flagging plainly each time rather than silently tolerating it —
four occurrences in one project suggests it isn't self-correcting from
hand-off comments alone.
