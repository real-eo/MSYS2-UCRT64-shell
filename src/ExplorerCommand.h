#pragma once

#include <windows.h>
#include <shobjidl.h>

class ExplorerCommand final : public IExplorerCommand, public IObjectWithSite {
private:
    ~ExplorerCommand();
    
    HRESULT GetBackgroundItems(IShellItemArray** items);
    
    LONG referenceCount_ = 1;
    IUnknown* site_ = nullptr;

public:
    ExplorerCommand();

    STDMETHODIMP QueryInterface(REFIID riid, void** object) override;

    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;

    STDMETHODIMP GetTitle(IShellItemArray* items, LPWSTR* title) override;
    STDMETHODIMP GetIcon(IShellItemArray* items, LPWSTR* icon) override;
    STDMETHODIMP GetToolTip(IShellItemArray* items, LPWSTR* tooltip) override;
    STDMETHODIMP GetCanonicalName(GUID* commandName) override;
    STDMETHODIMP GetState(IShellItemArray* items, BOOL okToBeSlow, EXPCMDSTATE* state) override;
    STDMETHODIMP Invoke(IShellItemArray* items, IBindCtx* bindContext) override;
    STDMETHODIMP GetFlags(EXPCMDFLAGS* flags) override;
    STDMETHODIMP EnumSubCommands(IEnumExplorerCommand** enumerator) override;
    STDMETHODIMP SetSite(IUnknown* site) override;
    STDMETHODIMP GetSite(REFIID riid, void** object) override;
};