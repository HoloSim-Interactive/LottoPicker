#!/usr/bin/env bash
# SPA-CGAL pipeline-health monitor (session vigil).
# Emits one line per actionable event; silence = healthy.
REPO="Holosim/SPA-CGAL"
STATE="${SPA_MONITOR_STATE:-$(dirname "$0")/monitor-state}"
mkdir -p "$STATE"
POLL=180

# Baselines (no events on first pass)
date -u +%Y-%m-%dT%H:%M:%SZ > "$STATE/since"
curl -s https://www.githubstatus.com/api/v2/summary.json 2>/dev/null \
  | jq -r '.components[] | select(.name=="Actions") | .status' > "$STATE/actions-status" || true
gh issue list --repo "$REPO" --state open --label status:needs-human --json number --jq '.[].number' 2>/dev/null | tr -d '\r' | sort > "$STATE/parked" || true
gh issue list --repo "$REPO" --state open --label status:blocked --json number --jq '.[].number' 2>/dev/null | tr -d '\r' | sort > "$STATE/blocked" || true
: > "$STATE/seen-runs"; : > "$STATE/stall-counts"; : > "$STATE/runner-miss"

while true; do
  sleep "$POLL"

  # 1) GitHub Actions component status transitions
  cur=$(curl -s https://www.githubstatus.com/api/v2/summary.json 2>/dev/null \
    | jq -r '.components[] | select(.name=="Actions") | .status' 2>/dev/null)
  if [ -n "$cur" ]; then
    prev=$(cat "$STATE/actions-status" 2>/dev/null)
    if [ "$cur" != "$prev" ] && [ -n "$prev" ]; then
      echo "ACTIONS-STATUS: $prev -> $cur (githubstatus.com)"
    fi
    echo "$cur" > "$STATE/actions-status"
  fi

  # 2) New workflow runs ending badly. Lagging 30-min window + seen-runs
  # dedup: a fixed created>since watermark misses any run that COMPLETES
  # more than one poll after creation (bit us 2026-08-26 22:58 — a real
  # relay failure at 20:25 was never reported).
  since=$(date -u -d '30 minutes ago' +%Y-%m-%dT%H:%M:%SZ)
  runs=$(gh api "repos/$REPO/actions/runs?created=>$since&per_page=30" \
    --jq '.workflow_runs[] | select(.status=="completed") | select(.conclusion=="failure" or .conclusion=="timed_out" or .conclusion=="startup_failure" or .conclusion=="cancelled") | "\(.id)\t\(.run_attempt)\t\(.name)\t\(.conclusion)\t\(.display_title)"' 2>/dev/null | tr -d '\r')
  if [ -n "$runs" ]; then
    while IFS=$'\t' read -r id attempt name concl title; do
      [ -z "$id" ] && continue
      # Dedup on id+ATTEMPT, not id alone: a re-run keeps the run's id, so
      # an id-only key silently swallows the second failure — the exact
      # case that matters, since a re-run is how you test whether a
      # failure was transient (bit us 2026-08-28, run 33187708410).
      key="$id#${attempt:-1}"
      grep -qx "$key" "$STATE/seen-runs" && continue
      echo "$key" >> "$STATE/seen-runs"
      # Concurrency-superseded and manually-culled duplicates are cancelled
      # before any job starts — pure churn, not worth a wake-up. Only report
      # a cancellation that killed real execution.
      if [ "$concl" = "cancelled" ]; then
        started=$(gh api "repos/$REPO/actions/runs/$id/jobs" \
          --jq '[.jobs[] | select(.started_at != null and .started_at != "0001-01-01T00:00:00Z")] | length' 2>/dev/null | tr -d '\r')
        [ "${started:-0}" -eq 0 ] && continue
      fi
      # CI failures are REAL signals again: the known-red register that
      # justified muting build-and-test emptied when #13 closed (2026-08-28)
      # and trunk went green. Do not re-mute without a named register entry.
      sfx=""; [ "${attempt:-1}" -gt 1 ] && sfx=" (attempt $attempt)"
      # Report GitHub's OWN failure annotation when there is one, rather
      # than leaving the cause to be guessed from the log (added
      # 2026-09-01). A self-hosted runner that loses its network is
      # annotated "The self-hosted runner lost communication with the
      # server" -- authoritative, one line, and the thing that was twice
      # misdiagnosed as usage exhaustion by grepping for strings that
      # appear in EVERY run's log, healthy or not.
      ann=""
      jid=$(gh api "repos/$REPO/actions/runs/$id/jobs" --jq '.jobs[0].id' 2>/dev/null | tr -d '')
      if [ -n "$jid" ]; then
        ann=$(gh api "repos/$REPO/check-runs/$jid/annotations"               --jq '[.[] | select(.annotation_level=="failure") | .message][0] // empty' 2>/dev/null               | tr -d '' | head -c 140)
      fi
      [ -n "$ann" ] && sfx="$sfx — $ann"
      echo "RUN-$(echo "$concl" | tr a-z A-Z): $name$sfx — ${title:0:70}"
    done <<< "$runs"
  fi
  # 3) Issues newly parked (needs-human / blocked)
  for pair in "status:needs-human parked" "status:blocked blocked"; do
    lbl=${pair% *}; f=${pair#* }
    curset=$(gh issue list --repo "$REPO" --state open --label "$lbl" --json number --jq '.[].number' 2>/dev/null | tr -d '\r' | sort)
    if [ $? -eq 0 ]; then
      new=$(comm -13 "$STATE/$f" <(echo "$curset") 2>/dev/null | grep -v '^$')
      for n in $new; do echo "PARKED: #$n gained $lbl"; done
      echo "$curset" > "$STATE/$f"
    fi
  done

  # 4) Stalled agent hand-offs: issue holds agent:* with no in-progress/pause
  #    status while no relay run is queued — the dropped-event signature.
  cand=$(gh issue list --repo "$REPO" --state open --json number,title,labels \
    --jq '.[] | select([.labels[].name] | any(startswith("agent:"))) | select([.labels[].name] | any(. == "status:in-progress" or . == "status:needs-human" or . == "status:blocked" or . == "status:on-hold" or . == "status:paused" or . == "status:waiting-on-lock" or . == "status:cancelled" or . == "status:ready-for-review" or . == "status:ready-for-client-test") | not) | "\(.number)\t\(.title)"' 2>/dev/null | tr -d '\r')
  # Count every relay run that is not finished, not just status=queued
  # (fixed 2026-08-29). A run that has STARTED is in_progress, and the old
  # query missed it: #45 was flagged HANDOFF-INCOMPLETE twice while its
  # systems-engineer session was actively running, because the suppression
  # saw zero "queued" runs and concluded nothing was happening.
  queued=$(gh api "repos/$REPO/actions/workflows/agent-relay.yml/runs?per_page=20" --jq '[.workflow_runs[] | select(.status != "completed")] | length' 2>/dev/null | tr -d '\r')
  # A candidate has TWO possible causes and they need OPPOSITE remedies:
  #   (a) the labeled event was dropped, no run ever created -> re-tap.
  #   (b) an agent DID run, finished, and cleared status:in-progress without
  #       setting a successor status or advancing the agent label -> the
  #       hand-off is half-applied; advancing it is right and RE-TAPPING IS
  #       WRONG, because it re-runs a role over work it already completed
  #       (~$1+ per duplicate session; see #23, 2026-08-27).
  # Distinguish by looking for a recently SUCCEEDED relay run carrying this
  # issue's title. Observed on #26, 2026-08-28: PM finished, posted its
  # result, removed status:in-progress, left no successor -> check 4 called
  # it a dropped event and prescribed a re-tap it must not get.
  recentok=""
  if [ -n "$cand" ]; then
    okwin=$(date -u -d '90 minutes ago' +%Y-%m-%dT%H:%M:%SZ)
    # NB: gh api --jq takes exactly one filter argument — it does NOT accept
    # --arg, and passing one fails with "accepts 1 arg(s), received 4" that
    # 2>/dev/null then hides, silently emptying the list and misclassifying
    # every candidate as a dropped event. Interpolate the window instead.
    recentok=$(gh api "repos/$REPO/actions/workflows/agent-relay.yml/runs?per_page=40" \
      --jq ".workflow_runs[] | select(.conclusion==\"success\") | select(.created_at > \"$okwin\") | .display_title" 2>/dev/null | tr -d '\r')
  fi
  newcounts=""
  while IFS=$'\t' read -r n title; do
    [ -z "$n" ] && continue
    c=$(grep "^$n " "$STATE/stall-counts" 2>/dev/null | awk '{print $2}')
    c=$(( ${c:-0} + 1 ))
    newcounts="$newcounts$n $c
"
    if [ "${queued:-0}" -eq 0 ]; then
      if [ "$c" -eq 4 ] || [ $((c % 12)) -eq 0 ]; then
        if [ -n "$title" ] && grep -Fqx "$title" <<< "$recentok"; then
          echo "HANDOFF-INCOMPLETE: #$n — a relay run for this issue SUCCEEDED recently but the issue holds an agent label with no status label. The agent finished and left the hand-off half-applied. Advance it to the next role; do NOT re-tap, that re-runs completed work."
        else
          echo "STALL: #$n holds an agent label with no relay run queued for ~$((c*3)) min and no recent successful run — labeled event likely dropped; re-tap when Actions is healthy"
        fi
      fi
    fi
  done <<< "$cand"
  printf '%s' "$newcounts" > "$STATE/stall-counts"

  # 5) Runners offline two consecutive polls (watchdog should have fixed it)
  off=$(gh api "repos/$REPO/actions/runners" --jq '.runners[] | select(.status!="online") | .name' 2>/dev/null | tr -d '\r')
  for r in $off; do
    m=$(grep "^$r " "$STATE/runner-miss" 2>/dev/null | awk '{print $2}'); m=$(( ${m:-0} + 1 ))
    [ "$m" -eq 2 ] && echo "RUNNER-OFFLINE: $r down 2 polls — watchdog has not recovered it"
    echo "$r $m"
  done > "$STATE/runner-miss.next" 2>/dev/null || true
  mv -f "$STATE/runner-miss.next" "$STATE/runner-miss" 2>/dev/null || true

  # 6) An on-hold issue whose gates have ALL closed but which nothing has
  #    released. dependency-check is the only thing that releases one, and
  #    it runs on a */10 cron that GitHub drops hard (measured 2026-08-28:
  #    four scheduled sweeps in 24h across four cron workflows, not ~430).
  #    Check 4 cannot see this — status:on-hold is in its exclusion list.
  #
  #    Predicate is RELEASABILITY, not sweeper silence. A quiet sweeper is
  #    harmless while every on-hold issue still has an open gate, and #21
  #    and #22 legitimately sit on-hold for days — keying on silence alone
  #    made this an hourly false alarm the moment #18/#20 were released.
  #    Requires two consecutive polls so a sweep already in flight is not
  #    reported as a stall.
  held=$(gh issue list --repo "$REPO" --state open --label status:on-hold \
    --json number,body --jq '.[] | "\(.number)\t\(.body | gsub("\n"; " "))"' 2>/dev/null | tr -d '\r')
  if [ -n "$held" ]; then
    closed=$(gh issue list --repo "$REPO" --state closed --limit 200 \
      --json number --jq '.[].number' 2>/dev/null | tr -d '\r')
    ready=""
    while IFS=$'\t' read -r num body; do
      [ -z "$num" ] && continue
      # Every #N on the line, not just the first (fixed 2026-08-29 with the
      # same bug in dependency-check.yml): comma-separated gate lists
      # otherwise collapse to their first element and this check reports
      # issues as releasable when their real gates are still open.
      gates=$(printf '%s' "$body" | grep -oE 'Finish-Start:.*' | grep -oE '#[0-9]+' | grep -oE '[0-9]+')
      [ -z "$gates" ] && continue          # no parseable gate: not our call
      allclosed=1
      for g in $gates; do
        grep -qx "$g" <<< "$closed" || { allclosed=0; break; }
      done
      [ "$allclosed" -eq 1 ] && ready="$ready $num"
    done <<< "$held"
    # Report a given set ONCE (2026-08-29): the old rule re-fired every 20
    # polls (~hourly) for as long as the condition held, which is spam when
    # the release is being held DELIBERATELY. A changed set is news; an
    # unchanged held set is not. Clearing the state file re-arms it.
    r=$(cat "$STATE/releasable-miss" 2>/dev/null); r=${r:-0}
    prev=$(cat "$STATE/releasable-reported" 2>/dev/null)
    if [ -n "$ready" ]; then
      r=$((r+1))
      if [ "$r" -ge 2 ] && [ "$ready" != "$prev" ]; then
        echo "RELEASABLE:$ready — every declared Finish-Start gate is closed but the issue still holds status:on-hold; dependency-check has not released it. Dispatch: gh workflow run dependency-check.yml"
        printf '%s' "$ready" > "$STATE/releasable-reported"
      fi
    else
      r=0
      # Deliberately do NOT clear releasable-reported here (2026-08-29):
      # a transient empty result (a gh hiccup, a poll landing mid-sweep)
      # otherwise re-arms the alert and the same set is reported again on
      # the next poll. A set that genuinely clears and later returns
      # unchanged is not news; STALL and HANDOFF-INCOMPLETE cover a real
      # regression. Clear the state file by hand to re-arm.
    fi
    echo "$r" > "$STATE/releasable-miss"
  fi
done
