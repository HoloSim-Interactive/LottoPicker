# Test-rig janitor (created 2026-08-29, at the client's direction).
#
# WHY: every agent that needs an Unreal editor stages its own throwaway
# host project under C:\temp (SpaHost36, SpaHost37te, SpaHost22TR, ...),
# ~5.2 GB each with Binaries/Intermediate/DerivedDataCache, and none of
# them clean up. On 2026-08-29 fifty of them held 170 GB, the disk hit
# zero, and three relay runs died in job setup with "There is not enough
# space on the disk" -- #34, #36 and #37, none of which had done any work
# yet. Agents have since started tidying after themselves, but that is
# per-agent discipline recorded in memory files: it works when an agent
# remembers and fails silently when its run dies at setup, which is the
# exact case that filled the disk. This does not rely on remembering.
#
# HOW IDLE IS MEASURED, and why it is not the directory's timestamp:
# a rig's own LastWriteTime moves for incidental reasons and says nothing
# about who owned it or what it was for. Each rig instead carries a marker
# file, .spa-rig.json, written the first time this janitor sees it. The
# marker records when the rig was first observed, which issue it belongs
# to (parsed from the name), and the newest file timestamp seen inside it
# on each sweep. When that newest timestamp stops advancing, the rig is
# idle, and the marker accumulates how long it has been so. A rig in use
# is writing files; a rig that has sat unused for IdleHours is finished.
#
# Deleting a rig is safe by construction: it is a staged copy of a repo
# that is the source of truth plus rebuildable artefacts. Worst case an
# agent re-stages one, which costs about a minute.

param(
  [string] $Root       = 'C:\temp',
  [string] $Pattern    = 'SpaHost*',
  [int]    $IdleHours  = 6,
  [string] $LogPath    = "$env:TEMP\rig-janitor.log",
  [switch] $WhatIfOnly
)

$ErrorActionPreference = 'SilentlyContinue'

function Write-JanitorLog([string] $Message) {
  Add-Content -Path $LogPath -Value ("{0}  {1}" -f (Get-Date -Format o), $Message)
}

if (-not (Test-Path $Root)) { return }

# Scoped deliberately to $Root\$Pattern. The standing host project
# (C:\SpaHost), the client's demo (C:\staging\SpaDemo) and every unrelated
# project under C:\temp are out of scope and must stay that way -- this
# only ever removes rigs an agent staged for one issue.
$rigs = Get-ChildItem -Path $Root -Directory -Filter $Pattern -ErrorAction SilentlyContinue
if (-not $rigs) { return }

$now       = Get-Date
$deleted   = 0
$freedGB   = 0.0

foreach ($rig in $rigs) {
  $markerPath = Join-Path $rig.FullName '.spa-rig.json'

  # Newest write anywhere inside the rig, excluding the marker itself so
  # that stamping the marker never looks like activity.
  $newest = Get-ChildItem $rig.FullName -Recurse -File -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -ne '.spa-rig.json' } |
            Sort-Object LastWriteTime -Descending |
            Select-Object -First 1
  if (-not $newest) { continue }
  $lastActivity = $newest.LastWriteTime

  # The issue this rig was staged for, from the conventional naming
  # (SpaHost42, SpaHost36te, SpaHost22TR). Informational: it tells a human
  # reading the marker what the rig was for without opening anything.
  $issue = ''
  if ($rig.Name -match '(\d+)') { $issue = $matches[1] }

  if (Test-Path $markerPath) {
    $marker = Get-Content $markerPath -Raw | ConvertFrom-Json
  } else {
    $marker = [PSCustomObject]@{
      rig            = $rig.Name
      issue          = $issue
      firstSeenUtc   = $now.ToUniversalTime().ToString('o')
      lastActivityUtc= $lastActivity.ToUniversalTime().ToString('o')
      lastSweepUtc   = $now.ToUniversalTime().ToString('o')
      idleHours      = 0
      note           = 'Written by rig-janitor.ps1. Throwaway Unreal host staged for one issue; safe to delete when idle. Delete this file to have the rig treated as new.'
    }
  }

  # RoundtripKind, not a bare Parse: a bare Parse returns Kind=Local and
  # comparing that against a .ToUniversalTime() value makes every rig look
  # freshly active by exactly the UTC offset, so idleHours never grows and
  # nothing is ever collected. Caught in the 2026-08-29 dry run.
  $prevActivity = [datetime]::Parse($marker.lastActivityUtc, [Globalization.CultureInfo]::InvariantCulture, [Globalization.DateTimeStyles]::RoundtripKind).ToUniversalTime()
  if ($lastActivity.ToUniversalTime() -gt $prevActivity) {
    $marker.lastActivityUtc = $lastActivity.ToUniversalTime().ToString('o')
    $marker.idleHours = 0
  } else {
    $marker.idleHours = [math]::Round(($now.ToUniversalTime() - $prevActivity).TotalHours, 2)
  }
  $marker.lastSweepUtc = $now.ToUniversalTime().ToString('o')
  if (-not $marker.issue) { $marker.issue = $issue }

  $sizeGB = [math]::Round(((Get-ChildItem $rig.FullName -Recurse -File -ErrorAction SilentlyContinue |
             Measure-Object Length -Sum).Sum / 1GB), 2)

  if ($marker.idleHours -ge $IdleHours) {
    if ($WhatIfOnly) {
      Write-JanitorLog ("WOULD DELETE {0} -- idle {1} h, {2} GB, issue #{3}" -f $rig.Name, $marker.idleHours, $sizeGB, $marker.issue)
    } else {
      Remove-Item $rig.FullName -Recurse -Force -ErrorAction SilentlyContinue
      if (Test-Path $rig.FullName) {
        # Almost always an editor still holding a DLL. Leave it; the next
        # sweep gets it, and a locked rig is by definition still in use.
        Write-JanitorLog ("LOCKED, kept {0} -- idle {1} h, {2} GB (a process still holds it)" -f $rig.Name, $marker.idleHours, $sizeGB)
      } else {
        $deleted++; $freedGB += $sizeGB
        Write-JanitorLog ("deleted {0} -- idle {1} h, freed {2} GB, issue #{3}" -f $rig.Name, $marker.idleHours, $sizeGB, $marker.issue)
      }
    }
  } else {
    $marker | ConvertTo-Json | Set-Content -Path $markerPath -Encoding UTF8
  }
}

if ($deleted -gt 0) {
  $free = [math]::Round((Get-PSDrive C).Free / 1GB, 1)
  Write-JanitorLog ("sweep complete: {0} rig(s) deleted, {1} GB freed, {2} GB free on C:" -f $deleted, [math]::Round($freedGB,1), $free)
}
