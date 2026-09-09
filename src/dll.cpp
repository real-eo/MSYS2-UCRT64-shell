#include <windows.h>

#include "ClassFactory.h"
#include "Guids.h"

// ? Since the linker pragmas preform the exports, we don't need to use `__declspec(dllexport)` here. 
extern "C" // __declspec(dllexport)
HRESULT STDAPICALLTYPE DllGetClassObjectImpl(REFCLSID rclsid, REFIID riid, void** object) {
    if (object == nullptr)
        return E_POINTER;

    *object = nullptr;

    if (rclsid != CLSID_Msys2Ucrt64Shell)
        return CLASS_E_CLASSNOTAVAILABLE;

    auto* factory = new ClassFactory();

    HRESULT result = factory->QueryInterface(riid, object);
    factory->Release();

    return result;
}

// ? Since the linker pragmas preform the exports, we don't need to use `__declspec(dllexport)` here. 
extern "C" // __declspec(dllexport) 
HRESULT STDAPICALLTYPE DllCanUnloadNowImpl() {
    // ? Returning `S_FALSE` is fine temporarily. Later, it will be replaced with a real global object/server-lock count.
    return S_FALSE;
}



// * NOTE: The following two lines are required to avoid the `C2375` error.
/* // ! ERROR:
    === Building [Debug] ===
    MSBuild version 18.8.2+ce25c0108 for .NET Framework

      dll.cpp
    C:\Users\reale\Code\C++\MSYS2-UCRT-shell\src\dll.cpp(7,24): error C2375: 'DllGetClassObject': redefinition; different linkage [C:\Users\reale\Code\C++\MSYS2-UCRT-shell\build\msys2_ucrt64_shell.vcxproj]
          C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um\combaseapi.h(1331,9):
          see declaration of 'DllGetClassObject'

    C:\Users\reale\Code\C++\MSYS2-UCRT-shell\src\dll.cpp(25,24): error C2375: 'DllCanUnloadNow': redefinition; different linkage [C:\Users\reale\Code\C++\MSYS2-UCRT-shell\build\msys2_ucrt64_shell.vcxproj]
          C:\Program Files (x86)\Windows Kits\10\Include\10.0.26100.0\um\combaseapi.h(1334,9):
          see declaration of 'DllCanUnloadNow'
*/

//  The error occurs because the `DllGetClassObject` and `DllCanUnloadNow` functions are already declared in `combaseapi.h`. 
//  The linker will complain about the redefinition of these functions if they are defined in this file. To avoid this, we 
//  use the `/export` linker option to export our own implementations of these functions with different names (`DllGetClassObjectImpl` 
//  and `DllCanUnloadNowImpl`). This way, we can provide our own implementations without conflicting with the existing declarations.
//  We additionally use the `PRIVATE` option to suppress LNK4104 warnings about duplicate exports. 

#pragma comment(linker, "/export:DllGetClassObject=DllGetClassObjectImpl,PRIVATE")
#pragma comment(linker, "/export:DllCanUnloadNow=DllCanUnloadNowImpl,PRIVATE")
