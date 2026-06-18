# Sync firmware/walkie/userConfig.h into the installed RF24Audio library.
# Run after every change to userConfig.h, then recompile walkie.

$src = Join-Path $PSScriptRoot "..\firmware\walkie\userConfig.h"

$candidates = @(
    (Join-Path $env:USERPROFILE "Documents\Arduino\libraries\RF24Audio\userConfig.h"),
    (Join-Path $env:USERPROFILE "OneDrive\Documents\Arduino\libraries\RF24Audio\userConfig.h")
)

if (-not (Test-Path $src)) {
    Write-Error "Missing $src"
    exit 1
}

$dst = $null
foreach ($path in $candidates) {
    if (Test-Path (Split-Path $path -Parent)) {
        $dst = $path
        break
    }
}

if (-not $dst) {
    $dst = $candidates[0]
    New-Item -ItemType Directory -Force -Path (Split-Path $dst -Parent) | Out-Null
}

Copy-Item -Force $src $dst
Write-Host "Copied userConfig.h -> $dst"
Write-Host "Recompile firmware/walkie before upload."
