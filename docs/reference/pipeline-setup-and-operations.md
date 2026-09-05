# Multi-Agent Delivery Pipeline — Setup and Operations

How to stand up a new project on the HoloSim multi-agent pipeline, how
the workstation that runs it is put together, and how the monitoring
assistant is operated.

Written 2026-08-31 from three completed projects (RubiksCubeSolver,
DungeonMaster, SPA-CGAL). SPA-CGAL shipped v1.0 and v1.1 in four days
on exactly this setup.

**Companions, not duplicated here:**

| Document | Covers |
| --- | --- |
| `KICKOFF_RUNBOOK.md` | GitHub project creation, step by step, with the exact commands |
| `.github/AGENT_LABELS.md` | Label protocol, comment format, dependency declarations |
| `docs/reference/multiagent-pipeline-playbook.md` | Operational traps, each learned by being bitten |
| `docs/LOCKING.md` | The symbolic file-lock convention for shared documents |
| `docs/reference/console-advisor-brief.md` | The operating brief for the monitoring seat — point a fresh console session at it |

---

## 1. What the pipeline is

A software delivery lifecycle run by AI agents, coordinated through
GitHub Issues. Each agent plays one engineering role. Work moves between
roles by **labels**, not by conversation.

| Role | Owns |
| --- | --- |
| Product Manager | The client's voice. Scope, stakeholder needs, priority. Every request for client input reaches the human through here. |
| Solutions Architect | Macro-architecture and high-level technical approach. Resolves escalations from the Systems Engineer. |
| Systems Engineer | Requirements, the RTVM, all project documentation, test procedures. |
| Software Engineer | Application code. |
| UI Designer | Interface flows and the code that renders them. |
| Scene Developer | 3D worlds, materials, lighting, level layout. |
| Test Engineer | Executes test procedures. Reports pass/fail. Runs trunk regression after merges. |
| CI/CD | Branch strategy, merges, version tags. Only stable, tested code reaches trunk. |

**The core mechanic:** adding an `agent:<role>` label to an issue fires
a GitHub Actions workflow that runs one Claude Code session as that
role. The session reads the issue, does the work, posts a comment, and
hands off by changing labels. Comments never trigger anything — only
labels do.

**Four documents are the project's spine**, produced in this order and
owned by the roles above: `PROJECT_DEFINITION.md` (scope),
`RTVM.md` (requirements and how each is verified), `SDD.md`
(architecture and decisions), `IMPLEMENTATION_PLAN.md` (build sequence).
Every requirement carries an ID; every test procedure mirrors its
requirement's number; every commit subject carries the ID it serves.

---

## 2. Prerequisites

**On the workstation:**

- Windows 11, Git for Windows, GitHub CLI (`gh`), authenticated
- Claude Code installed natively (path matters — see §4.3)
- A Claude subscription with enough headroom (see §7 on budget)
- Whatever the project's own toolchain needs (for Unreal work: UE 5.6,
  Visual Studio 2022 with Desktop C++, CMake 3.20+)

**On GitHub:**

- An account or organisation that can host the repository
- A fine-grained personal access token for the relay (§3.3)

---

## 3. Part A — Standing up the GitHub project

`KICKOFF_RUNBOOK.md` in the template repository is the authoritative
step-by-step and should be followed directly. This is the shape of it so
a reader knows what they are committing to.

### 3.1 Create the repository from the template

Copy the `=TEMPLATE=` repository. It carries the workflows, the label
definitions, the agent role definitions, the four document skeletons,
the locking scripts, and a barebones Unreal host project for plugin work.

### 3.2 Create the labels

`scripts/setup-labels.sh` creates the full set. Three families:

- `agent:*` — whose turn it is. **The only trigger.**
- `status:*` — what state the work is in (`in-progress`, `blocked`,
  `ready-for-test`, `ready-for-commit`, `on-hold`, `needs-human`,
  `waiting-on-lock`, `ready-for-client-test`, …)
- `type:*` — requirement, bug, blocker

### 3.3 Secrets

