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

**UI-001 pass (issue #6, 2026-09-05):** TP-UI-001's step 3 says "proceeds
to ranking without error" but ranking (UI-002/CORE) isn't built yet — this
is expected per the Implementation Plan's ordering, not a defect. Judge
step 3 as satisfied if the tool exits 0 with no error on a well-formed
config path, even though it doesn't yet actually rank anything. Re-check
this note once UI-002/CORE land — at that point "proceeds to ranking"
should mean real ranking output, and the looser bar no longer applies.
