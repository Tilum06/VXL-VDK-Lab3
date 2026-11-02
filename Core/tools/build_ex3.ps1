param(
    [string]$ToolchainBin = $env:STM32_TOOLCHAIN_BIN
)

$ErrorActionPreference = 'Stop'
$coreRoot = Split-Path -Parent $PSScriptRoot
$projectRoot = Split-Path -Parent $coreRoot
$buildRoot = Join-Path $coreRoot '.build-ex3'

if ($ToolchainBin) {
    $gcc = Join-Path $ToolchainBin 'arm-none-eabi-gcc.exe'
    $size = Join-Path $ToolchainBin 'arm-none-eabi-size.exe'
    $objcopy = Join-Path $ToolchainBin 'arm-none-eabi-objcopy.exe'
} else {
    $gcc = (Get-Command arm-none-eabi-gcc -ErrorAction Stop).Source
    $size = Join-Path (Split-Path $gcc) 'arm-none-eabi-size.exe'
    $objcopy = Join-Path (Split-Path $gcc) 'arm-none-eabi-objcopy.exe'
}

function Invoke-Checked([string]$Program, [string[]]$Arguments) {
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) {
        throw "$Program failed with exit code $LASTEXITCODE"
    }
}

New-Item -ItemType Directory -Force -Path $buildRoot | Out-Null
$includes = @(
    (Join-Path $coreRoot 'Inc'),
    (Join-Path $projectRoot 'Drivers/STM32F1xx_HAL_Driver/Inc'),
    (Join-Path $projectRoot 'Drivers/STM32F1xx_HAL_Driver/Inc/Legacy'),
    (Join-Path $projectRoot 'Drivers/CMSIS/Device/ST/STM32F1xx/Include'),
    (Join-Path $projectRoot 'Drivers/CMSIS/Include')
)
$common = @('-mcpu=cortex-m3', '-mthumb', '-mfloat-abi=soft', '-g3', '-O0',
    '-ffunction-sections', '-fdata-sections', '-DUSE_HAL_DRIVER', '-DSTM32F103x6')
foreach ($include in $includes) { $common += "-I$include" }

$sources = @(
    Get-ChildItem (Join-Path $coreRoot 'Src') -Filter '*.c'
    Get-ChildItem (Join-Path $projectRoot 'Drivers/STM32F1xx_HAL_Driver/Src') -Filter '*.c'
)
$objects = @()
foreach ($source in $sources) {
    $object = Join-Path $buildRoot ($source.BaseName + '.o')
    Invoke-Checked $gcc ($common + @('-std=gnu11', '-Wall', '-Werror',
        '-c', $source.FullName, '-o', $object))
    $objects += $object
}

$startup = Join-Path $coreRoot 'Startup/startup_stm32f103c6ux.s'
$startupObject = Join-Path $buildRoot 'startup_stm32f103c6ux.o'
Invoke-Checked $gcc ($common + @('-c', $startup, '-o', $startupObject))
$objects += $startupObject
$elf = Join-Path $buildRoot 'lab3.elf'
$map = Join-Path $buildRoot 'lab3.map'
$linkerScript = Join-Path $projectRoot 'STM32F103C6UX_FLASH.ld'
Invoke-Checked $gcc (@('-mcpu=cortex-m3', '-mthumb', '-mfloat-abi=soft',
    '--specs=nano.specs', '--specs=nosys.specs', '-static', "-T$linkerScript",
    '-Wl,--gc-sections', "-Wl,-Map=$map", '-Wl,--cref', '-o', $elf) +
    $objects + @('-Wl,--start-group', '-lc', '-lm', '-Wl,--end-group'))
Invoke-Checked $size @($elf)
Invoke-Checked $objcopy @('-O', 'ihex', $elf, (Join-Path $buildRoot 'lab3.hex'))
Invoke-Checked $objcopy @('-O', 'binary', $elf, (Join-Path $buildRoot 'lab3.bin'))
Write-Output "Full firmware build passed: $elf"