> **Plan trap (2026-09-02, HoloSim-Interactive):** on a **Free** GitHub
> organization, org-level Actions secrets are NOT available to private
> repositories -- the "Private repositories" option is greyed out with
> "cannot be used by private repositories with your plan", and the API
> still reports `visibility: all`, which is misleading. Every relay run
> in a private repo then fails at auth ("Claude Code is not installed on
> this repository"). Two fixes: add `RELAY_TOKEN` and
> `CLAUDE_CODE_OAUTH_TOKEN` as **repository** secrets on each private
> repo (works on any plan; one minute per project), or upgrade the org
> to Team, after which org secrets reach private repos (HoloSim-AIM is
> on Team, which is why it never hit this). Check `gh api orgs/<org>
> -q .plan.name` before assuming org secrets will flow.
>
> **Transfer trap (same day):** repository secrets DO survive a repo
> transfer -- but a fine-grained PAT's *resource owner* does not follow
> the repo. A `RELAY_TOKEN` minted against the personal account keeps
> working only for personally-owned repos; after the repo moves into an
> org, every `gh` call fails with `GraphQL: Could not resolve to a
> Repository` (the token cannot see org-owned repos, so GitHub reports
> them as nonexistent). After any transfer, re-mint the PAT with the
> new owner as resource owner and update the secret's value. Also
> confirm the Claude GitHub App is installed on the new owner
> (`gh api orgs/<org>/installations`) -- the relay's app-token exchange
> fails with "Claude Code is not installed on this repository" until it
> is.

Two, and neither carries over from the template automatically:

| Secret | What it is | Why not the default token |
| --- | --- | --- |
| `CLAUDE_CODE_OAUTH_TOKEN` | Claude Code authentication | — |
| `RELAY_TOKEN` | Fine-grained PAT with Issues + Contents write | Labels added with the default `GITHUB_TOKEN` **do not fire new workflow runs**, which silently breaks every hand-off |

That second row is the single most common setup failure. If hand-offs
never trigger, check this first.

### 3.4 Verify Actions are enabled, then submit the kickoff issue

The kickoff issue starts the Product Manager's client interview. The
human answers in the issue thread; the PM writes
`PROJECT_DEFINITION.md`; the Systems Engineer follows with the RTVM,
then the SDD and the implementation plan; those produce the feature
issues, and the pipeline runs itself from there.

### 3.5 A default task for the human, before code generation

For Unreal projects: confirm a barebones UE host project exists for
build and test staging, or copy `resources/ue-host-project/` into place.
It is text-only and takes a minute by hand; agents regenerating it from
scratch costs real budget for no benefit.

---

## 4. Part B — The workstation

Everything below runs on one physical machine. GitHub-hosted runners are
not used: the projects need a real Unreal Engine install, a real GPU,
and long-running builds.

### 4.1 Runner installs

Three self-hosted GitHub Actions runners, each its own directory:

| Directory | Runner name | Label | Serves |
| --- | --- | --- | --- |
| `C:\actions-runner` | `holosim-agents-01` | `agents` | Agent hand-offs |
| `C:\actions-runner-agents2` | `holosim-agents-02` | `agents` | Agent hand-offs (second lane) |
| `C:\actions-runner-ci` | `holosim-ci-01` | `ci` | Builds and verification |

**Labels name a PURPOSE, never a toolchain.** All three share one
machine, so a capability label could not distinguish them, and a tool
name would be a fossil the day the project changes.

**Concurrency is the primary budget lever.** Two `agents` runners means
two Claude sessions at once — roughly double throughput and double
spend. Dropping to one serialises everything. Switching is a
re-registration, not a rebuild.

**Registering a runner to a repository:**

```powershell
cd C:\actions-runner
$reg = gh api -X POST repos/OWNER/REPO/actions/runners/registration-token -q .token
.\config.cmd --unattended --url https://github.com/OWNER/REPO `
  --token $reg --name holosim-agents-01 --labels agents --work _work --replace
```

**Moving one between repositories** (a runner belongs to exactly one):
stop the listener *and its parent `run.cmd` shell* — the shell relaunches
the listener otherwise — then `config.cmd remove --token <old-repo
remove-token>` and register again as above.

### 4.2 Keep-alive: the launcher and the watchdog

**`start-actions-runner.cmd`** in the user's Startup folder launches all
three listeners plus the watchdog at logon.

**`C:\actions-runner\watchdog.ps1`** polls GitHub every 3 minutes and
restarts any runner **GitHub reports offline**.

The reason it polls GitHub rather than checking for a process: a
listener can stay alive while silently losing its connection. Observed
once for 32 minutes — the process was running, logged nothing, and
GitHub considered the runner offline the whole time. Process-existence
checks cannot see that; only GitHub's own view can. Logs to
`%TEMP%\runner-watchdog.log`.

### 4.3 Known workstation traps

Each of these cost real time at least once.

| Trap | Symptom | Fix |
| --- | --- | --- |
| PATH ordering | Every `shell: bash` step fails with mangled `C:actions-runner...` paths after installing WSL | Each runner's `.env` must list `C:\Program Files\Git\bin` **first** |
| `runner` context in job-level `env:` | Workflow validates as broken, **no jobs created**, run named by file path | Use it at step scope only |
| MSYS path conversion | `/mnt/c/...` args to Windows exes get mangled | `MSYS_NO_PATHCONV: 1` |
| `gh` emits CRLF | Interpolated issue numbers carry a stray `\r` | Pipe through `tr -d '\r'` |
| Bash heredocs eat a backslash level | `C:\path` becomes `C:path`; `'\r'` becomes a literal CR | Forward slashes, or build with `chr(92)` |
| Shallow clones lie about ancestry | `git log --format=%P` reports a merge as having no parents | Deepen before any ancestry check |
| Claude Code installer | "Windows is not supported by this script" | Point the action at the installed binary via `path_to_claude_code_executable` |

### 4.4 Disk hygiene: the test-rig janitor

Agents that need an editor stage a throwaway host project per issue
(~5 GB each) and do not clean up. Fifty of them once filled the disk and
killed three runs in job setup with "not enough space on the disk".

**`C:\actions-runner\rig-janitor.ps1`** runs hourly from inside the
watchdog loop, in its own try/catch so it can never take runner recovery
down with it.

Each rig gets a marker file, **`.spa-rig.json`**, at its root:

```json
{
  "rig": "SpaHost42",
  "issue": "42",
  "firstSeenUtc": "...",
  "lastActivityUtc": "...",
  "lastSweepUtc": "...",
  "idleHours": 18.9
}
```

Idle is measured from the **newest file write anywhere inside the rig**,
excluding the marker itself, so stamping never looks like activity. A rig
being written to never ages; one idle past the threshold (default 6
hours) is deleted. A rig locked by a running editor is kept and retried —
locked means in use. Deletion is safe by construction: a rig is a staged
copy of a repository that is the source of truth, plus rebuildable
artefacts.

It is **local PowerShell only** — no Claude usage, no Actions minutes.

Dry run before trusting a threshold change:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File C:\actions-runner\rig-janitor.ps1 -WhatIfOnly
```

---

## 5. Part C — The monitor

The monitor is a shell script polling GitHub every 3 minutes, run as a
background task by the human's Claude Code console session. It emits one
line per **actionable** event; silence means healthy. It exists because
the pipeline's failure modes are mostly silent.

**Location:** `C:\actions-runner\spa-pipeline-monitor.sh` — deliberately
outside any session's scratch directory so it survives.
A reference copy is vendored at `resources/spa-pipeline-monitor.sh` in
this template, alongside `resources/watchdog.ps1` and
`resources/rig-janitor.ps1`.

**Running it** (as a persistent background task, with its own state dir
so two operators cannot corrupt each other's baselines):

```bash
SPA_MONITOR_STATE="C:/actions-runner/monitor-state-console" \
  bash "C:/actions-runner/spa-pipeline-monitor.sh"
```

### 5.1 What it watches

| Alert | Means | Usual action |
| --- | --- | --- |
| `RUN-FAILURE` / `RUN-TIMED_OUT` | A workflow run failed | Read the log; classify |
| `RUN-CANCELLED` | A run that had **started** was killed | Investigate — pending culls are filtered out |
| `PARKED` | An issue gained `needs-human` or `blocked` | Read the escalation |
| `STALL` | Issue holds an agent label, nothing queued ~12 min, no recent success | The labeled event was dropped — re-tap |
| `HANDOFF-INCOMPLETE` | Same, but a run for it **succeeded** recently | Advance to the next role; **do not re-tap** — that re-runs completed work |
| `RELEASABLE` | Every declared gate is closed but the issue is still on-hold | Dispatch `dependency-check` |
| `SWEEPER-SILENT` | The cron sweeps have not fired | Dispatch manually |
| `RUNNER-OFFLINE` | A runner is down past the watchdog's window | Check the host |
| `ACTIONS-STATUS` | GitHub Actions changed state | See §5.3 |

### 5.2 Why each check is shaped the way it is

These are corrections, each from a real miss:

- **Failed-run detection uses a lagging 30-minute window with
  seen-ID dedup**, not a forward watermark. A watermark misses any run
  that *completes* after it passes.
- **Dedup keys on run ID + attempt.** A re-run keeps the ID, so an
  ID-only key swallows the second failure — the one that matters, since
  a re-run is how you test whether a failure was transient.
- **Cancellations are only reported if a job actually started.**
  Concurrency supersedes and deliberate culls are churn.
- **Stall suppression counts every relay run that is not `completed`,
  not just `queued`.** A run that has started is `in_progress`; counting
  only `queued` flags actively-running issues as stalled.
- **`RELEASABLE` reports each distinct set once**, and does not clear
  its memory on a transient empty result — otherwise a hiccup re-arms
  the same alert every cycle.

### 5.3 Operating rules the monitor cannot enforce

- **After any tap, verify a run was created.** The label edit succeeding
  proves nothing.
- **During a GitHub Actions incident, do not re-tap.** Events are usually
  *delayed* (5–25 minutes observed), not dropped, and every tap adds to
  the backlog. One issue once received three full agent sessions when an
  incident's backlog flushed. Check
  `githubstatus.com/api/v2/summary.json` before debugging your own
  workflows.
- **The agent-label add must be the LAST label edit of a hand-off.** The
  per-issue concurrency group holds one pending run and the newest event
  wins it; adding the agent label and *then* swapping status labels lets
  the status event supersede and cancel the real pickup.
- **`dependency-check` supersedes itself.** It uses
  `cancel-in-progress: true`, so dispatching again while one is queued
  cancels it and restarts the wait. Dispatch once, then wait for a runner.

### 5.4 The assistant's standing role

The console session running the monitor is not only a watcher. Its
standing brief:

1. **Tap the glass** — restore dropped hand-offs, cull duplicate runs,
   dispatch stuck sweeps, without asking each time.
2. **Carry client context into escalations.** Agents rule well when
   given the human's own words; the assistant posts those to the issue
   so a ruling lands in one pass rather than three.
3. **Escalate to the human only what genuinely needs them** — a scope
   decision, a bar that would have to move, a manned test.
4. **Never merge, never relax a bar, never delete without asking.**

The habits behind all four — and the failure modes they exist to catch —
are written out in `docs/reference/console-advisor-brief.md`.

---

## 6. Part D — Operating rhythm

### 6.1 Daily

Nothing, when it is working. Issues release as their gates close, roles
hand off, CI/CD merges, and the monitor is silent.

### 6.2 Pausing (before a shutdown, or to conserve budget)

```bash
for w in agent-relay dependency-check lock-retry stall-recovery usage-reset-resume; do
  gh workflow disable "$w.yml" --repo OWNER/REPO
done
```

Then cancel any in-flight `agent-relay` runs and note which issues hold
`agent:*` labels. Powering off mid-run severs sessions; the work is
recoverable but wasted.

### 6.3 Resuming

Re-enable the same five, then **cycle** (remove and re-add) the agent
label on each issue that held one, verifying a run appears for each. For
issues merely sitting `on-hold`, one `dependency-check` dispatch is
enough. Re-arm the monitor.

### 6.4 Budget levers

| Lever | Effect |
| --- | --- |
| Runner concurrency | One `agents` runner halves parallel spend |
| Model pinning | Reserve the strongest model for the few genuinely mathematical issues; everything else on the default. Set as a list in `agent-relay.yml`'s `claude_args` |
| Workflow pause | Freezes new work without touching what is in flight |
| Comment brevity | Agents write to each other constantly; verbosity is a real line item |
| Console session length | Every turn re-sends the whole conversation. `/clear` at phase boundaries, leaning on memory files instead |

### 6.5 Usage exhaustion is self-healing

The relay's failure handler classifies rate-limit exhaustion, parks the
issue with `status:needs-human` and the reset time, and the
`usage-reset-resume` sweep un-parks it once the window resets. Classify
from the run's local execution output, never from `gh run view --log`
mid-run — that returns empty.

---

## 7. Troubleshooting quick reference

| Symptom | First thing to check |
| --- | --- |
| Hand-offs never fire at all | `RELAY_TOKEN` is a real PAT, not `GITHUB_TOKEN` |
| One issue stopped moving | Was a run *created*? If not, re-tap. If a run succeeded, advance instead |
| Nothing releases from `on-hold` | Dispatch `dependency-check`; the crons drop far more often than they fire |
| An issue released too early | Check its gate line parses — every `#N` on it, not just the first |
| Every `shell: bash` step broke | PATH ordering in the runner's `.env` |
| Work vanished after acquiring a doc lock | `lock-acquire.sh` hard-resets onto the remote; it now refuses when HEAD is ahead, but push before locking |
| Most commits in a burst never got verified | CI concurrency must key on the SHA, not the ref |
| Runs die in job setup | Disk. Check the janitor's log |
| CI red on every push | Is there a known-red register entry? A deliberately failing test makes the whole suite red |

---

## 8. What this does not cover

- **Project-specific build commands** — those belong in the project's
  own `CLAUDE.md`.
- **The agent role definitions** — `.claude/agents/*.md` in the template.
- **How to write a good kickoff interview** — `KICKOFF_RUNBOOK.md` §8.
