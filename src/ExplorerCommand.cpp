#include "ExplorerCommand.h"

#include <shellapi.h>
#include <shobjidl.h>
#include <shlwapi.h>
#include <shlobj.h>

#include <string>


// * Ctor & dtor
ExplorerCommand::ExplorerCommand() {}
ExplorerCommand::~ExplorerCommand() {
    if (site_ != nullptr)
        site_->Release();                                                               // ? Release the site in the destructor to avoid memory leaks
}

// * Logging functions
namespace Log {
    inline void BackgroundItemsFailure(const HRESULT& result) {
        wchar_t message[128];
        swprintf_s(message, L"MSYS2: GetBackgroundItems failed: 0x%08lX\n", static_cast<unsigned long>(result));
        OutputDebugStringW(message);
    }
}

// * Private methods
HRESULT ExplorerCommand::GetBackgroundItems(IShellItemArray** items) {
    // Sanity check
    if (items == nullptr) 
        return E_POINTER;
    
    *items = nullptr;

    if (site_ == nullptr) 
        return E_UNEXPECTED;

    // Get the service provider from the site
    IServiceProvider* serviceProvider = nullptr;
    HRESULT result = site_->QueryInterface(
        IID_PPV_ARGS(&serviceProvider)
    );

    if (FAILED(result)) {
        Log::BackgroundItemsFailure(result);
        return result;
    }

    // Get the top-level browser from the service provider
    IShellBrowser* browser = nullptr;
    result = serviceProvider->QueryService(
        SID_STopLevelBrowser,
        IID_PPV_ARGS(&browser)
    );

    serviceProvider->Release();
    
    if (FAILED(result)) {
        Log::BackgroundItemsFailure(result);
        return result;
    }

    // Get the active shell view from the browser
    IShellView* shellView = nullptr;
    result = browser->QueryActiveShellView(&shellView);
    browser->Release();
    
    if (FAILED(result)) {
        Log::BackgroundItemsFailure(result);
        return result;
    }

    // Get the current folder from the shell view
    IFolderView* folderView = nullptr;
    result = shellView->QueryInterface(
        IID_PPV_ARGS(&folderView)
    );

    if (FAILED(result)) {
        shellView->Release();
        Log::BackgroundItemsFailure(result);
        return result;
    }

    IShellFolder* shellFolder = nullptr;
    result = folderView->GetFolder(
        IID_PPV_ARGS(&shellFolder)
    );

    folderView->Release();
    shellView->Release();

    if (FAILED(result)) {
        Log::BackgroundItemsFailure(result);
        return result;
    }

    // Get the current folder PIDL
    IPersistFolder2* persistFolder = nullptr;
    result = shellFolder->QueryInterface(
        IID_PPV_ARGS(&persistFolder)
    );

    shellFolder->Release();
    
    if (FAILED(result)) {
        Log::BackgroundItemsFailure(result);
        return result;
    }

    PIDLIST_ABSOLUTE folderPidl = nullptr;
    result = persistFolder->GetCurFolder(&folderPidl);

    persistFolder->Release();
    
    if (FAILED(result)) {
        Log::BackgroundItemsFailure(result);
        return result;
    }

    // Create a shell item array from the folder PIDL
    IShellItem* shellItem = nullptr;
    result = SHCreateItemFromIDList(
        folderPidl,
        IID_PPV_ARGS(&shellItem)
    );

    CoTaskMemFree(folderPidl);
    
    if (FAILED(result)) {
        Log::BackgroundItemsFailure(result);
        return result;
    }

    // Create a shell item array from the shell item
    result = SHCreateShellItemArrayFromShellItem(
        shellItem,
        IID_PPV_ARGS(items)
    );

    shellItem->Release();

    if (FAILED(result)) {
        Log::BackgroundItemsFailure(result);
        return result;
    }

    return result;
}

// * Public methods
STDMETHODIMP ExplorerCommand::QueryInterface(REFIID riid, void** object) {
    if (object == nullptr)
        return E_POINTER;

    *object = nullptr;


    if (riid == IID_IUnknown || riid == IID_IExplorerCommand) {
        *object = static_cast<IExplorerCommand*>(this);
    }

    else if (riid == IID_IObjectWithSite) {
        *object = static_cast<IObjectWithSite*>(this);
    }

    else {
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
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

    // TODO: Make the icon path dynamic based on the MSYS2 installation directory. For now, it is hardcoded.
    return SHStrDupW(
        L"C:\\msys64\\ucrt64.ico",
        icon
    );
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

STDMETHODIMP ExplorerCommand::Invoke(IShellItemArray* items, IBindCtx*) {
    IShellItemArray* targetItems = items;

    if (targetItems == nullptr) {
        HRESULT result = GetBackgroundItems(&targetItems);
        if (FAILED(result)) return result;
    } 
    
    else {
        targetItems->AddRef();
    }

    IShellItem* item = nullptr;
    HRESULT result = targetItems->GetItemAt(0, &item);
    targetItems->Release();                                                             // ? Release the reference to the shell item array to avoid memory leaks

    if (FAILED(result)) return result;

    PWSTR folderPath = nullptr;
    result = item->GetDisplayName(SIGDN_FILESYSPATH, &folderPath);
    item->Release();

    if (FAILED(result)) return result;

    SHELLEXECUTEINFOW executeInfo{};
    executeInfo.cbSize = sizeof(executeInfo);
    executeInfo.lpFile = L"C:\\msys64\\ucrt64.exe";
    executeInfo.lpDirectory = folderPath;
    executeInfo.nShow = SW_SHOWNORMAL;

    BOOL launched = ShellExecuteExW(&executeInfo);
    DWORD error = launched ? ERROR_SUCCESS : GetLastError();

    CoTaskMemFree(folderPath);

    return launched ? S_OK : HRESULT_FROM_WIN32(error);
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

STDMETHODIMP ExplorerCommand::SetSite(IUnknown* site) {
    if (site_ != nullptr) {
        site_->Release();
        site_ = nullptr;
    }

    if (site != nullptr) {
        site_ = site;
        site_->AddRef();
    }

    return S_OK;
}

STDMETHODIMP ExplorerCommand::GetSite(REFIID riid, void** object) {
    if (object == nullptr)
        return E_POINTER;

    *object = nullptr;

    if (site_ == nullptr)
        return E_FAIL;

    return site_->QueryInterface(riid, object);
}
