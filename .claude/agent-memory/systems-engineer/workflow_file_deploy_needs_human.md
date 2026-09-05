---
name: workflow-file-deploy-needs-human
description: No agent (any role) can create/update files under .github/workflows/ — GitHub rejects the push outright; confirmed by direct test, not just theory.
metadata:
  type: project
---

Confirmed by direct test on issue #26 (DELIV-901, 2026-09-05): committing and
pushing a new file to `.github/workflows/` (copying
`docs/ci/windows-verification.yml` there) is rejected by GitHub itself at
push time:

> refusing to allow a GitHub App to create or update workflow
> `.github/workflows/<name>.yml` without `workflows` permission

This is a platform-level restriction on the GitHub App token every agent in
this pipeline runs as — it is **not** role-specific. Software Engineer hit
the same wall at issue #5 and deferred it; Systems Engineer independently
re-confirmed it at #26 rather than trusting the carried-forward note. Do not
assume a different role or a different git technique (rebase, separate
branch, etc.) will get around it — the rejection happens at `git push`,
before any PR/merge step, and is unconditional.

**Why:** `docs/ci/*.yml` templates exist specifically so a human can deploy
them with one `cp` + push when a new workflow is needed — this is
deliberate, not an oversight to route around.

**How to apply:** If a task requires adding or modifying a file under
`.github/workflows/`, don't retry with a different agent or approach — go
straight to `status:needs-human` with **no** `agent:*` label at all
(bypassing Product Manager too), per `.github/AGENT_LABELS.md`'s
infrastructure-failure exception. State exactly which file needs to be
copied/edited and from where, so the human's action is a single copy-paste,
not a research task.
