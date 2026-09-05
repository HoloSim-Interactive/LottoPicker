# Issue label convention

This is the contract the `agent-relay` workflow runs on. Keep it in sync
with `.github/workflows/agent-relay.yml` if you change it.

## Role labels — whose turn it is

Exactly one of these should be present on an issue that's currently
assigned to an agent. Adding one is what triggers that agent's workflow
run.

- `agent:product-manager`
- `agent:solutions-architect`
- `agent:systems-engineer`
- `agent:software-engineer`
- `agent:ui-designer`
- `agent:scene-developer`
- `agent:test-engineer`
- `agent:cicd`

Never leave two `agent:*` labels on the same issue at once. If it's
unclear who should act next, that's itself a question for
`agent:product-manager`.

## Status labels — modifiers, not triggers

- `status:in-progress` — an agent run is currently active on this issue
- `status:ready-for-client-test` — a UI is wired to the real service and
  awaiting the client's hands-on verdict via Product Manager. Test
  Engineer verifies mocks; the client verifies the integrated system.
- `status:ready-for-review` — a UI design (flow, wireframes, framework
  choice) is complete and awaiting client review via Product Manager.
  Not a hand-off to Software Engineer; nothing gets built until this
  comes back approved.
- `status:blocked` — paired with whichever `agent:*` label is receiving
  an escalation (e.g. `agent:solutions-architect`, `agent:product-manager`);
  marks this as an escalation rather than a fresh assignment
- `status:ready-for-test` — implementation complete, awaiting the Test
  Engineer
- `status:ready-for-rtvm-update` — a test passed; paired with
  `agent:systems-engineer`. Signals the fast path: update the RTVM
  status for the relevant requirement, then pass straight to
  `agent:cicd` — this is not a new requirement to define.
- `status:ready-for-commit` — RTVM is current and tests passed,
  awaiting CI/CD
- `status:verified` — the linked RTVM item is closed
- `status:cancelled` — a test procedure or requirement changed
  mid-test; the in-flight test iteration is void and will restart once
  a new build is ready
- `status:needs-human` — either an automated escalation path has been
  exhausted (e.g. five consecutive fail/rebuild/retest cycles on the
  same requirement, or three automatic retries of a rate-limit/overload
  error), or the run itself couldn't execute at all
  (credit balance exhausted, invalid or revoked API key, key lacks
  model access, or a missing OIDC permission) — nothing an agent could
  act on in any of these cases. The relay stops here on purpose. A
  human reviews the thread and either resolves it directly or
  manually relabels to
  resume.
- `status:waiting-on-lock` — this issue's agent backed off after
  failing to acquire a file lock (see `docs/LOCKING.md`). A scheduled
  sweep retries it periodically; no action needed unless it's stuck
  for an unusually long time.
- `status:retry-1` / `status:retry-2` / `status:retry-3` — a run
  failed with a rate-limit or service-overload error and is being
  retried automatically, with an increasing delay (30s, 60s, 120s).
  Purely informational; no action needed unless it escalates to
  `status:needs-human` after the third attempt.
- `status:on-hold` — this issue has declared dependencies that aren't
  satisfied yet, so it carries no `agent:*` label — it isn't anyone's
  turn. A scheduled sweep (`dependency-check.yml`) checks it
  periodically and releases it once every dependency clears — to the
  role named in the issue's `Owner:` line, or to
  `agent:software-engineer` if none is declared. No action needed unless it's stuck for an
  unusually long time, in which case check whether its declared
  dependency issues actually exist and are progressing.

## Type labels

- `type:requirement` — traces to a specific RTVM line item
- `type:blocker` — a question raised by an agent, not a client-facing ask
- `type:bug`

## Title convention

Issues that trace to a requirement start with the RTVM ID:

```
[RTVM-014] Short description of the requirement
```

This makes the RTVM ID searchable across issues, commits, and PRs without
needing a label per ID.

## Issue types

