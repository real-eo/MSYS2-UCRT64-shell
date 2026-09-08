#include "ExplorerCommand.h"

#include <shlwapi.h>


ExplorerCommand::ExplorerCommand() {}
ExplorerCommand::~ExplorerCommand() {}

STDMETHODIMP ExplorerCommand::QueryInterface(REFIID riid, void** object) {
    if (object == nullptr)
        return E_POINTER;

    *object = nullptr;

    if (riid == IID_IUnknown || riid == IID_IExplorerCommand) {
        *object = static_cast<IExplorerCommand*>(this);
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) ExplorerCommand::AddRef() {
    return static_cast<ULONG>(
        InterlockedIncrement(&referenceCount_)
    );
}

STDMETHODIMP_(ULONG) ExplorerCommand::Release() {
    ULONG count = static_cast<ULONG>(
        InterlockedDecrement(&referenceCount_)
    );

    if (count == 0)
        delete this;

    return count;
}

// * IExplorerCommand methods
STDMETHODIMP ExplorerCommand::GetTitle(IShellItemArray*, LPWSTR* title) {
    if (title == nullptr)
        return E_POINTER;

    *title = nullptr;

    return SHStrDupW(
        L"Open in MSYS2 UCRT64",
        title
    );
}

STDMETHODIMP ExplorerCommand::GetIcon(IShellItemArray*, LPWSTR* icon) {
    if (icon == nullptr)
        return E_POINTER;

    *icon = nullptr;

    // TODO: No icon yet.
    return E_NOTIMPL;
}

STDMETHODIMP ExplorerCommand::GetToolTip(IShellItemArray*, LPWSTR* tooltip) {
    if (tooltip == nullptr)
        return E_POINTER;

    *tooltip = nullptr;

    return SHStrDupW(
        L"Open this folder in MSYS2 UCRT64",
        tooltip
    );
}

STDMETHODIMP ExplorerCommand::GetCanonicalName(GUID* commandName) {
    if (commandName == nullptr)
        return E_POINTER;

    *commandName = GUID_NULL;
    return S_OK;
}

STDMETHODIMP ExplorerCommand::GetState(IShellItemArray*, BOOL, EXPCMDSTATE* state) {
    if (state == nullptr)
        return E_POINTER;

    *state = ECS_ENABLED;
    return S_OK;
}

STDMETHODIMP ExplorerCommand::Invoke(IShellItemArray*, IBindCtx*) {
    // Implement folder-path extraction and MSYS2 launching next.
    return E_NOTIMPL;
}

STDMETHODIMP ExplorerCommand::GetFlags(EXPCMDFLAGS* flags) {
    if (flags == nullptr)
        return E_POINTER;

    *flags = ECF_DEFAULT;
    return S_OK;
}

STDMETHODIMP ExplorerCommand::EnumSubCommands(IEnumExplorerCommand** enumerator) {
    if (enumerator == nullptr)
        return E_POINTER;

    *enumerator = nullptr;

    return E_NOTIMPL;                                                                   // ? This command has no submenu
}
