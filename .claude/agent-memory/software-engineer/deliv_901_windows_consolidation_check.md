---
name: deliv-901-windows-consolidation-check
description: DELIV-901 consolidation check (issue #26) found windows-verification.yml still undeployed after being carried forward as "non-blocking" since Generate Code Base — it became the real gate
metadata:
  type: project
---

Issue #26 (DELIV-901, Inspection method) is the one-time consolidation
check `docs/SDD.md`'s Target-platform verification strategy calls for:
confirm `CMakePresets.json`'s `windows-vs2022` preset is correct, and
that `docs/ci/windows-verification.yml`'s evidence is available for
Test Engineer sign-off. See [[lottopicker_scaffold]] and
[[inspection_type_rtvm_items]] for the background pattern this builds
on.

**Finding:** `CMakePresets.json` was correct (verified structurally +
re-ran the Linux proxy build, 144/144 targets, 88/88 tests, per SDD's
own rationale that a green Linux build is a faithful proxy for this
codebase). But `docs/ci/windows-verification.yml` was **still sitting
in `docs/ci/`, never deployed** to `.github/workflows/` — it had been
flagged at issue #5 (Generate Code Base) and re-flagged at every
subsequent hand-off in that thread as "non-blocking, deferred to the
DELIV-901 consolidation phase." This issue *is* that phase, so the
same gap that was fine to defer everywhere else became a real blocker
here: Test Engineer can't produce real MSBuild/`windows-latest`
evidence for TP-DELIV-901 sign-off without the workflow actually
running, and it can't run undeployed.

**Why this matters:** don't assume a "non-blocking, carried forward"
note stays non-blocking forever — check at the specific issue that was
named as its resolution point. `.github/workflows/build-and-test.yml`
*had* been deployed at some point (diffed identical to its
`docs/ci/` template) — only `windows-verification.yml` was still
missing, so "check `.github/workflows/` contents directly" is more
reliable than trusting a stale memory note.

**How to apply:** on any future consolidation-type issue that names a
specific deferred item as its resolution point, re-verify the actual
current repo state (`ls .github/workflows/`, `diff` against
`docs/ci/`) rather than trusting what a past hand-off comment said was
true then. If still undeployed at the point the issue says it must
resolve, escalate to Systems Engineer with `status:blocked` — per the
issue's own wording ("flag to Systems Engineer if this template still
needs a human to deploy it") — rather than handing to Test Engineer,
who can't sign off without the evidence artifact.
