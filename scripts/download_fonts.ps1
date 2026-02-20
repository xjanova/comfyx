# Download required fonts for ComfyX (Windows PowerShell)
# These fonts are under SIL Open Font License

$FontDir = Join-Path $PSScriptRoot "..\assets\fonts"
New-Item -ItemType Directory -Force -Path $FontDir | Out-Null

Write-Host "Downloading NotoSans..." -ForegroundColor Cyan
try {
    Invoke-WebRequest -Uri "https://cdn.jsdelivr.net/gh/google/fonts/ofl/notosans/NotoSans-Regular.ttf" `
        -OutFile "$FontDir\NotoSans-Regular.ttf"
} catch {
    Write-Host "Warning: Could not download NotoSans" -ForegroundColor Yellow
}

Write-Host "Downloading NotoSansThai..." -ForegroundColor Cyan
try {
    Invoke-WebRequest -Uri "https://cdn.jsdelivr.net/gh/google/fonts/ofl/notosansthai/NotoSansThai-Regular.ttf" `
        -OutFile "$FontDir\NotoSansThai-Regular.ttf"
} catch {
    Write-Host "Warning: Could not download NotoSansThai" -ForegroundColor Yellow
}

Write-Host "Downloading JetBrainsMono..." -ForegroundColor Cyan
try {
    $zipPath = "$env:TEMP\JetBrainsMono.zip"
    Invoke-WebRequest -Uri "https://github.com/JetBrains/JetBrainsMono/releases/download/v2.304/JetBrainsMono-2.304.zip" `
        -OutFile $zipPath
    $extractPath = "$env:TEMP\JetBrainsMono"
    Expand-Archive -Path $zipPath -DestinationPath $extractPath -Force
    Copy-Item "$extractPath\fonts\ttf\JetBrainsMono-Regular.ttf" "$FontDir\" -Force
    Remove-Item $zipPath -Force
    Remove-Item $extractPath -Recurse -Force
} catch {
    Write-Host "Warning: Could not download JetBrainsMono" -ForegroundColor Yellow
}

Write-Host "`nFonts downloaded to: $FontDir" -ForegroundColor Green
Get-ChildItem $FontDir
