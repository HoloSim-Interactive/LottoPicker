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

Done

---

## 6. Verify the credential line in `agent-relay.yml`

DOne

---

## 7. Submit the kickoff issue



---

## 8. Where the interview happens, and how to answer it

Done

---

## 9. What "done" with kickoff looks like

DOne

## Default Task for the Human Client: UE host project (added 2026-08-27)

N/A
