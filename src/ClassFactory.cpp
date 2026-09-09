#include "ClassFactory.h"

#include "ExplorerCommand.h"


ClassFactory::ClassFactory() {}
ClassFactory::~ClassFactory() {}

STDMETHODIMP ClassFactory::QueryInterface(REFIID riid, void** object) {
    // OutputDebugStringW(L"MSYS2: ClassFactory::QueryInterface\n");                       // | DEBUG

    if (object == nullptr)
        return E_POINTER;

    *object = nullptr;

    if (riid == IID_IUnknown || riid == IID_IClassFactory) {
        *object = static_cast<IClassFactory*>(this);
        AddRef();
        return S_OK;
    }

    // OutputDebugStringW(L"MSYS2: ClassFactory::QueryInterface -> E_NOINTERFACE\n");      // | DEBUG
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) ClassFactory::AddRef() {
    return static_cast<ULONG>(
        InterlockedIncrement(&referenceCount_)
    );
}

STDMETHODIMP_(ULONG) ClassFactory::Release() {
    ULONG count = static_cast<ULONG>(
        InterlockedDecrement(&referenceCount_)
    );

    if (count == 0)
        delete this;

    return count;
}

STDMETHODIMP ClassFactory::CreateInstance(IUnknown* outer, REFIID riid, void** object) {
    // OutputDebugStringW(L"MSYS2: ClassFactory::CreateInstance\n");                       // | DEBUG
    if (object == nullptr)
        return E_POINTER;

    *object = nullptr;

    if (outer != nullptr)
        return CLASS_E_NOAGGREGATION;

    auto* command = new ExplorerCommand();

    HRESULT result = command->QueryInterface(riid, object);

    // if (SUCCEEDED(result))                                                              // | DEBUG
        // OutputDebugStringW(L"MSYS2: CreateInstance -> success\n");                      // | DEBUG
    // else                                                                                // | DEBUG
        // OutputDebugStringW(L"MSYS2: CreateInstance -> failure\n");                      // | DEBUG


    command->Release();

    return result;
}

STDMETHODIMP ClassFactory::LockServer(BOOL) {
    return S_OK;
}