Every project runs through these at minimum — not an exhaustive list,
just the baseline. Each of the first five is a single issue producing
one artifact; the sixth is many issues, one per buildable feature:

1. **Project Kickoff** — triggers Product Manager's client
   interview; produces `docs/PROJECT_DEFINITION.md`.
2. **RTVM** — Systems Engineer breaks the Project Definition down into
   requirements; produces `docs/RTVM.md`'s line items.
3. **SDD** — Systems Engineer defines system architecture, with
   Solutions Architect and Software Engineer input as needed; produces
   `docs/SDD.md`.
4. **Implementation Plan** — Systems Engineer sequences the build with
   Solutions Architect and Product Manager, most-critical-first;
   produces
   `docs/IMPLEMENTATION_PLAN.md`, and is what actually creates the
   Generate Code Base issue and every `[RTVM-014]`-style issue below,
   each with its dependencies declared.
5. **Generate Code Base** — Software Engineer's first task: the actual
   project scaffolding (a Visual Studio solution, an Unreal project,
   whatever the platform needs). No dependencies of its own; almost
   everything else depends on it.
6. **`[RTVM-014] ...`** — one issue per atomic, buildable, testable
   feature. This is where Software Engineer, Test Engineer, and CI/CD
   all work via comments and hand-offs on the *same* issue — never a
   new issue per action taken on one feature.

The first five each close themselves out and create the next one in
the chain — they don't relabel forward the way the sixth type does.

## Declaring dependencies

When Systems Engineer creates the Generate Code Base issue and the
`[RTVM-014]`-style issues during the Implementation Plan step, any
issue that isn't immediately ready to start needs its dependencies
declared in its body, under a `## Dependencies` heading:

```
## Dependencies
- Owner: agent:ui-designer
- Finish-Start: #12
- Start-Start: #15
```

**Owner** names who should receive the issue when its dependencies
clear. Required for any dependent issue that isn't Software
Engineer's — a UI Designer or Scene Developer issue gated on the ICD,
for instance. Omit it only when the owner is Software Engineer; that's
the fallback the sweep uses, so older issues without the field still
release correctly.

The inline bullet above is the preferred form, but the parser is
tolerant: a plain `Owner: agent:x` line, a bold `**Owner:**` line, or
a separate `## Owner` heading with the role beneath it all work. It
finds the word "Owner" and takes the first `agent:<role>` on that line
or the next three. Write it the way that reads best; just make sure
the role name is within a few lines of the word.

**Finish-Start** — the referenced issue must be closed first.
**Start-Start** — the referenced issue must have started (moved past
`status:on-hold`), but doesn't need to be finished — the two can
progress concurrently once both are underway.

**Both forms are read: one gate per line, or several on one line
separated by commas.** These are equivalent:

```
- Finish-Start: #12
- Finish-Start: #13
```
```
- Finish-Start: #12, #13
```

*This was not always true, and the failure was silent* (fixed
2026-08-29). The sweep's pattern anchored on the literal label and so
matched **once per line**, meaning a comma-separated list collapsed to
its first entry. On SPA-CGAL's v1.1 plan that released six issues whose
real gates were still open — including a documentation issue that began
writing up features nobody had built yet. Nothing errored; the sweep
simply believed the gates were closed. If you change this parsing,
**fail toward over-capturing**: an extra gate holds an issue, which is
recoverable, while a missed gate starts work whose prerequisites do not
exist. Anything on the line that looks like `#N` is treated as a gate,
so keep prose off dependency lines.

### Parking an issue on a human, without looking stalled

When a role finishes its turn but the next step is the **human's** — a
client decision, a manned test, an answer to a question — it keeps its
own `agent:*` label, removes `status:in-progress`, and triggers nobody.
That is correct, but on its own it is **indistinguishable from a dropped
hand-off**: an agent label with no status label is exactly the signature
a stall detector looks for, and re-tapping it re-runs a role over work it
already completed.

