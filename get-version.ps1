# get-version.ps1

# extract the version
$HeaderFile = "src/AppVersion.hpp"
$AppVersion = (Get-Content $HeaderFile | Select-String -Pattern 'constexpr const char\* APP_VERSION "(.*)"' | ForEach-Object {$_.Matches.Groups[1].Value})

if ([string]::IsNullOrEmpty($AppVersion)) {
    $AppVersion = "unknown"
}

# Write the version to the GitHub Actions output
Write-Host "::set-output name=app_version::$AppVersion"