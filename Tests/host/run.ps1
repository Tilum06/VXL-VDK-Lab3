$ErrorActionPreference = 'Stop'
$project = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$build = Join-Path $project '.build-rewrite-tests'
New-Item -ItemType Directory -Path $build -Force | Out-Null
$sources = @('global', 'scheduler_on', 'software_timer', 'button', 'traffic_light',
             'led_7seg', 'mode_controller', 'fsm_automatic', 'fsm_manual', 'fsm_setting') |
    ForEach-Object { Join-Path $project "Core/Src/$_.c" }
& gcc -std=c11 -Wall -Wextra -Werror -I $PSScriptRoot -I (Join-Path $project 'Core/Inc') `
    (Join-Path $PSScriptRoot 'test_lab4.c') @sources -o (Join-Path $build 'test_lab4.exe')
if ($LASTEXITCODE -ne 0) { throw 'Host compilation failed' }
& (Join-Path $build 'test_lab4.exe')
if ($LASTEXITCODE -ne 0) { throw 'Host tests failed' }
