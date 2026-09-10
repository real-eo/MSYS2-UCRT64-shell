# Pack the package.
# ! OPTION A (sparse, active): `packaging\` contains ONLY the manifest + assets.
# !   The DLL and stub exe live at the external location (build output), so
# !   `/nv` is required to skip the "referenced files must be in the package"
# !   validation. Output goes OUTSIDE `packaging\` so it doesn't get packed
# !   into itself on the next run.
# !
# ! OPTION B (full package): `packaging\` additionally contains the DLL and
# !   stub exe as payload. The command is identical - `/nv` stays harmless -
# !   but validation would also pass without it. Output still goes outside
# !   `packaging\` for the same self-packing reason.
& "C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\makeappx.exe" pack `/nv` /d .\packaging /p .\Msys2Ucrt64Shell.msix /o