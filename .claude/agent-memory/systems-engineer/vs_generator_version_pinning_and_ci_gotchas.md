---
name: vs-generator-version-pinning-and-ci-gotchas
description: DELIV-901 (issue #26) — CMake "Visual Studio" generator names are pinned to one VS release; runner-image VS upgrades break a single hardcoded preset. Fix pattern and a PowerShell here-string gotcha hit while writing it.
metadata:
  type: project
---

**The trap:** a CMake `CMakePresets.json` configure preset with
`"generator": "Visual Studio 17 2022"` only ever works on a machine that
has VS2022 installed. There is no CMake generator name meaning "whichever
Visual Studio is installed" — each release gets its own literal string.
On this project, GitHub's `windows-latest` runner migrated its installed
VS version mid-project (VS2022 → VS2026), and the sole `windows-vs2022`
preset started failing with `could not find any instance of Visual
Studio` — not a code regression, an environment one.

**Fix pattern, reusable on any project pinning a VS-generator CMake
preset:**
- Keep one preset per supported VS release side by side
  (`windows-vs2022`, `windows-vs2026`, ...) rather than swapping the
  pinned version in place — a client on the older IDE stays working.
- In any CI/detection script that needs to *configure* one of these
  automatically (not a human opening the IDE, which self-selects), collect
  every VS-generator preset from `CMakePresets.json` (read the `generator`
  field, don't guess names), sort newest-first, and try each with
  `cmake --preset <name>` until one succeeds. A wrong-version preset fails
  fast and cleanly; that's an expected skip, not a build error.
- Adding a future VS release becomes a one-line preset addition, not a
  design change.
- See `docs/SDD.md`'s Build & Toolchain Conventions (dated 2026-09-05
  entry) for the full write-up on this project, and
  [[lottopicker_sdd_decisions]] for how this fits with the rest of the
  target-platform strategy.

**PowerShell gotcha hit while implementing the detect-and-try-each-preset
loop:** a here-string (`@" ... "@`) with the closing `"@` indented (as it
will be inside a YAML `run: |` block written at normal code indentation)
is a **parse error**: "White space is not allowed before the string
terminator." Verified directly with `pwsh` before pushing. A plain
double-quoted string assigned across multiple physical lines (e.g. from a
multi-line `${{ needs.x.outputs.y }}` GitHub Actions substitution) does
NOT have this restriction — PowerShell double/single-quoted strings can
span lines fine, only here-string terminators can't be indented. Prefer
`$x = "${{ ... }}"` over a here-string when embedding a possibly-indented
GHA expression substitution in a `pwsh` step.
