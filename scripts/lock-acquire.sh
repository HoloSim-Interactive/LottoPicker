#!/usr/bin/env bash
# Acquire a symbolic lock on a file before editing it.
#
# Usage: lock-acquire.sh <path> <holder-role> <issue-number> [reason]
#
# Exit 0 = lock acquired, safe to edit <path>
# Exit 1 = lock held by someone else (and not stale) -- back off,
#          see docs/LOCKING.md
#
# This is advisory, not an OS-level lock. It's race-free anyway
# because every acquire attempt ends in a git push, and git only
# accepts one fast-forward push at a time -- the loser's push is
# rejected. That rejection is the actual enforcement mechanism; the
# lock file's existence is just the human/agent-readable record of it.

set -euo pipefail

PATH_TO_LOCK="$1"
HOLDER="$2"
ISSUE="$3"
REASON="${4:-}"

LOCK_FILE=".claude/locks/${PATH_TO_LOCK}.lock"
STALE_AFTER_MINUTES=60
MAX_ATTEMPTS=3

git config user.name  >/dev/null 2>&1 || git config user.name  "agent-relay-bot"
git config user.email >/dev/null 2>&1 || git config user.email "agent-relay-bot@users.noreply.github.com"

BRANCH="$(git rev-parse --abbrev-ref HEAD)"
if [[ "$BRANCH" == "HEAD" ]]; then
  echo "Detached HEAD -- can't determine a branch to push the lock to." >&2
  echo "Check out a branch before acquiring a lock." >&2
  exit 1
fi

# This script does `git reset --hard` below to guarantee it's comparing
# against the true remote state. That would silently destroy uncommitted
# work, so anything in the working tree gets stashed first and restored
# on every exit path. You are meant to lock BEFORE editing, but locking
# file B while holding edits to file A is legitimate, and losing those
# edits to a lock call would be a nasty, invisible failure.
STASHED=false
if [[ -n "$(git status --porcelain)" ]]; then
  git stash push --include-untracked -m "lock-acquire autostash" >/dev/null 2>&1 && STASHED=true
  if $STASHED; then
    echo "Working tree wasn't clean -- stashed it; will restore before exit." >&2
  fi
fi

restore_stash() {
  if $STASHED; then
    if git stash pop >/dev/null 2>&1; then
      echo "Restored stashed working-tree changes." >&2
    else
      echo "WARNING: could not auto-restore stashed changes -- they are safe in" >&2
      echo "'git stash list' as 'lock-acquire autostash'. Resolve by hand." >&2
    fi
  fi
}
trap restore_stash EXIT

# The autostash above protects the WORKING TREE. It does not protect local
# COMMITS, and the reset below is a hard reset onto the remote branch --
# so an agent that had committed but not yet pushed lost that work
# outright (measured on SPA-CGAL issue #13, 2026-08-29: two commits gone,
# recovered only because they were still in the object store). Refuse
# rather than destroy: a lock is never worth someone's commits.
git fetch origin >/dev/null 2>&1
AHEAD="$(git rev-list --count "origin/${BRANCH}..HEAD" 2>/dev/null || echo 0)"
if [[ "${AHEAD:-0}" -gt 0 ]]; then
  echo "REFUSING to acquire the lock: HEAD is $AHEAD commit(s) ahead of" >&2
  echo "origin/${BRANCH}, and acquiring resets hard onto the remote --" >&2
  echo "which would delete them. Push (or stash) your commits first, then" >&2
  echo "re-run. Nothing has been changed." >&2
  exit 3
fi

for attempt in $(seq 1 "$MAX_ATTEMPTS"); do
  git fetch origin >/dev/null 2>&1
  git reset --hard "origin/${BRANCH}" >/dev/null 2>&1

  if [[ -f "$LOCK_FILE" ]]; then
    ACQUIRED_AT="$(jq -r '.acquired_at // empty' "$LOCK_FILE" 2>/dev/null || true)"
    if [[ -z "$ACQUIRED_AT" ]] || ! date -u -d "$ACQUIRED_AT" +%s >/dev/null 2>&1; then
      # Unreadable or truncated lock file -- a half-written artifact, not a
      # real claim. Treat it as abandoned rather than crashing the caller.
      echo "Lock file at $LOCK_FILE is unreadable -- treating as abandoned." >&2
      AGE_MIN=$(( STALE_AFTER_MINUTES + 1 ))
    else
      AGE_MIN=$(( ($(date -u +%s) - $(date -u -d "$ACQUIRED_AT" +%s)) / 60 ))
    fi
    if (( AGE_MIN < STALE_AFTER_MINUTES )); then
      CURRENT_HOLDER="$(jq -r '.holder' "$LOCK_FILE")"
      CURRENT_ISSUE="$(jq -r '.issue' "$LOCK_FILE")"
      echo "Locked by $CURRENT_HOLDER (issue #$CURRENT_ISSUE), age ${AGE_MIN}m -- not stale." >&2
      exit 1
    fi
    echo "Existing lock is ${AGE_MIN}m old (> ${STALE_AFTER_MINUTES}m) -- treating as abandoned." >&2
  fi

  mkdir -p "$(dirname "$LOCK_FILE")"
  jq -n \
    --arg path "$PATH_TO_LOCK" \
    --arg holder "$HOLDER" \
    --arg issue "$ISSUE" \
    --arg reason "$REASON" \
    --arg now "$(date -u +%Y-%m-%dT%H:%M:%SZ)" \
    '{path: $path, holder: $holder, issue: ($issue|tonumber), acquired_at: $now, reason: $reason}' \
    > "$LOCK_FILE"

  git add "$LOCK_FILE"
  git commit -m "Lock: $PATH_TO_LOCK ($HOLDER, issue #$ISSUE)" >/dev/null

  if git push origin "HEAD:${BRANCH}" >/dev/null 2>&1; then
    echo "Lock acquired: $PATH_TO_LOCK" >&2
    exit 0
  fi

  echo "Push rejected on attempt $attempt -- someone likely raced us. Retrying." >&2
  sleep $(( attempt * 3 ))
done

echo "Could not acquire lock on $PATH_TO_LOCK after $MAX_ATTEMPTS attempts." >&2
exit 1
