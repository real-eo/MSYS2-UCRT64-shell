# Register the sparse package (Option A, active).
# Re-run this after every rebuild; no makeappx/signing needed.
Add-AppxPackage -Path .\packaging\Msys2Ucrt64Shell.msix -ExternalLocation "C:\Users\reale\Code\C++\MSYS2-UCRT-shell\build\Release"

# ! OPTION B (full package): uncomment when the DLL is more complete.
# ! Also: set AllowExternalContent="false" in AppxManifest.xml, copy the DLL
# ! into packaging\, and uncomment the copy step in build.bat first.
# ? NOTE: The following two steps are already in scripts/build-msix.ps1 and scripts/sign-msix.ps1, so you can just run those instead of the two lines below.
# & "C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\makeappx.exe" pack /d .\packaging /p .\packaging\Msys2Ucrt64Shell.msix /o
# & "C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\signtool.exe" sign /fd SHA256 /a /f .\packaging\Msys2Ucrt64Shell.pfx /p <password> .\packaging\Msys2Ucrt64Shell.msix
# ? Actual registration of the full package (uncomment when doing Option B):
# Add-AppxPackage -Path .\packaging\Msys2Ucrt64Shell.msix
