#include <windows.h>
#include <memory>
#ifdef _MSC_VER
#pragma comment(                                                                                             \
    linker,                                                                                                  \
    "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#include "Ui.h"
#include "TrayIcon.h"
#include "TimerManager.h"
#include "BreakWindow.h"
#include "MainDialog.h"
#include "ConfigManager.h"
std::unique_ptr<TimerManager> g_timer;
std::unique_ptr<BreakWindow> g_break;
std::unique_ptr<MainDialog> g_main;
std::unique_ptr<TrayIcon> g_tray;
UINT g_taskbarCreated = 0;
LRESULT CALLBACK HostProc(HWND w, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == g_taskbarCreated && g_tray)
    {
        g_tray->Restore();
        return 0;
    }
    switch (msg)
    {
    case WM_TIMER:
        if (g_timer)
            g_timer->OnTick();
        return 0;
    case WM_USER + 1:
        if (g_tray)
            return g_tray->HandleTrayMessage(wp, lp);
        return 0;
    case WM_CLOSE:
        DestroyWindow(w);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(w, msg, wp, lp);
}
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int)
{
    using DpiFn = BOOL(WINAPI *)(HANDLE);
    auto dpiFn =
        reinterpret_cast<DpiFn>(GetProcAddress(GetModuleHandleW(L"user32"), "SetProcessDpiAwarenessContext"));
    if (dpiFn)
        dpiFn(reinterpret_cast<HANDLE>(-4));
    else
        SetProcessDPIAware();
    HANDLE mutex = CreateMutexW(nullptr, FALSE, L"Local\\WorkWellDesktopV2");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        HWND other = FindWindowW(L"WorkWellMainV2", L"WorkWell");
        if (other)
        {
            ShowWindow(other, SW_RESTORE);
            SetForegroundWindow(other);
        }
        if (mutex)
            CloseHandle(mutex);
        return 0;
    }
    Gdiplus::GdiplusStartupInput input;
    ULONG_PTR token;
    if (Gdiplus::GdiplusStartup(&token, &input, nullptr) != Gdiplus::Ok)
    {
        CloseHandle(mutex);
        return 1;
    }
    INITCOMMONCONTROLSEX controls{sizeof(controls), ICC_STANDARD_CLASSES | ICC_UPDOWN_CLASS};
    InitCommonControlsEx(&controls);
    WNDCLASSW wc{};
    wc.hInstance = instance;
    wc.lpfnWndProc = HostProc;
    wc.lpszClassName = L"WorkWellHostV2";
    RegisterClassW(&wc);
    HWND host = CreateWindowExW(WS_EX_TOOLWINDOW, wc.lpszClassName, L"WorkWellHost", WS_POPUP, 0, 0, 0, 0,
                                nullptr, nullptr, instance, nullptr);
    g_taskbarCreated = RegisterWindowMessageW(L"TaskbarCreated");
    g_timer = std::make_unique<TimerManager>();
    TimerManager::Settings settings;
    ConfigManager::Load(settings);
    g_timer->SetSettings(settings);
    g_break = std::make_unique<BreakWindow>(instance);
    g_break->SetTimerManager(g_timer.get());
    g_main = std::make_unique<MainDialog>(instance, g_timer.get());
    g_tray = std::make_unique<TrayIcon>(instance, host, g_timer.get());
    g_tray->SetMainDialog(g_main.get());
    g_timer->SetOnTick(
        [](int seconds, bool active)
        {
            g_tray->UpdateTimer(seconds, active);
            g_main->UpdateCountdown(seconds);
            g_break->Refresh();
        });
    g_timer->SetOnBreakStart(
        []()
        {
            g_main->HideForBreak();
            g_tray->SetBreakState(true);
            g_break->Show(g_timer->Remaining());
        });
    g_timer->SetOnBreakEnd(
        []()
        {
            g_break->Hide();
            g_tray->SetBreakState(false);
            g_main->ShowCompletion();
        });
    g_timer->SetOnWarning([]() { g_tray->ShowWarning(); });
    g_timer->SetOnBreakDeferred(
        []()
        {
            g_break->Hide();
            g_tray->SetBreakState(false);
            g_main->Show();
        });
    SetTimer(host, 1, 250, nullptr);
    g_main->Show();
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        if (g_break->Translate(&msg) || g_main->Translate(&msg))
            continue;
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    g_tray.reset();
    g_main.reset();
    g_break.reset();
    g_timer.reset();
    Gdiplus::GdiplusShutdown(token);
    if (mutex)
        CloseHandle(mutex);
    return 0;
}
