---
name: ci-template-detection-must-not-assume-committed-artifacts
description: docs/ci/ templates that detect toolchains by file presence (e.g. *.vcxproj) miss CMake-generated equivalents that don't exist until configure time — detect by reading config fields (generator), not by guessing filenames.
metadata:
  type: project
---

Found on issue #26 (DELIV-901, 2026-09-05), after the workflow-file deploy
itself was resolved (see [[workflow_file_deploy_needs_human]]): once
`windows-verification.yml` actually ran, `detect` reported `cpp=none`
because it only looked for a committed `*.vcxproj`. This project's C++
deliverable is CMake-first (`docs/SDD.md`'s `windows-vs2022` preset) and
never commits a `.sln`/`.vcxproj` — CMake generates them at configure time,
which is the normal convention, not a gap in the deliverable.

**Why:** a detection script that only recognizes committed build artifacts
will silently skip any project that generates its build files instead —
exactly the CMake-with-VS-generator case this SDD chose. The failure mode
looks like a passing-but-empty run (`detect: success`, `cpp-verify:
skipped`), not an error, so it's easy to miss without inspecting the run's
own summary output.

**How to apply:** when a `docs/ci/*.yml` template's detection step tests
for a toolchain, check whether that toolchain can also exist in a
generated-not-committed form (CMake presets generating a `.sln`, `.csproj`
scaffolding tools, etc.) and detect via the *source config's* declared
intent (e.g. a `CMakePresets.json` preset's `generator` field) rather than
only the generated file. Keep the fix generic per `docs/ci/README.md`'s
rule — read the field, don't hardcode a preset name — since the next
project's preset naming will differ. Fixed here by adding a
`cmake_vs_preset` output to `detect` (reads `generator` via `jq`) and a
`cmake --preset <name>` configure step ahead of the existing
discover-a-`.sln`-and-build-it logic in `cpp-verify`, so nothing downstream
needed to change.
