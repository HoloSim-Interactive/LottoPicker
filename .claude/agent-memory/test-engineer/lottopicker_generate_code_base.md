---
name: lottopicker-generate-code-base
description: How to verify LottoPicker's CMake/Catch2 scaffold and how the Generate Code Base issue (#5) hands off despite having no RTVM item.
metadata:
  type: project
---

LottoPicker (HoloSim-Interactive/LottoPicker) is a C++17 CLI, built with
CMake + Ninja on Linux via presets, Catch2 v3 via FetchContent.

**Verification command** (issue #5, 2026-09-05, passed):
```
cmake --preset default && cmake --build --preset default && ctest --preset default
./build/default/src/lottopicker   # prints "lottopicker 0.1.0", exit 0
```
Runner already has cmake 3.31, ninja, gcc/g++ preinstalled — no setup needed.
Full clean build (incl. fetching/building Catch2) took well under the
default 2-minute Bash timeout.

**No-RTVM-item issues still route through the normal pass path.** Generate
Code Base (and the other "first five" doc-producing issue types) don't
trace to an `docs/RTVM.md` row, but Test Engineer's standard hand-off
(`status:ready-for-rtvm-update` → `agent:systems-engineer`) still applies —
Systems Engineer is the one who decides what "no RTVM update needed"
means for that issue type (e.g. closing it directly to release the
already-created, `status:on-hold` downstream RTVM issues via
`dependency-check.yml`), not Test Engineer's call to skip.

**Regression pass on trunk (2026-09-05):** After CI/CD's fast-forward
merge to `main` (no separate merge commit — main just advanced to the
feature commits), same verification command passed cleanly from a
clean `main` checkout: 113/113 targets, 1/1 smoke test, `lottopicker`
runs. Worth checking `git log --oneline -1` on `main` matches the
commits cited in the merge comment before assuming a merge commit
exists to diff against.

**Known deferred item, not a test gate:** `docs/ci/windows-verification.yml`
exists but `.github/workflows/windows-verification.yml` does not yet — a
human needs to copy it once (GitHub App can't grant itself `workflows`
permission). Deferred to DELIV-901 per Software Engineer's comment on #5;
don't treat its absence as a failure on feature issues before then.

**Post-merge regression fast path (issue #8, 2026-09-05):** when CI/CD or
Systems Engineer hands back `status:ready-for-test` purely for
"regression testing needed" after a trunk merge (no new code, RTVM
already updated with the commit SHA recorded), just pull `main`, confirm
the RTVM row and SHA match what was reported, and rerun the standard
build/test command on trunk. On pass, hand off the normal way
(`status:ready-for-rtvm-update` → `agent:systems-engineer`) even though
RTVM is already current — Systems Engineer's fast path (per
AGENT_LABELS.md) is to no-op through to CI/CD when nothing needs
changing, not for Test Engineer to skip straight to CI/CD.

**Inspection-type RTVM items (e.g. CORE-207, issue #8, 2026-09-05) need no
build/runtime verification at all** — the TP-CORE-xxx test procedure is a
checklist against the delivered doc's own content (source count, per-source
applicability statements, named recommendation), not a command to run.
Read the doc, check each numbered criterion in `docs/RTVM.md`'s TP-CORE-xxx
literally, and hand off the same way (`status:ready-for-rtvm-update` →
`agent:systems-engineer`) as any code pass.
