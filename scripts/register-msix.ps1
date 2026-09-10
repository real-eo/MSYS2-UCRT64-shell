# ? Remove any existing registration first - same identity + different contents is blocked (0x80073CFB) 
# ? unless the old package is removed or the version in AppxManifest.xml is incremented.
Get-AppxPackage Msys2Ucrt64Shell | Remove-AppxPackage

# Register the sparse package (Option A, active).
# ! The package contains only the manifest + assets; the DLL and stub exe live at the external location (build output).
# ! Re-run after every rebuild of the DLL. No makeappx/signing needed unless the manifest itself changed (then re-run 
# ! `pack-msix.ps1` + `sign-msix.ps1`). Requires: `.\Msys2Ucrt64Shell.msix` (packed + signed), and `build\Release\`
# ! containing `msys2_ucrt64_shell.dll` + `msys2_ucrt64_shell_stub.exe`.
Add-AppxPackage -Path .\Msys2Ucrt64Shell.msix -ExternalLocation "C:\Users\reale\Code\C++\MSYS2-UCRT-shell\build\Release"

# ! OPTION B (full package): use this line instead when the DLL is more complete.
# ! Also: set `AllowExternalContent="false"` in `AppxManifest.xml`, copy the DLL and stub exe into `packaging\`, and 
# ! uncomment the copy step in `build.bat` first. A full package is self-contained: no `-ExternalLocation` needed.
# Add-AppxPackage -Path .\Msys2Ucrt64Shell.msix