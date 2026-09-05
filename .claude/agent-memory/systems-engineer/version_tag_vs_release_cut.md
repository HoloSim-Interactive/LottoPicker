---
name: version-tag-vs-release-cut
description: CI/CD tagging every merge (v1.0.x) does not by itself mean a release completed — only notify PM when CI/CD explicitly says a release was cut
metadata:
  type: project
---

On issue #15 (CORE-202), CI/CD's commit-confirmation comment reported
a merge tagged `v1.0.138` but explicitly stated "RTVM not fully
Verified across the table ... no release cut, tag only." The
role instructions say to notify the Product Manager "if CI/CD's
comment also states a new version number" — but LottoPicker's CI/CD
tags essentially every merge to main as part of its normal versioning
scheme, not just release cuts.

**Why this matters:** treating every tagged merge as a release
notification would spam the Product Manager with noise on routine
merges, defeating the purpose of that notification being a genuine
milestone signal.

**How to apply:** only comment to the Product Manager when CI/CD's
comment says a release was actually cut (look for explicit language
like "release cut" / "release" vs. "tag only"). A bare version tag
with no release language is not itself the trigger — read CI/CD's own
characterization of the merge, don't infer from the tag's presence
alone.
