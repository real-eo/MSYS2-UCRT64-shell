# MSYS2 UCRT64 Shell

A Windows shell extension that adds an **"Open in MSYS2 UCRT64"** entry to the Windows 11 context menu. Right-clicking a folder (or empty space inside a folder) opens an MSYS2 UCRT64 terminal in that directory.

The extension is implemented as a COM in-process DLL exposing `IExplorerCommand`, registered on the modern Windows 11 context menu through a **sparse MSIX package** - the same mechanism used by VS Code, Windows Terminal, and PowerToys.

## Project overview

```text
msys2-ucrt64-shell/
├── CMakeLists.txt              # Build definition (DLL + stub exe)
├── build.bat                   # One-stop build script (configure + build)
├── src/
│   ├── dll.cpp / dll.h         # COM exports: DllGetClassObject, DllCanUnloadNow
│   ├── ClassFactory.cpp/.h     # IClassFactory implementation
│   ├── ExplorerCommand.cpp/.h  # IExplorerCommand + IObjectWithSite (the real logic)
│   ├── Guids.h                 # The CLSID, defined once
│   ├── Registration.cpp/.h     # Reserved for self-registration (currently unused)
│   └── stub.cpp                # Minimal exe required by the MSIX manifest schema
├── packaging/
│   ├── AppxManifest.xml        # Sparse package manifest (COM + context menu registration)
│   └── assets/                 # PNG logos required by the manifest
├── scripts/
│   ├── generate-certificate.ps1  # Create a self-signed signing certificate
│   ├── export-certificate.ps1    # Export it as a .pfx
│   ├── trust-certificate.ps1     # Import it into LocalMachine\Root (elevated)
│   ├── pack-msix.ps1             # Build the .msix (manifest + assets only)
│   ├── sign-msix.ps1             # Sign the .msix
│   ├── register-msix.ps1         # Register/unregister the package
│   ├── verify-package.ps1        # Check the package is installed
│   ├── reload-explorer.bat       # Restart Explorer
│   └── register.reg              # Legacy Win10-style registration (fallback)
└── dev/                        # Development notes (not shipped)
```

### How it works

```text
Right-click in Explorer
  -> Windows 11 menu reads the sparse package manifest
  -> finds the verb -> CLSID
  -> loads msys2_ucrt64_shell.dll (from the external location)
  -> DllGetClassObject -> ClassFactory -> ExplorerCommand
  -> GetTitle / GetIcon / GetState
  -> Invoke()
       items != nullptr  -> use the selected folder
       items == nullptr  -> IObjectWithSite: query Explorer's active view
                            for the current folder (background case)
  -> ShellExecuteExW(ucrt64.exe, working directory = folder)
```

The DLL implements:
- `IExplorerCommand` - title, icon, tooltip, state, invocation
- `IObjectWithSite` - lets `Invoke()` recover the current folder when Explorer invokes the command from the directory background (where no item array is supplied)
- `IClassFactory` - standard COM boilerplate
- `DllGetClassObject` / `DllCanUnloadNow` - the DLL's COM entry points

## Setup guide

Everything below is a one-time setup. Read all of it before starting; several steps have gotchas that will cost you time if missed.

### Requirements

- Windows 11 (the modern context menu requires it; the classic fallback works on Windows 10 via `scripts/register.reg`)
- Visual Studio with the C++ workload (MSVC + Windows SDK), or a standalone CMake + MSVC toolchain
- CMake 3.20 or newer
- Windows SDK (provides `makeappx.exe` and `signtool.exe`)
- MSYS2 installed (the code assumes `C:\msys64`; adjust the paths in `ExplorerCommand.cpp` if yours differs)

#### Machine-specific paths

