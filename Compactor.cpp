#include <windows.h>
#include <shlobj.h>

const int ID_EMPTY_BIN = 40057;
const int ID_EXIT_PROGRAM = 40058;
const int ID_OPEN_BIN = 40059;
const int ID_BIN_PROPERTIES = 40060;
const int ID_ENABLE_HIDE = 40061;
const int ID_DISABLE_HIDE = 40062;
const wchar_t CLASS_NAME[] = L"CompactorIconWindow";
const wchar_t TOOLTIP[] = L"Recycle Bin";
const UINT WM_ICON_NOTIFY = 34592;

bool hideWhenEmpty = false;
NOTIFYICONDATA iconData;
HICON iconEmpty;
HICON iconFull;
SHQUERYRBINFO sqrbi = {sizeof(SHQUERYRBINFO)};
bool isEmpty = true;
HKEY hKey;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_APP:
            SHQueryRecycleBin(NULL, &sqrbi);
            if (isEmpty != sqrbi.i64NumItems < 1) {
                isEmpty = sqrbi.i64NumItems < 1;
                iconData.hIcon = isEmpty ? iconEmpty : iconFull;
                Shell_NotifyIcon(hideWhenEmpty ? isEmpty * 2 : NIM_MODIFY, &iconData);
            }
            break;
        case WM_DESTROY:
            Shell_NotifyIcon(NIM_DELETE, &iconData);
            PostQuitMessage(0);
            break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        case WM_COMMAND:
            switch (wparam) {
                case ID_EMPTY_BIN:
                    SHEmptyRecycleBin(NULL, NULL, NULL);
                    break;
                case ID_EXIT_PROGRAM:
                    DestroyWindow(hwnd);
                    break;
                case ID_OPEN_BIN:
                    ShellExecute(NULL, L"open", L"shell:RecycleBinFolder", NULL, NULL, SW_SHOWNORMAL);
                    break;
                case ID_BIN_PROPERTIES:
                    ShellExecute(NULL, L"properties", L"shell:RecycleBinFolder", NULL, NULL, SW_SHOWNORMAL);
                    break;
                case ID_ENABLE_HIDE: {
                    DWORD value = 1;
                    RegSetValueEx(hKey, L"HideEmpty", NULL, REG_DWORD, (PBYTE)&value, sizeof(DWORD));
                    Shell_NotifyIcon(isEmpty * 2, &iconData);
                    hideWhenEmpty = true;
                    break;
                }
                case ID_DISABLE_HIDE:
                    RegDeleteValue(hKey, L"HideEmpty");
                    Shell_NotifyIcon(NIM_ADD, &iconData);
                    hideWhenEmpty = false;
                    break;
            }
            break;
        case WM_ICON_NOTIFY:
            switch (lparam) {
                case WM_LBUTTONUP:
                    ShellExecute(NULL, L"open", L"shell:RecycleBinFolder", NULL, NULL, SW_SHOWNORMAL);
                    break;
                case WM_MBUTTONUP:
                    SHEmptyRecycleBin(NULL, NULL, NULL);
                    break;
                case WM_RBUTTONUP:
                case WM_CONTEXTMENU: {
                    SHQUERYRBINFO sqrbi = {};
                    UINT flags = MF_STRING;
                    HICON hIcon = iconFull;

                    sqrbi.cbSize = sizeof SHQUERYRBINFO;
                    SHQueryRecycleBin(NULL, &sqrbi);

                    if (sqrbi.i64NumItems < 1) {
                        flags |= MF_GRAYED;
                        hIcon = iconEmpty;
                    }

                    POINT lpClickPoint;
                    GetCursorPos(&lpClickPoint);

                    HMENU hMenu = CreatePopupMenu();
                    AppendMenu(hMenu, MF_STRING, ID_OPEN_BIN, L"Open");
                    AppendMenu(hMenu, flags, ID_EMPTY_BIN, L"Empty Recycle Bin");
                    AppendMenu(hMenu, MF_STRING, ID_BIN_PROPERTIES, L"Properties");
                    AppendMenu(hMenu, MF_SEPARATOR, NULL, NULL);
                    AppendMenu(hMenu, hideWhenEmpty * MF_CHECKED, ID_ENABLE_HIDE + hideWhenEmpty, L"Hide when empty");
                    AppendMenu(hMenu, MF_STRING, ID_EXIT_PROGRAM, L"Exit");
                    SetMenuDefaultItem(hMenu, ID_OPEN_BIN, FALSE);
                    SetForegroundWindow(hwnd);
                    TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, lpClickPoint.x, lpClickPoint.y, 0, hwnd, NULL);
                    DestroyMenu(hMenu);

                    break;
                }
            }
            break;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
    CreateMutex(NULL, TRUE, L"b3649a7a-111b-11eb-adc1-0242ac120002");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return 0;
    }

    RegCreateKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Compactor", NULL, NULL, NULL, KEY_READ | KEY_WRITE, NULL, &hKey, NULL);
    hideWhenEmpty = RegQueryValueEx(hKey, L"HideEmpty", NULL, NULL, NULL, NULL) == ERROR_SUCCESS;

    HMODULE shell32 = LoadLibrary(L"SHELL32.dll");
    iconEmpty = (HICON)LoadImage(shell32, MAKEINTRESOURCE(32), IMAGE_ICON, 16, 16, NULL);
    iconFull = (HICON)LoadImage(shell32, MAKEINTRESOURCE(33), IMAGE_ICON, 16, 16, NULL);
    FreeLibrary(shell32);

    WNDCLASS winClass = {};
    winClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    winClass.hInstance = hInstance;
    winClass.lpszClassName = CLASS_NAME;
    winClass.style = CS_HREDRAW | CS_VREDRAW;
    winClass.lpfnWndProc = WindowProc;
    winClass.hIcon = iconEmpty;
    RegisterClass(&winClass);

    HWND hwnd = CreateWindowEx(NULL, CLASS_NAME, NULL, CW_USEDEFAULT, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);

    memset(&iconData, 0, sizeof(iconData));
    iconData.cbSize = sizeof(iconData);
    iconData.hIcon = iconEmpty;
    iconData.hWnd = hwnd;
    iconData.uID = WM_ICON_NOTIFY;
    iconData.uCallbackMessage = WM_ICON_NOTIFY;
    iconData.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    wcscpy_s(iconData.szTip, sizeof(iconData.szTip) / sizeof(WCHAR), TOOLTIP);

    if (!hideWhenEmpty || !isEmpty) {
        Shell_NotifyIcon(NIM_ADD, &iconData);
    }

    LPITEMIDLIST szPath;
    SHGetFolderLocation(NULL, CSIDL_BITBUCKET, NULL, NULL, &szPath);
    SHChangeNotifyEntry changeInfo;
    changeInfo.pidl = szPath;
    changeInfo.fRecursive = FALSE;

    WindowProc(hwnd, WM_APP, NULL, NULL);

    ULONG handle = SHChangeNotifyRegister(
        hwnd,
        SHCNRF_ShellLevel,
        SHCNE_CREATE | SHCNE_DELETE | SHCNE_MKDIR | SHCNE_RMDIR | SHCNE_UPDATEDIR | SHCNE_UPDATEIMAGE,
        WM_APP,
        1,
        &changeInfo
    );
    MSG msg;

    while (GetMessage(&msg, NULL, NULL, NULL)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    SHChangeNotifyDeregister(handle);

    return 0;
}