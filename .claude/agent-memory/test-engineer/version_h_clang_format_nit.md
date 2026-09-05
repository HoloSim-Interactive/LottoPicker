---
name: version-h-clang-format-nit
description: include/lottopicker/Version.h fails clang-format --dry-run -Werror; pre-existing since scaffolding, not a regression.
metadata:
  type: project
---

`clang-format --dry-run -Werror` flags `include/lottopicker/Version.h`
(the `kVersion` line and closing namespace comment) on every run,
going back to the original scaffolding commit `b5f93dc` — it has never
been touched by any feature branch since.

**Why:** Confirmed via `git log --oneline --all -- include/lottopicker/Version.h`
on trunk as of 2026-09-05 (`main` @ `c44ed31`): only two commits ever
touched the file, both scaffolding/RTVM-metadata, neither a
reformatting change. This is almost certainly a clang-format version
drift between whatever generated the scaffold and the version installed
in this environment, not something any Software Engineer change caused.

**How to apply:** When running `clang-format --dry-run -Werror` as part
of a build/regression pass, expect this file to show up. Don't count it
against whichever issue you're testing — note it as a pre-existing,
non-blocking side observation (as done in issue #10's trunk regression
comment) rather than a fail. If it ever stops appearing or a *new* file
starts failing similarly, that's worth flagging for real.