Several files in this repository contain **absolute paths from my original machine**. They are examples, not defaults - update them to match your own system before building:
| File | Hardcoded path | What to change it to |
|---|---|---|
| `src/ExplorerCommand.cpp` | `C:\msys64\ucrt64.exe` (in `Invoke()`) | Your MSYS2 install root, e.g. `D:\msys2\ucrt64.exe` |
| `src/ExplorerCommand.cpp` | `C:\msys64\ucrt64.ico` (in `GetIcon()`) | Your MSYS2 install root, or an icon embedded in the DLL |
| `scripts/register-msix.ps1` | `C:\Users\<user>\...\build\Release` (`-ExternalLocation`) | The absolute path to **your** `build\Release` folder containing the DLL and stub exe |
| `scripts/register.reg` | `C:\Users\<user>\...\build\Debug\msys2_ucrt64_shell.dll` | The absolute path to your built DLL (legacy fallback only) |
| `scripts/register.reg` | `C:\msys64\ucrt64.ico` | Your MSYS2 install root |

The paths in `packaging/AppxManifest.xml` are relative and need no changes.

A quick way to find every machine-specific path:
```powershell
Get-ChildItem -Recurse -Include *.cpp,*.h,*.ps1,*.reg,*.xml -Path src,scripts,packaging |
    Select-String -Pattern "[A-Z]:\\"
```

### 1. Regenerate the CLSID

The CLSID in `src/Guids.h` is unique to this machine. **You must generate yourown** - a CLSID collision with another component causes silent, painful failures.
```powershell
[guid]::NewGuid()
```

Put the result in `src/Guids.h` (both the comment and the `constexpr CLSID`) and in `packaging/AppxManifest.xml` (both `desktop5:Verb Clsid` attributes and the `com:Class Id`). All four must match.

A CLSID is an identifier, not a secret - it is safe to publish.

### 2. Build as 64-bit

Explorer on 64-bit Windows is a 64-bit process and can only load 64-bit COM DLLs. A 32-bit DLL registers fine but never loads. `build.bat` always targets x64; if you configure CMake manually, use:
```bat
cmake -S . -B build -G "Visual Studio 18 2026" -A x64
```

Verify an existing build directory with:
```powershell
Select-String -Path .\build\CMakeCache.txt -Pattern "CMAKE_GENERATOR_PLATFORM"
```

It must say `x64`, not `Win32`.

### 3. Configure the MSYS2 launcher

The context menu launches `C:\msys64\ucrt64.exe` with the selected folder as its working directory. By default, the MSYS2 login scripts change the working directory to home, ignoring it. To keep the selected folder, edit `C:\msys64\ucrt64.ini` and make sure `CHERE_INVOKING` is enabled:
```ini
MSYSTEM=UCRT64
CHERE_INVOKING=1
```

Without this line the terminal opens at the MSYS2 root regardless of what the DLL passes.

### 4. Create and trust a signing certificate

The sparse package must be signed. Generate a self-signed certificate:
```powershell
.\scripts\generate-certificate.ps1
.\scripts\export-certificate.ps1
```

Then trust it (elevated PowerShell):
```powershell
.\scripts\trust-certificate.ps1
```

Gotchas:
- The certificate subject (`CN=...`) must **exactly** match the `Publisher` attribute in `packaging/AppxManifest.xml`.
- The `.pfx` contains your **private key**. It is gitignored and must never be committed or packed into the `.msix`.
- The password is dev-only; the scripts prompt for it rather than storing it.
- Anyone cloning this project should generate their **own** certificate, just like the CLSID.

### 5. Know the two packaging modes

| | Option A: sparse (active) | Option B: full package |
|---|---|---|
| Package contains | manifest + assets only | manifest + assets + DLL + stub exe |
| DLL location | build output (`-ExternalLocation`) | inside the `.msix` |
| Rebuild loop | build + re-register | build + copy + pack + sign + register |
| Registration | `Add-AppxPackage -Path ... -ExternalLocation` | `Add-AppxPackage -Path` |
| Manifest flag | `AllowExternalContent="true"` | `AllowExternalContent="false"` |

Option A is active and recommended during development. Option B is documented in comments in `AppxManifest.xml`, `build.bat`, and `scripts/register-msix.ps1` for when the DLL is complete and you want a single self-contained file for distribution.

## Build guide

### Quick start

```bat
build.bat
```

That is the whole loop for a Debug build. The script:

