$ErrorActionPreference = "Stop"

function Resolve-Pio {
  $cmd = Get-Command pio -ErrorAction SilentlyContinue
  if ($cmd -and $cmd.Source) {
    return $cmd.Source
  }

  $fallback = Join-Path $env:USERPROFILE ".platformio\penv\Scripts\pio.exe"
  if (Test-Path $fallback) {
    return $fallback
  }

  throw "PlatformIO executable not found. Add 'pio' to PATH or install it at $fallback"
}

$pio = Resolve-Pio

# Use a repo-local PlatformIO core by default to avoid stale lock/permission
# issues when switching between Windows and Linux.
if (-not $env:PLATFORMIO_CORE_DIR -or [string]::IsNullOrWhiteSpace($env:PLATFORMIO_CORE_DIR)) {
  $repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
  $env:PLATFORMIO_CORE_DIR = (Join-Path $repoRoot ".pio-core")
}

function Ensure-Dir([string]$path) {
  if (!(Test-Path $path)) {
    New-Item -ItemType Directory -Path $path | Out-Null
  }
}

function Build-Compiledb([string]$env, [string]$targetDir) {
  & $pio run -t compiledb -e $env
  Ensure-Dir $targetDir
  Copy-Item compile_commands.json (Join-Path $targetDir "compile_commands.json") -Force
}

function Test-EnvExists([string]$envName) {
  if (!(Test-Path "platformio.ini")) {
    return $false
  }
  $pattern = "^\s*\[env:$([regex]::Escape($envName))\]\s*$"
  return [bool](Select-String -Path "platformio.ini" -Pattern $pattern -Quiet)
}

function Build-CompiledbIfExists([string]$envName, [string]$targetDir) {
  if (Test-EnvExists $envName) {
    Build-Compiledb -env $envName -targetDir $targetDir
  } else {
    Write-Host "Skipping missing environment: $envName"
  }
}

function Inject-HeaderEntries([string]$dbPath) {
  if (!(Test-Path $dbPath)) {
    return
  }

  $db = Get-Content $dbPath -Raw | ConvertFrom-Json
  $baseEntry = $db | Where-Object { $_.file -match '\.cpp$' } | Select-Object -First 1
  if (-not $baseEntry -or -not $baseEntry.command) {
    return
  }

  $baseCmd = $baseEntry.command.Trim()
  $baseNoSrc = $baseCmd -replace '(\s+)([^ \t]+\.cpp)$', ''

  $headers = Get-ChildItem -Path "include" -Recurse -File -Include *.h, *.hpp
  if (-not $headers) {
    return
  }

  $headerEntries = foreach ($h in $headers) {
    $rel = Resolve-Path -Relative $h.FullName
    $rel = $rel -replace '^\.[\\/]', ''
    $cmd = "$baseNoSrc -x c++ $rel"
    [pscustomobject]@{
      command   = $cmd
      directory = (Get-Location).Path
      file      = $rel
      output    = ".pio\build\clangd\headers\$($h.BaseName).o"
    }
  }

  $filtered = @(
    $db | Where-Object {
      $_.file -notmatch '(^|[\\/])include[\\/].*\.(h|hpp)$'
    }
  )
  $finalDb = $filtered + $headerEntries
  $finalDb | ConvertTo-Json -Depth 8 | Set-Content $dbPath -Encoding UTF8
}

function Get-UnityTestFlags {
  $flags = "-DPIO_UNIT_TESTING -DUNIT_TEST -I.pio\libdeps\native_csc\Unity\src -Itest\test_csc -Itest"
  if ((Test-Path ".pio\build\native_csc\unity_config\unity_config.h") -or (Test-Path ".pio\build\native_csc\unity_config\UnityConfig.h")) {
    $flags += " -DUNITY_INCLUDE_CONFIG_H -I.pio\build\native_csc\unity_config"
  }
  return $flags
}

function Inject-CscTestEntry {
  $dbPath = ".clangd-db/native_csc/compile_commands.json"
  $db = Get-Content $dbPath -Raw | ConvertFrom-Json

  $testFile = "test\test_csc\test_csc.cpp"
  $baseEntry = $db | Where-Object {
    $_.file -eq "src\core\steering\csc\csc.cpp" -or
    $_.file -eq "src/core/steering/csc/csc.cpp"
  } | Select-Object -First 1

  if (-not $baseEntry -or -not $baseEntry.command) {
    throw "Could not synthesize native_csc test compile command from csc.cpp entry."
  }

  $baseCmd = $baseEntry.command.Trim()
  $baseNoSrc = $baseCmd -replace '(\s+src[\\/]+core[\\/]+steering[\\/]+csc[\\/]+csc\.cpp)$', ''
  $baseNoSrc = $baseNoSrc -replace '(-o\s+)\S+', '$1.pio\build\native_csc\test\test_csc\test_csc.o'
  $unityFlags = Get-UnityTestFlags
  $testCommand = "$baseNoSrc $unityFlags test\test_csc\test_csc.cpp"

  $outputPath = ".pio\build\native_csc\test\test_csc\test_csc.o"
  $outputMatch = [regex]::Match($testCommand, "-o\s+(\S+)")
  if ($outputMatch.Success) {
    $outputPath = $outputMatch.Groups[1].Value
  }

  $testEntry = [pscustomobject]@{
    command = $testCommand
    directory = (Get-Location).Path
    file = $testFile
    output = $outputPath
  }

  $filtered = @($db | Where-Object { $_.file -ne $testFile -and $_.file -ne "test\test_csc\test_csc.cpp" })
  $finalDb = $filtered + $testEntry
  $finalDb | ConvertTo-Json -Depth 6 | Set-Content $dbPath -Encoding UTF8
}

Build-Compiledb -env "genericSTM32F411RE" -targetDir ".clangd-db/genericSTM32F411RE"
Build-Compiledb -env "sim" -targetDir ".clangd-db/sim"
Build-Compiledb -env "native_csc" -targetDir ".clangd-db/native_csc"
Build-CompiledbIfExists -envName "gui" -targetDir ".clangd-db/gui"
Build-CompiledbIfExists -envName "gui_sim" -targetDir ".clangd-db/gui_sim"
Inject-HeaderEntries -dbPath ".clangd-db/genericSTM32F411RE/compile_commands.json"
Inject-HeaderEntries -dbPath ".clangd-db/sim/compile_commands.json"
Inject-HeaderEntries -dbPath ".clangd-db/native_csc/compile_commands.json"
Inject-HeaderEntries -dbPath ".clangd-db/gui/compile_commands.json"
Inject-HeaderEntries -dbPath ".clangd-db/gui_sim/compile_commands.json"
Inject-CscTestEntry

# Keep the root compile_commands.json focused on the main firmware env.
& $pio run -t compiledb -e genericSTM32F411RE
Inject-HeaderEntries -dbPath "compile_commands.json"
