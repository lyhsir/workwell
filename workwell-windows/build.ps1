# WorkWell for Windows - PowerShell 构建脚本
# 使用方法：在 PowerShell 中运行 .\build.ps1

$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  WorkWell for Windows - 构建脚本" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 检查 CMake
Write-Host "[1/3] 检查环境..." -ForegroundColor Yellow
try {
    $cmakeVersion = cmake --version 2>&1
    Write-Host "✓ CMake 已安装" -ForegroundColor Green
} catch {
    Write-Host "✗ 未找到 CMake，请先安装 CMake" -ForegroundColor Red
    Write-Host "  下载地址: https://cmake.org/download/" -ForegroundColor Gray
    Read-Host "按回车键退出"
    exit 1
}

# 创建构建目录
Write-Host ""
Write-Host "[2/3] 配置项目..." -ForegroundColor Yellow
if (!(Test-Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
}
Set-Location "build"

# 配置 CMake
Write-Host "  运行: cmake .. -G 'Visual Studio 17 2022' -A x64" -ForegroundColor Gray
cmake .. -G "Visual Studio 17 2022" -A x64 2>&1 | ForEach-Object {
    if ($_ -match "error") {
        Write-Host "  $_" -ForegroundColor Red
    } else {
        Write-Host "  $_" -ForegroundColor Gray
    }
}

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "✗ CMake 配置失败" -ForegroundColor Red
    Set-Location ..
    Read-Host "按回车键退出"
    exit 1
}

# 编译
Write-Host ""
Write-Host "[3/3] 编译 Release 版本..." -ForegroundColor Yellow
Write-Host "  运行: cmake --build . --config Release" -ForegroundColor Gray
cmake --build . --config Release 2>&1 | ForEach-Object {
    if ($_ -match "error|warning") {
        Write-Host "  $_" -ForegroundColor Yellow
    } else {
        Write-Host "  $_" -ForegroundColor Gray
    }
}

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "✗ 编译失败" -ForegroundColor Red
    Set-Location ..
    Read-Host "按回车键退出"
    exit 1
}

# 完成
Set-Location ..
Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "✓ 构建完成！" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "可执行文件位置:" -ForegroundColor Cyan
Write-Host "  build\bin\Release\WorkWell.exe" -ForegroundColor White
Write-Host ""

if (Test-Path "build\bin\Release\WorkWell.exe") {
    $fileSize = (Get-Item "build\bin\Release\WorkWell.exe").Length / 1KB
    Write-Host "文件大小: $([math]::Round($fileSize, 2)) KB" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "运行程序:" -ForegroundColor Cyan
    Write-Host "  .\build\bin\Release\WorkWell.exe" -ForegroundColor White
}

Write-Host ""
Read-Host "按回车键退出"