1. Stops Explorer (it locks the DLL, causing `LNK1168` otherwise)
2. Locates CMake (PATH first, then via `vswhere.exe`)
3. Sets up the MSVC environment (`vcvarsall.bat x64`) if `cl.exe` is absent
4. Configures CMake for x64
5. Builds
6. Restarts Explorer and exits with code 0 (success) or 1 (failure)

Available arguments:

```text
build.bat                  Debug build (default)
build.bat release          Release build
build.bat clean            Delete the build directory
build.bat rebuild          Clean, then Debug build
build.bat release rebuild  Clean, then Release build
```

Check the exit code with `$LASTEXITCODE` in PowerShell.

### What gets built

- `build\<config>\msys2_ucrt64_shell.dll` - the COM shell extension
- `build\<config>\msys2_ucrt64_shell_stub.exe` - a do-nothing exe whose only purpose is satisfying the MSIX manifest schema (the `Executable` attribute must match `*.exe`; a DLL is rejected)

### Why the COM exports look unusual

The Windows SDK already declares `DllGetClassObject` and `DllCanUnloadNow` in `combaseapi.h`. Defining functions with those names produces `error C2375: redefinition; different linkage`. The workaround in `dll.cpp` defines the implementations under internal names (`DllGetClassObjectImpl`) and aliases them at link time:
```cpp
#pragma comment(linker, "/export:DllGetClassObject=DllGetClassObjectImpl,PRIVATE")
#pragma comment(linker, "/export:DllCanUnloadNow=DllCanUnloadNowImpl,PRIVATE")
```

`PRIVATE` suppresses the `LNK4104` warning. Verify the exports after building:

```bat
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
dumpbin /exports build\Debug\msys2_ucrt64_shell.dll
```

You should see `DllCanUnloadNow` and `DllGetClassObject`.

## Packaging and registration guide

Run these after every build that changes the manifest or assets. A DLL-only rebuild only needs the register step.

```powershell
.\scripts\pack-msix.ps1      # pack manifest + assets into .msix
.\scripts\sign-msix.ps1      # sign it with the .pfx
.\scripts\register-msix.ps1  # remove old registration, register the new package
```

Then restart Explorer (`scripts\reload-explorer.bat`) and right-click a folder.

### What each step does and why

