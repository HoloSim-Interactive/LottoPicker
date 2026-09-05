---
name: lottopicker-scaffold
description: CMake project layout and CI/Windows-workflow handling established at LottoPicker's Generate Code Base step (issue #5)
metadata:
  type: project
---

LottoPicker's Generate Code Base (issue #5, branch `issue-5`) set up:
`CMakeLists.txt` + `CMakePresets.json` (`default` Ninja preset,
`windows-vs2022` preset), `src/` (host `main.cpp` + `lottopicker_lib`
target — currently `INTERFACE` since no feature `.cpp` exists yet;
switch it to `STATIC` with real sources the moment the first RTVM
feature issue lands one), `include/lottopicker/`, `tests/` (Catch2 v3
via `FetchContent`, one smoke test), `.clang-format` (LLVM/100-col),
and a `README.md` skeleton with `TODO (DELIV-900)` markers for the
build/config/launch/output sections.

**Why:** `docs/SDD.md`'s Build & Toolchain Conventions section fixed
these choices; this is the first build task everything else in
`docs/IMPLEMENTATION_PLAN.md` depends on.

**Windows workflow decision:** `docs/PROJECT_DEFINITION.md` +
`docs/RTVM.md` DELIV-901 confirm Windows/Visual Studio *is* a real
target for this project, so `docs/ci/windows-verification.yml` is
needed — but no agent can push under `.github/workflows/` (the GitHub
App has no `workflows` permission). Said so plainly in the hand-off
comment rather than attempting the push; the file itself already
documents this and needs zero per-project customization (it
autodetects `.vcxproj`/`.csproj`). A human copies it once.

**How to apply:** on any future LottoPicker issue, don't rediscover
this — `lottopicker_lib` staying `INTERFACE` isn't a mistake, it's
intentional until real sources exist. Don't re-flag the Windows
workflow gap either; it's already handed off, just confirm it landed
if relevant.
