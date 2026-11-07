#include "qhb.h"

#include <objbase.h>
#include <mmsystem.h>

using namespace qhb;

static App* g_app = nullptr;

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE:
            g_app = new App(hwnd, (HINSTANCE)GetWindowLongPtrW(hwnd, GWLP_HINSTANCE));
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            if (g_app) g_app->paint(hdc, &ps.rcPaint);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_SIZE:
            if (g_app) g_app->onResize();
            return 0;
        case WM_DPICHANGED:
            if (g_app) g_app->onDpiChanged(HIWORD(wp), (const RECT*)lp);
            return 0;
        case WM_MOUSEMOVE:
            if (g_app) g_app->onMouseMove(GET_X_LPARAM(lp), GET_Y_LPARAM(lp), wp);
            return 0;
        case WM_MOUSELEAVE:
            if (g_app) g_app->onMouseLeave();
            return 0;
        case WM_LBUTTONDOWN:
            if (g_app) g_app->onLButtonDown(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
            return 0;
        case WM_LBUTTONUP:
            if (g_app) g_app->onLButtonUp(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
            return 0;
        case WM_LBUTTONDBLCLK:
            if (g_app) g_app->onLButtonDblClk(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
            return 0;
        case WM_POINTERDOWN:
            if (g_app) g_app->onPointerDown((UINT)wp, GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
            return 0;
        case WM_POINTERUPDATE:
            if (g_app) g_app->onPointerUpdate((UINT)wp, GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
            return 0;
        case WM_POINTERUP:
            if (g_app) g_app->onPointerUp((UINT)wp, GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
            return 0;
        case WM_POINTERCAPTURECHANGED:
            if (g_app) g_app->onPointerCaptureLost((UINT)wp);
            return 0;
        case WM_RBUTTONUP:
            if (g_app) g_app->onRButtonUp(GET_X_LPARAM(lp), GET_Y_LPARAM(lp));
            return 0;
        case WM_GESTURE:
            if (g_app && g_app->onGesture(lp)) return 0;
            break;
        case WM_MOUSEWHEEL: {
            POINT pt{GET_X_LPARAM(lp), GET_Y_LPARAM(lp)};
            ScreenToClient(hwnd, &pt);
            bool ctrl = (GET_KEYSTATE_WPARAM(wp) & MK_CONTROL) != 0;
            bool shift = (GET_KEYSTATE_WPARAM(wp) & MK_SHIFT) != 0;
            if (g_app) g_app->onMouseWheel(pt.x, pt.y, GET_WHEEL_DELTA_WPARAM(wp), ctrl, shift);
            return 0;
        }
        case WM_KEYDOWN:
            if (g_app) {
                bool ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
                bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
                bool alt = (GetKeyState(VK_MENU) & 0x8000) != 0;
                if (g_app->onKeyDown(wp, ctrl, shift, alt)) return 0;
            }
            break;
        case WM_KEYUP:
            if (g_app) g_app->onKeyUp(wp);
            return 0;
        case WM_CHAR:
            if (g_app && g_app->onChar((wchar_t)wp)) return 0;
            break;
        case App::WM_APP_COMMIT_TEXT:
            if (g_app) g_app->commitText();
            return 0;
        case App::WM_APP_COMMIT_NAME:
            if (g_app) g_app->finishAppNameEdit();
            return 0;
        case WM_TIMER:
            if (g_app) g_app->onTimer(wp);
            return 0;
        case WM_SETCURSOR:
            if (LOWORD(lp) == HTCLIENT && g_app) {
                g_app->refreshCursor();
                return TRUE;
            }
            break;
        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wp;
            SetTextColor(hdc, RGB(32, 38, 51));
            SetBkColor(hdc, RGB(255, 255, 255));
            return (LRESULT)GetStockObject(WHITE_BRUSH);
        }
        case WM_DROPFILES:
            if (g_app) g_app->onDrop((HDROP)wp);
            return 0;
        case WM_CLOSE:
            if (g_app) g_app->onCloseRequest();
            else DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            delete g_app;
            g_app = nullptr;
            PostQuitMessage(0);
            return 0;
        default:
            break;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE, PWSTR, int) {
    timeBeginPeriod(1);
    SetCurrentProcessExplicitAppUserModelID(L"HAVEGET.Whiteboard.1");
    // Per-monitor DPI awareness (V2 when available).
    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (user32) {
        typedef BOOL(WINAPI* SetCtx)(HANDLE);
        auto fn = (SetCtx)GetProcAddress(user32, "SetProcessDpiAwarenessContext");
        if (fn) fn((HANDLE)-4);
        else SetProcessDPIAware();
    }
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    GdiplusStartupInput gsi;
    ULONG_PTR token = 0;
    if (GdiplusStartup(&token, &gsi, nullptr) != Ok) return 1;

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = inst;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hIcon = (HICON)LoadImageW(inst, MAKEINTRESOURCEW(IDI_APP), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE);
    if (!wc.hIcon) wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    wc.hIconSm = (HICON)LoadImageW(inst, MAKEINTRESOURCEW(IDI_APP), IMAGE_ICON,
                                    GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
                                    LR_DEFAULTCOLOR);
    if (!wc.hIconSm) wc.hIconSm = wc.hIcon;
    wc.hbrBackground = nullptr;
    wc.lpszClassName = L"HAVEGETWhiteboardWindow";
    if (!RegisterClassExW(&wc)) {
        GdiplusShutdown(token);
        CoUninitialize();
        return 1;
    }

    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"HAVEGET白板", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                                CW_USEDEFAULT, CW_USEDEFAULT, 1500, 940, nullptr, nullptr, inst, nullptr);
    if (!hwnd) {
        GdiplusShutdown(token);
        CoUninitialize();
        return 1;
    }
    DragAcceptFiles(hwnd, TRUE);
    ShowWindow(hwnd, SW_MAXIMIZE);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    GdiplusShutdown(token);
    CoUninitialize();
    timeEndPeriod(1);
    return 0;
}