**pack-msix.ps1** runs `makeappx` with `/nv` (skip validation). This is required for a sparse package: the manifest references the DLL and stub exe, which live at the external location rather than inside the package, so the "referenced files must exist in the package" check would fail. The output goes to the repo root (not `packaging\`) so it is never packed into itself.

**sign-msix.ps1** signs with `signtool` using the `.pfx`. An unsigned package is rejected with `0x80073CFF` ("no valid license or sideloading policy").

**register-msix.ps1** removes any existing registration (`Get-AppxPackage ... | Remove-AppxPackage`) and registers the new one with `Add-AppxPackage -Path ... -ExternalLocation`. The removal step is needed because re-installing the same identity with different contents is blocked (`0x80073CFB`); the alternative is incrementing `Version` in the manifest.

### Verifying

```powershell
.\scripts\verify-package.ps1
```

`Status: Ok` and `SignatureKind: Developer` mean the package is registered. The external location must contain both `msys2_ucrt64_shell.dll` and `msys2_ucrt64_shell_stub.exe`.

### Uninstalling

```powershell
Remove-AppxPackage Msys2Ucrt64Shell
```

This cleanly removes the context menu entry - one advantage over `.reg` files.

## Development notes

### Debugging

`OutputDebugStringW` tracing works from inside Explorer. Run Sysinternals DebugView (`Dbgview.exe`, not installed by default) as administrator with Capture > Capture Win32 enabled, or attach Visual Studio to `explorer.exe`.

Key trace points: `DllGetClassObject` (Explorer found the registration), `GetTitle` (the COM object was created), `Invoke` (the command was clicked).

### Background vs. folder invocation

- Right-clicking a **folder**: Explorer supplies an `IShellItemArray`; the first item is the folder.
- Right-clicking the **background**: Explorer supplies `nullptr`. The code falls back to `IObjectWithSite`, walking `site -> IServiceProvider -> IShellBrowser -> IShellView -> IFolderView -> IShellFolder -> IPersistFolder2 -> PIDL -> IShellItemArray`.

`IShellView` has no `GetFolder` method, and `SVGIO_BACKGROUND` does not provide `IShellFolder` directly (`E_NOINTERFACE`) - the `IFolderView` `QueryInterface` step is required.

### Registry fallback (Windows 10 / classic menu)

`scripts/register.reg` registers the same CLSID through the classic `Directory\shell` verb mechanism. It appears under "Show more options" on Windows 11. Do not use both registrations simultaneously - you get duplicate entries. The stale `command` subkey from an old-style registration conflicts with `ExplorerCommandHandler`; delete the old verb keys if switching.

### Known limitations

- Windows 11 controls exact placement of first-level menu items; the entry appears where the shell decides.
- `GetIcon` currently points at `C:\msys64\ucrt64.ico`. Making the icon path dynamic (or embedding it in the DLL) is a TODO.
- `Registration.cpp` is reserved for self-registration (`DllRegisterServer`) and is currently unused; `regsvr32` will fail until it is implemented.
- `DllCanUnloadNow` always returns `S_FALSE`; a real reference count is a future improvement.

### Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| Menu item never appears | DLL not x64, CLSID mismatch, or Explorer not restarted | Rebuild as x64 (`build.bat`), verify the CLSID matches in `Guids.h` and `AppxManifest.xml`, then restart Explorer (`scripts\reload-explorer.bat`) |
| Menu item appears but does nothing | `Invoke()` failed or the launcher path is wrong | Trace with DebugView (`Dbgview.exe`) for the launch error code; verify `C:\msys64\ucrt64.exe` exists (`Test-Path C:\msys64\ucrt64.exe`) |
| `LNK1168` cannot open .dll for writing | Explorer has the DLL loaded | Restart Explorer before rebuilding (`scripts\reload-explorer.bat`), or run `build.bat`, which stops Explorer first |
| `0x80073CFF` on register | Package not signed, or certificate not trusted | Re-run `pack-msix.ps1` + `sign-msix.ps1`; trust the certificate elevated (`scripts\trust-certificate.ps1`) |
| `0x80073CFB` on register | Same identity, different contents | Remove the old package first (`Get-AppxPackage Msys2Ucrt64Shell \| Remove-AppxPackage`), or increment `Version` in `AppxManifest.xml` and re-pack/re-sign |
| `C00CE014` manifest error | Wrong COM namespace in the manifest | Use `windows.comServer` with `com:SurrogateServer`/`com:Class`, not `windows.comInterface`/`InProcessServer` (see PowerToys' manifests for a working reference) |
| `Executable` pattern error | Manifest `Executable` must match `*.exe` | Build the stub exe (`msys2_ucrt64_shell_stub.exe`) and reference it in the manifest; the DLL stays in `com:Class Path` |
| Terminal opens at MSYS2 root | `CHERE_INVOKING=1` missing from `ucrt64.ini` | Edit `C:\msys64\ucrt64.ini` and enable `CHERE_INVOKING=1` (see Setup guide, step 3) |
| `Invoke` entered but nothing happens | Launch failed inside `Invoke()` | Check DebugView for the logged error code; 2 = wrong launcher path, 193 = wrong file type, 5 = access denied |
| `0x80080204` manifest invalid | Schema violation in `AppxManifest.xml` | The error names the line/column; compare against the schema docs: https://learn.microsoft.com/en-us/uwp/schemas/appxpackage/uapmanifestschema/schema-root |
| Package registered but no menu entry | Explorer cached the old menu, or external location is missing files | Restart Explorer; verify `build\Release\` contains both `msys2_ucrt64_shell.dll` and `msys2_ucrt64_shell_stub.exe` |
| `dumpbin` is not recognized | VS tools not on PATH | Run from a Developer Command Prompt, or `call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"` first |
| PowerShell scripts blocked | Execution policy | `Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser`, or run `powershell -ExecutionPolicy Bypass -File .\scripts\<script>.ps1` |