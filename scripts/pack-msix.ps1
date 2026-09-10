# Pack (manifest + assets only - no DLL). /nv skips semantic validation,
# required for sparse packages: the stub exe/DLL live at the external
# location, not inside the package.
& "C:\Program Files (x86)\Windows Kits\10\bin\10.0.26100.0\x64\makeappx.exe" pack /nv /d .\packaging /p .\packaging\Msys2Ucrt64Shell.msix /o