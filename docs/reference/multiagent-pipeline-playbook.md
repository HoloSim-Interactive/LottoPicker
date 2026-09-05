# Multi-agent pipeline playbook

Operational knowledge distilled from three projects run on this
label-relay pipeline (RubiksCubeSolver, DungeonMaster, SPA-CGAL),
written 2026-08-27 at the client's request so it survives outside any
one advisor session's memory. Everything here was learned by being
bitten at least once. Companion: `ue-headless-on-this-runner.md` for
the engine-specific command patterns.

## How the relay actually moves

- **Labels are the only trigger.** `agent:<role>` *labeled* events fire
  `agent-relay.yml`; comments never trigger anything. A hand-off that
  posts a "Next:" comment and sets a status label but leaves the agent
  label already-present fires **nothing** — adding a label that is
  already on the issue produces no event. The fix ("tap the glass") is
  remove + re-add the agent label.
- **After any tap, verify a run was CREATED** (`gh api
  .../workflows/agent-relay.yml/runs`). The label edit succeeding
  proves nothing; GitHub can drop or delay the event.
- **During a GitHub Actions incident (check
  githubstatus.com/api/v2/summary.json before debugging your own
  workflows): do NOT re-tap.** Events are usually *delayed* (5–25 min
  observed), not dropped, and every tap adds another event to the
  backlog. When the incident cleared on 2026-08-26, one issue received
  three full agent sessions — one per backlogged event. Wait for
  operational status plus a stall check, then tap once.
- **A gate that parses wrong fails silently, and it fails toward
  releasing.** `dependency-check` reads `Finish-Start:` / `Start-Start:`
  lines out of the issue body; before 2026-08-29 its pattern matched
  once per line, so `Finish-Start: #22, #32, #33` collapsed to `#22`
  and the sweep released six issues whose real gates were open — one of
  them a documentation issue that started describing unbuilt features.
  No error, no warning: the sweep simply believed the gates were closed.
  Two habits follow. **Audit a new plan's gate lines before releasing
  it** (`gh issue view N --json body` and count the `#N`s against what
  the plan intends), and when a batch of issues is released at once,
  **spot-check that the ones which moved were supposed to move** —
  a documentation or verification issue going in-progress while the
  features it covers are still open is the visible symptom.
- **The agent-label add must be the LAST label edit of a hand-off.**
  The per-issue concurrency group holds one pending run and the newest
  event wins it: adding `agent:<role>` and then swapping status labels
  lets the later status-label event supersede and auto-cancel the real
  pickup, after which the survivor skips on the job-if. The issue then
  wears the agent label with nothing running (observed on #13,
  2026-08-27). Recovery is the standard tap.
- **Duplicate runs**: per-issue `concurrency` groups auto-cancel
  stacked *pending* runs (newest wins) but never an in-progress one.
  After an event backlog flushes, cull redundant pending/queued runs
  with `gh run cancel` — keep exactly one per issue, and check the
  in-progress run's role isn't already superseded by the thread.
- **A supersede-on-dispatch sweep can be starved by impatience.**
  `dependency-check` uses `cancel-in-progress: true`, so dispatching it
  again while one is queued cancels the queued one and restarts the
  wait. When all runners are busy the sweep sits queued for minutes —
  that is normal. Dispatch once, then wait for a runner.
- **Diagnosing a dead issue**: add an inert label (e.g. `question`).
  If even that creates no (skipped) run, event delivery is broken —
  it is never your workflow file.

## Scoring automation runs (the vacuous-pass family)

- `Automation RunTests` exits **255** on a tree with *no matching
  test* — indistinguishable by exit code from a failing test. Every
  automation leg asserts an **executed-test count**, never an exit
  code alone.
- `ctest` running a single aggregated test binary exits 1 for one
  failing case and for many — grade on executed counts plus the
  failing-case set matched exactly against a known-red register
  (Systems-Engineer-maintained, each entry naming the issue that will
  grade it).
- Editor stdout is filtered by default: read log expectations from
  `<host>/Saved/Logs/<Host>.log` **with a named positive-control line**
  proving the log was captured, or an "expected: absent" check passes
  vacuously.
- Where the expectation is a *message a user reads*, grade by
  displaying the string the product emits — a green assertion only
  proves the product matched the test's own copy.

## Diagnosing a failed run

- **Diagnose a failed run from its GitHub ANNOTATION, not from grepping
  the log.** `gh api repos/O/R/check-runs/<job-id>/annotations` gives the
  authoritative one-line cause; a self-hosted dropout reads "The
  self-hosted runner lost communication with the server."
