#include <windows.h>

#include "ClassFactory.h"
#include "Guids.h"

extern "C" __declspec(dllexport) 
HRESULT STDAPICALLTYPE DllGetClassObject(REFCLSID rclsid, REFIID riid, void** object) {
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

extern "C" __declspec(dllexport) 
HRESULT STDAPICALLTYPE DllCanUnloadNow() {
    // ? Returning `S_FALSE` is fine temporarily. Later, it will be replaced with a real global object/server-lock count.
    return S_FALSE;
}
