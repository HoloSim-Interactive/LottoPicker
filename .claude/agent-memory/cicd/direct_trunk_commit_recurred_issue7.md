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

**Recurred a 5th time on issue #19 (DATA-OUT-300, 2026-09-05):** SE
committed the Verified status straight to `main` (6ac2b1b) while
`issue-19` was still open with a stale ("Approved") `docs/RTVM.md`.
Merge was still clean: `issue-19` never touched `docs/RTVM.md` in its
own commits, so the 3-way merge just took main's version wholesale —
no conflict markers at all, not even a cosmetic one. This is the
mechanic worth remembering: a direct-trunk RTVM edit on a file the
branch doesn't otherwise touch *always* auto-resolves in git's favor
of main, regardless of how stale the branch's copy is. Only worry if
the branch itself also edits `docs/RTVM.md` (e.g. adding a Commit SHA
note) — that's the case that would produce a real conflict.

**Recurred a 6th time on issue #21 (OUT-400, 2026-09-05):** SE
committed the Verified status straight to `main` (`42fce49`) before
CI/CD had merged `issue-21` at all — same premature shape as issue #7.
Merge (`6cd0bfc`) was still clean and first-push-succeeded: `issue-21`
never touched `docs/RTVM.md`. No release cut this time either
(several RTVM items still Approved).

**Recurred a 7th time on issue #23 (OUT-401, 2026-09-05):** SE
committed the Verified status straight to `main` (`b260fce`) with
commit column "pending merge", before CI/CD had merged `issue-23` at
all. Merge (`8ed9543`) was still clean, first-push-succeeded, no
race: `issue-23` never touched `docs/RTVM.md`. Tagged v1.0.213, no
release cut (NFR-500/DELIV-900/DELIV-901 still Approved). Pattern is
now consistent enough across 7 issues to treat as this project's
normal workflow rather than a recurring anomaly worth re-flagging
every time — still worth noting in the memory record, but no longer
needs its own callout line in the hand-off comment unless the branch
itself also touches RTVM.md.

**Recurred an 8th time on issue #25 (DELIV-900, 2026-09-05):** SE
committed the Verified status straight to `main` (`d067fb4`) before
CI/CD merged `issue-25`. Merge (`7fcd590`) still clean, first-push
succeeded (no concurrent-merge race this time); `issue-25` never
touched `docs/RTVM.md` itself — only conflict was the usual
keep-both memory-index line in `test-engineer/MEMORY.md`. Tagged
v1.0.231, no release (DELIV-901 still Approved).
