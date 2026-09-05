# Kickoff Runbook

Generic, reusable version — fill in the fields below, then follow the
numbered steps. This file lives at the root of every project cloned
from `=TEMPLATE=`; there's nothing project-specific left in it until
you fill in the table.

---

## Fill in before starting

| Field | Value |
| --- | --- |
| Project Name | Lotto Picker |
| Client Name | Kyle at HoloSim Entertainment |
| Preferred Software Programming Language | C++ |
| Description of deliverable(s) | A simple CLI application that analyzes lottery numbers based on the history of lottery number picks.  The tool calculates probabilities for all combinations of numbers, ranking them based on the premise that although the probability of any number being drawn as a set of 6 numbers from a pool of 54 numbers is always 6/54, or 1/9, the probability of that number coming up in two consecutive draws (week after week) is 1/9*1/9=1/81. So that number coming up is less likely.  The likelihood of a number being picked after it was picked two weeks prior is slightly better than 1/81, but still lower than 1/9. Expand that logic going all the way back to the beginning of the lottery history. Technically, the average frequency of a specific number appearing in the history of picked number sets should be the same for every individual number. But if we follow the probability metrics described above, if we rank the digits by that probability we would likely discover a skew, which nature should eventually even out by responding with the digit that showed up least.  Expand this concept to 2-digit pairs that appear together, and 3-digit sets, 4-digit sets, 5-digit sets, and 6-digit sets.  Eventually, combining all of these probabilities should allow a person to rank all possible 6 digit sets by a single probability metric.  The goal is not to "pick the next winning lottery ticket".  The goal is to rank all possible 6-digit sets so that the next set of numbers to come up in the lottery appear somewhere near the top of that list. In addition to that, accepting the idea (however naive) that such a ranking system could be devised that would consistently capture the winning lottery pick within a particular percentage of the top of the list (e.g. the top .005%), it would be reasonable to say that the algorithm wouldn't (and couldn't) be that accurate EVERY week, but it would be sufficient for it to be consistently accurate at least once within a given weekly interval. So in other words, it might not capture the full winning lottery pick EVERY week, but it DOES consistently capture it within that window at least once every 10 weeks, for example. The game is to try and get the algorithm tuned well enough to be able to say, "If I buy a certain number of lottery tickets every week for a certain number of weeks, I am highly likely to win".  This is, of course, completely academic, since the numbers are expected to be so large that purchasing that many lottery tickets would be impractical, both in the cost of the ticket purchases, the complexity of BUYING that many tickets, and the low guarantee of return on the investment. This is not a gambling tool, this is a statistics challenge.|
| General budget (time/tokens) | Let's do this cheap.  After all, it's not like I've won the lottery. |
| User documentation needed? | No. |

**Notes on specific fields:**

- **User documentation needed?** — No

- **Client Name** — Done
- **Description of deliverable(s)** — Done
- **General budget (time/tokens)** — Done

---

## 0. Naming — check before creating anything

Done

---

## 1. Create the repository from `=TEMPLATE=`

Done


---

## 2. Secrets — none of these carry over from the template automatically

Done

---

## 3. Confirm the GitHub App covers this repo

Done

---

## 4. Confirm Actions are enabled

Done

---

## 5. Create the labels

```bash
gh auth login   # one-time, if not already authenticated
./scripts/setup-labels.sh
```

Confirm with `gh label list` — expect 22 labels, including
`agent:product-manager`.

---

## 6. Verify the credential line in `agent-relay.yml`

Open `.github/workflows/agent-relay.yml`, check the `with:` block
under the "Run Claude Code as..." step, and confirm it matches
whichever credential you chose in step 2a — exactly one of
`claude_code_oauth_token` or `anthropic_api_key` active
(uncommented), the other commented out. If it doesn't match:

```bash
git add .github/workflows/agent-relay.yml
git commit -m "Set active credential to match this project's choice"
git push
```

---

## 7. Submit the kickoff issue

On the new repo: **Issues → New issue → Project Kickoff**. This
auto-applies `agent:product-manager` the moment it's submitted.

In the "What are we building?" field, use the **Description of
deliverable(s)** value from the table above, expanded to full
sentences if it was kept brief there. If you're tracking a budget and
want it recorded as project context, mention it explicitly here too.

---

## 8. Where the interview happens, and how to answer it

The Product Manager's questions appear as a comment on the kickoff
issue — check the **Issues** tab, open the issue, scroll to comments.

To reply: **just write a plain comment** — no special mention syntax
needed. Comments don't trigger anything on their own; only labels do.
After posting your reply:

1. Open the issue's **Labels** section
2. Remove `agent:product-manager`
3. Immediately re-add `agent:product-manager`

That relabel is what wakes the agent back up to read what you wrote.
Repeat for as many rounds as the interview takes.

If a run seems to be taking a while, check the **Actions** tab for
current status before assuming something's wrong.

---

## 9. What "done" with kickoff looks like

Once scope is fully defined and confirmed, the Product Manager closes
the kickoff issue and opens a new one titled **"RTVM"**, labeled
`agent:systems-engineer` — expected behavior, not an error. That's
where requirements decomposition begins, and it proceeds on its own
from there.

## Default Task for the Human Client: UE host project (added 2026-08-27)

For any Unreal Engine plugin project, BEFORE the Generate Code Base
issue runs: confirm a barebones UE host project exists for build and
test staging, or copy `resources/ue-host-project/` into the staging
location (its README has the two commands). Agents must never spend
budget regenerating this boilerplate from scratch — the resource is
text-only, engine-pinned, and proven against UE 5.6 headless runs.
If the client prefers to generate it themselves in the editor, that
is equally fine; what matters is it exists before code generation.
