#pragma once

#include <windows.h>
#include <unknwn.h>

class ClassFactory final : public IClassFactory {
private:
    ~ClassFactory();

    LONG referenceCount_ = 1;

public:
    ClassFactory();

    STDMETHODIMP QueryInterface(REFIID riid, void** object) override;
    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;

    STDMETHODIMP CreateInstance(IUnknown* outer, REFIID riid, void** object) override;

    STDMETHODIMP LockServer(BOOL lock) override;

};