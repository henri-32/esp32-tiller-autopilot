$ErrorActionPreference = "Stop"

$pio = "C:\Users\henri\.platformio\penv\Scripts\pio.exe"

if (!(Test-Path $pio)) {
  throw "PlatformIO executable not found: $pio"
}

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

function Get-CscTestCompileCommand {
  function Find-CommandInOutput($lines) {
    return $lines |
      ForEach-Object { $_.ToString().Trim() } |
      Where-Object { $_ -like "g++*test\test_csc\test_csc.cpp" -and $_ -like "* -c *" } |
      Select-Object -First 1
  }

  $output = & $pio test -e native_csc -vvv --without-testing 2>&1
  if ($LASTEXITCODE -ne 0) {
    throw "Failed to build test compile info for native_csc."
  }

  $commandLine = Find-CommandInOutput $output
  if ($commandLine) {
    return $commandLine
  }

  # Cached test build can hide compile command lines. Force a clean rebuild once.
  & $pio run -e native_csc -t clean | Out-Null
  if ($LASTEXITCODE -ne 0) {
    throw "Failed to clean native_csc before re-reading test compile command."
  }

  $output = & $pio test -e native_csc -vvv --without-testing 2>&1
  if ($LASTEXITCODE -ne 0) {
    throw "Failed to rebuild test compile info for native_csc."
  }

  $commandLine = Find-CommandInOutput $output
  if (-not $commandLine) {
    throw "Could not find test compile command for test\test_csc\test_csc.cpp in pio test output."
  }

  return $commandLine
}

function Inject-CscTestEntry {
  $dbPath = ".pio/build/native_csc/compile_commands.json"
  $db = Get-Content $dbPath -Raw | ConvertFrom-Json

  $testFile = "test\test_csc\test_csc.cpp"
  $testCommand = Get-CscTestCompileCommand

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

  $filtered = @($db | Where-Object { $_.file -ne $testFile -and $_.file -ne "test\\test_csc\\test_csc.cpp" })
  $finalDb = $filtered + $testEntry
  $finalDb | ConvertTo-Json -Depth 6 | Set-Content $dbPath -Encoding UTF8
}

Build-Compiledb -env "genericSTM32F411RE" -targetDir ".pio/build/genericSTM32F411RE"
Build-Compiledb -env "native_csc" -targetDir ".pio/build/native_csc"
Inject-CscTestEntry

# Keep the root compile_commands.json focused on the main firmware env.
& $pio run -t compiledb -e genericSTM32F411RE
