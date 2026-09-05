# docs/ci/ — workflow templates awaiting deployment

Files here are **not active**. GitHub only runs workflows from
`.github/workflows/`, and no agent in this pipeline can write there —
the GitHub App has no `workflows` permission, and it cannot be granted
because the App doesn't declare it. So these live here, agents maintain
them here, and a human copies whichever ones a project needs into
`.github/workflows/` once.

## The rule that matters

**Nothing in this folder may name a specific project.** No `.sln`
filename, no executable name, no project-specific path. Every file
here detects what's in the repo and acts accordingly.

This isn't style. Twice now a workflow was written against one
project, promoted into `=TEMPLATE=` with its project-specific values
baked in, and then shipped to the next project pointing at a solution
that didn't exist:

- `windows-verification.yml` carried `SudokuSolver.sln` into the PLC
  Emulator project, where every run failed at the first MSBuild step.
- `build-and-test.yml` carried `PlcEmulator.sln` into the Rubik's Cube
  project, and mislabeled itself against that project's requirements.

Both were caught by a human reading the file, not by anything failing
usefully — a workflow pointing at a missing solution fails in a way
that looks like a build error, not a configuration error.

**If you are editing a file here to make it work for the project
you're on, stop.** That's the failure mode. Change the detection so it
handles your case, or add a branch — but leave it project-agnostic.
Anything genuinely specific to one project belongs in that project's
own `.github/workflows/`, added by the client, and should not come
back here.

## What's here

- **`build-and-test.yml`** — the ordinary CI gate. Detects CMake and
  .NET, builds and tests on both ubuntu-latest and windows-latest.
- **`windows-verification.yml`** — deeper Windows-only evidence pass.
  Detects C++/MSBuild and C#/.NET, runs only the matching legs,
  publishes an evidence artifact. Produces evidence, never verdicts.

Neither builds engine projects — Unreal and Unity aren't on hosted
runners. A library built with CMake that an engine later consumes *is*
covered, and that's usually where a game project's testable logic
should live.