- **`out_of_credits` in a run log does NOT mean out of credits.** Every
  run, including every successful one, emits a `rate_limit_event`
  carrying `rateLimitType`, `overageStatus: rejected` and
  `overageDisabledReason: out_of_credits` — these describe the account's
  *overage configuration*, not this run's fate. The only discriminating
  field is `rate_limit_info.status` (`allowed` vs `rejected`). Two
  network dropouts were reported to the client as usage exhaustion
  before his own dashboard (4% weekly used) exposed it, and the relay's
  classifier had the same bug, so it would have mislabelled them
  unattended.
- **Steps left `in_progress`/`pending` on a job whose conclusion is
  `failure` mean the job was KILLED, not that it errored.** Look for a
  runner dropout, a machine sleeping, or resource starvation — not for a
  fault in the work.

## Windows / tooling traps

- Bash heredocs eat one level of backslashes; `C:\path` becomes
  `C:path`. Use forward slashes, the Write tool, or byte construction.
- `gh` on Windows emits CRLF — pipe through `tr -d '\r'` before
  comparing or interpolating anything.
- MSYS path conversion mangles `/...` args to Windows exes; set
  `MSYS_NO_PATHCONV=1` for steps calling WSL.
- The `runner` context is invalid in job-level `env:` (step-level
  only); a workflow that uses it there validates as broken with **no
  jobs created** and the run named by its file path.
- Runner `.env` must keep `C:\Program Files\Git\bin` first on PATH —
  installing WSL shadows `bash.exe` and every `shell: bash` step
  breaks with mangled paths.
- **Shallow clones lie about ancestry**: at the graft boundary,
  `git log --format=%P` reports a merge as having *no parents* —
  deepen before any ancestry or parent check.
- A PowerShell process-filter matching its own command line
  (`CommandLine -like '*watchdog*'`) kills its own shell; exclude
  `$PID` or build the pattern by concatenation.

## Budget levers (client's standing cost controls)

- **Runner concurrency** is the primary lever: one `agents` runner =
  one Claude session at a time; a second doubles throughput and spend.
  Re-registering an install between repos is the switch.
- **Pause**: every queued issue sits `status:on-hold` with no agent
  label; disabling `dependency-check.yml` freezes the release of new
  issues without touching in-flight work. Re-enable + dispatch to
  resume. `status:paused`/`pause-all` for a hard stop.
- **Usage exhaustion is self-healing**: the relay failure handler
  parks issues (`status:needs-human` + reset time) when
  `rate_limit_info.status` is *not* `allowed`; `usage-reset-resume`
  (cron) un-parks after the window resets. Classify from the local
  `execution_file` output, never from `gh run view --log` mid-run — and
  never from the ambient overage fields (see *Diagnosing a failed run*).
- **Comment discipline**: concise comments (~100 words unless ruling
  on something), "Readers, this is <Role>:" headers, one bolded
  **Next:** line — stall-recovery parses it.
- Long-running turns: the relay's `timeout-minutes` must exceed the
  longest legitimate turn or the kill skips the failure classifier
  entirely and the issue strands with no park comment. Raised 30 -> 90
  -> 180 on SPA-CGAL; a measurement or adoption round legitimately runs
  for hours.

## Monitoring (advisor-session side)

- Poll GitHub's *view* of runners, not process existence — a listener
  can be alive with a dead connection; the watchdog restarts on
  GitHub-reported-offline only.
- A run-watch keyed on `created > watermark` misses any run that
  *completes* after the watermark passes it. Use a lagging window
  (30 min) plus seen-id dedup.
- Report cancellations only when a job actually started — concurrency
  supersedes and deliberate culls are churn.
- Silence = healthy. Alert on: failed/killed runs, `needs-human` /
  `blocked` appearing, an agent-labeled issue with nothing queued
  (~12 min), runners offline past the watchdog's window.

## Client-relations facts that shape everything above

- The client pre-approves delegated outcomes — do not re-ask a
  delegated question.
- Escalations travel **with numbers**; measured tables get one-pass
  rulings (issue #9's 431-component escalation was ruled same-day
  because every step was measured, failing tests left failing).
- Bars are never quietly relaxed; remove the artefact, not the bar.
- Trunk merges only through CI/CD after a Test Engineer PASS, with
  regression on the merged head; Verified = every leg passed **and**
  the code is on trunk.
