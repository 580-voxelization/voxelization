$ErrorActionPreference = "Stop"

$repo = Split-Path -Parent $MyInvocation.MyCommand.Path
$bash = "C:\msys64\usr\bin\bash.exe"

if (!(Test-Path $bash)) {
    throw "MSYS2 was not found at $bash. Install MSYS2 first, then install the UCRT64 gcc/cmake/ninja packages."
}

if ($repo -notmatch "^([A-Za-z]):\\(.*)$") {
    throw "Expected a Windows absolute path, got: $repo"
}

$drive = $matches[1].ToLowerInvariant()
$rest = $matches[2] -replace "\\", "/"
$repoMsys = "/$drive/$rest"

& $bash -lc "export PATH=/ucrt64/bin:`$PATH; cd '$repoMsys' && cmake -S . -B build -G Ninja && cmake --build build"