**Add `status:ready-for-client-test` when parking on a human.** It says
what is actually true, and every sweep already treats it as an
intentional pause: `reconcile-labels.sh` will not touch it, and a
monitor's stall check excludes it. Remove it when the human replies,
then cycle the agent label to resume.

Observed three times on SPA-CGAL (issues #26, #41, #48) before it was
written down; each cost a false stall alert and nearly cost a duplicate
agent session.

An issue with any declared dependency gets `status:on-hold` instead of
an `agent:*` label when created — `dependency-check.yml` checks it
periodically and releases it automatically to its declared `Owner:`
once every dependency clears. An issue with no dependencies (like
Generate Code Base itself) gets its owner's `agent:*` label
immediately.

## Don't guess — communicate

When you're stumped, there are two failure modes, and the pipeline
has been good at avoiding the first: inventing a plausible-looking
fix and presenting it as resolution. Keep avoiding that. Not guessing
is right.

The second failure mode is the one to watch for, because it feels like
caution and isn't: concluding that a problem you can't locate isn't
real, or quietly substituting a different problem you *can* solve and
presenting that as the answer. "I can't find the cause, so the
report must be mistaken" is a guess — about the client's evidence —
and a worse one than any fix would have been. So is "I'll change the
thing nearby and see."

What to do instead, especially when the client is asking for action
and nothing available looks likely to succeed on its own:

- **State what you can see and what you can't.** Be specific about
  the boundary: "the only declaration I can find in this repo is X;
  the compiler says there's another; it's somewhere my tools don't
  reach."
- **Lay out the options you'd consider, even the ones you're not
  confident in.** Name the trade-off of each in a sentence. "Rename
  our type — cheap, but I can't confirm it's the collision. Ask the
  client for the other declaration's location — slower, but certain.
  Both — belt and suspenders."
- **Ask for direction.** One rung up per the escalation ladder, or
  to the client through Product Manager if it's already at the top.
  Then stop.

What not to do: act on the least-bad option silently and describe it
as the fix; or leave the issue in a state that looks resolved when
it's actually parked on a question nobody was asked.

The client has said, in as many words, that they value the
conservatism. Keep it. This principle just insists that "I don't know"
be *said*, with options attached, rather than converted into "it's
fine" or "I did something adjacent."

## Escalation ladder

No role is a transparent pass-through for a question it can't answer.
The ladder: `cicd` → `test-engineer` → `software-engineer` /
`ui-designer` / `scene-developer` → `systems-engineer` →
`solutions-architect` → `product-manager` → user. Software Engineer,
UI Designer, and Scene Developer sit at the same rung — all three
escalate to Systems Engineer, none escalates to another. Their
shared boundary is the ICD Systems Engineer owns; a disagreement
among them is by definition an ICD question.
Product Manager is the only rung that talks to the human client
directly — every request for client input reaches the user through
here, regardless of which rung it originated at. When a role can't
resolve something itself:

1. **Try to resolve it first.** Don't escalate reflexively — the next
   rung up isn't always more qualified, just next in line.
2. **If you can't, escalate to the next rung up — in your own words.**
   Summarize, rephrase, or reference ("see Systems Engineer's
   questions 1–3 above") rather than forwarding verbatim. If you're
   relaying a question that already climbed from further down the
   ladder, say so, so the next rung knows this didn't originate with
   you.
3. **When an answer comes back down to you, relay it to whoever
   escalated to you** — don't just resolve your own concern and move
   on. The role that originally asked should get an answer via the
   same chain it went up, not silence.

Two deliberate exceptions skip most of the ladder:

- **Test Engineer's 5-consecutive-failure escalation** lands on
  `agent:product-manager` directly. Every rung already had its shot at
  this exact problem, repeatedly, and failed; climbing it again would
  repeat a demonstrated failure rather than add fresh consideration.
- **Infrastructure failures** (budget exhaustion, an invalid API key,
  a missing permission — see "Persisting your work" and the workflow's
  own failure handling) go straight to `status:needs-human` with no
  `agent:*` label at all, bypassing Product Manager too. These aren't
  requests for client input; they're operational problems no role,
  including Product Manager, can act on. Routing them through Product
  Manager first would just add a hop for something it can't resolve
  any faster than the human can.

Every other escalation — including Product Manager asking the
user — climbs one rung at a time.

## Notify vs. hand off

These are different actions and shouldn't be conflated:

- **Notify** — a comment addressed to a role by name, for their
  awareness. No relabel. Use this when the next *action* isn't theirs
  but they need to know something changed (e.g. Solutions Architect
  telling Systems Engineer about a scope refinement nobody asked for;
  Systems Engineer telling Software Engineer an RTVM item changed).
- **Hand off** — a comment plus a relabel, because the next action
  genuinely is theirs.

When a rule says "notify X, then notify Y" and both are real actions
someone has to take (not just awareness), treat it as two sequential
handoffs — X acts and relabels to Y — rather than trying to address two
roles' turns at once. See `status:ready-for-rtvm-update` above for the
concrete example.

## Persisting your work

Every job starts from a fresh `git checkout` and its container is
destroyed the moment the job ends — nothing local carries over to the
next run, for any role. Writing a file with Edit/Write isn't enough by
itself; if you don't commit and push it before you finish, it's gone,
not just uncommitted. This applies equally to documents (RTVM, SDD,
PROJECT_DEFINITION) and to your own `MEMORY.md` — a memory update that
happens after your last commit in a run is lost exactly the same way
a code change would be. Make committing and pushing everything you
touched the last thing you do, every run, regardless of what else
you've already committed earlier in that same run.

**When a push to `main` is rejected**, another agent pushed first —
this is normal with several issues in flight, not an error. Recover
with `git pull --rebase` and push again, never a plain `git pull`.
A plain pull creates a "Merge remote-tracking branch 'origin/main'"
commit that carries no work and buries the real commits in noise.
Rebase replays your commits on top of theirs and keeps the history
linear and readable.

## Branch convention

Every `[RTVM-014]`-style feature issue's work happens on a branch
named `issue-<number>` — issue #5 uses `issue-5`, always, no
variation. This is deterministic on purpose: every role can compute
the branch name directly from the issue number and check it out
before starting, without needing to parse it out of a comment
anywhere. Software Engineer creates it; Test Engineer and CI/CD check
it out before doing anything else on that issue. Nothing gets merged
to trunk except by CI/CD, and only once Test Engineer has signed off.

## Memory structure

Your `MEMORY.md` is an **index, not a store**. It is loaded on every
single run you ever do, so every line in it is re-read — and re-paid
for — on every future hand-off for the rest of the project. A memory
file that accumulates full explanations gets more expensive forever.

So: keep `MEMORY.md` to one line per lesson — a link plus a
one-sentence summary — and put the actual detail in its own file
alongside it:

```
.claude/agent-memory/<role>/
  MEMORY.md                          <- index, always loaded
  shallow_clone_merge_base.md        <- opened only when relevant
  rtvm_merge_conflict_parallel.md
```

An index line looks like:

```
- [Shallow-clone merge-base gotcha](shallow_clone_merge_base.md) —
  unshallow before merging branches with divergent fetch depths.
```

**When to split a lesson out:** when it needs more than a line or two
to be useful — a reproduction, a command sequence, the reasoning
behind a decision, or the specific issues where it recurred. **When
not to:** a genuinely one-line fact stays one line in the index, with
no file of its own. The split is for depth, not for tidiness.

**Detail files** should say what happened, why it matters, and what to
do differently next time. Name them descriptively in snake_case so the
index line and the filename tell the same story.

The payoff: a future run reads a short index, recognizes one line as
relevant, and opens exactly that file — instead of re-reading
everything you have ever learned in order to find the one thing that
applies.

## Hand-off mechanics

Two rules, both learned from real stalls rather than theory:

**Clear the status label you inherited.** A `status:ready-for-*` label
describes one point in the pipeline. When you hand off, the one that
routed work *to* you is no longer true — remove it before setting the
next one. Skipping this doesn't fail loudly; it accumulates, and an
issue carrying both `status:ready-for-rtvm-update` and
`status:ready-for-commit` is telling two different stories about where
it actually is.

**Never combine label edits into one `gh issue edit` call.** A single
command with several flags fails as a whole if any one label isn't
present — and `gh` does error on removing a label that isn't there.
When that happens, the `--add-label` that would have triggered the
next agent never runs, and the issue stalls with no error anywhere
visible. Use separate, individually tolerant commands
(`2>/dev/null || true` on removals), and always perform the
`agent:<next-role>` add **last**, since that's what actually fires the
relay — everything else must already be correct by the time it lands.

## Comment structure

Every comment carries two fixed elements, regardless of role or
outcome — not organic habit, a fixed structure, because tooling now
depends on it and not just readability.

**First line: every intended reader, then `this is`, then you.**

```
Systems Engineer, Software Engineer, this is Test Engineer:
Systems Engineer, this is Test Engineer:
Client, this is Product Manager:
```

List everyone who should read this — one name or several — then close
with `this is <your role>`. The `this is` marker is not optional and
not decoration: without it, a two-recipient line like "Systems
Engineer, Software Engineer:" is genuinely unreadable, because nothing
distinguishes a second recipient from the sender. Recipients first
(who needs to act), sender last (who is asking), always separated by
`this is`.

Use `Client` as the recipient when the intended reader is the human
rather than another role.

**Last line: an explicit, structured next-status line.** This is what
`stall-recovery.yml` actually parses to recover a stalled hand-off —
get the format right and recovery is reliable; get it wrong and
recovery is back to guessing. Exactly one of:

```
**Next:** `agent:<role-name>`
```
for a genuine hand-off,
```
**Next:** Continuing — <what happens next, briefly>
```
if you're keeping the issue and moving to your own next step, or
```
**Next:** Waiting on human reply
```
for the one case where nobody should be triggered until the client
answers.

### Scoped brevity protocol (client directive, 2026-09-02)

test-engineer and cicd additionally follow the brevity rules in their
role files: outcome first, ~100 words for routine reports, deltas by
reference (SHA / path / run URL), full detail preserved on failures.
Scoped to those two roles on purpose: their comments are the highest
volume, while the high-judgment roles' detail is load-bearing --
escalations and client-facing comments are never length-capped.

## Document locations

Every role's file references these; keep the paths consistent across
projects built from this template:

- `docs/PROJECT_DEFINITION.md` — Product Manager's scope
  definition: business analysis, stakeholder needs, MVP definition
- `docs/RTVM.md` — Systems Engineer's requirements traceability and
  verification matrix (plain markdown table: ID, category, requirement,
  verification method, test procedure reference, status)
- `docs/SDD.md` — Systems Engineer's software design document and
  system architecture
- `docs/IMPLEMENTATION_PLAN.md` — Systems Engineer's build sequence,
  most-critical-first, ideally with a Mermaid diagram (renders natively
  on GitHub, is close enough to UML for this purpose without needing
  separate tooling)
- `docs/LOCKING.md` — the symbolic file-locking convention; read this
  before editing any binary asset or shared document
- `VERSION` — repo root, just `MAJOR.MINOR` (e.g. `1.0`). CI/CD reads
  this and appends its own computed build number; only Systems
  Engineer or Product Manager edits it, deliberately, at the start of
  a new release cycle. See "Versioning and releases" in cicd.md.

## Handoff protocol

Every handoff:
1. Removes the acting role's `agent:*` label
2. Adds exactly one new `agent:*` label for the next role
3. Adds a relevant `status:*` label alongside it when the handoff is
   anything other than the normal next step (an escalation, a
   cancellation, an RTVM-update fast path)

The workflow's job only reacts to label-*add* events, so a handoff always
means adding the next label — removing one on its own does nothing.
