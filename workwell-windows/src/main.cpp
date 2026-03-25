#define _WIN32_WINNT 0x0601
#define _WIN32_IE 0x0601

#include <windows.h>
#include <string>
#include <memory>

#include "TrayIcon.h"
#include "TimerManager.h"
#include "BreakWindow.h"
#include "MainDialog.h"
#include "ConfigManager.h"

HINSTANCE g_hInstance = NULL;
std::unique_ptr<TimerManager> g_timerManager;
std::unique_ptr<BreakWindow> g_breakWindow;
std::unique_ptr<MainDialog> g_mainDialog;
std::unique_ptr<TrayIcon> g_trayIcon;

const wchar_t CLASS_NAME[] = L"WorkWellClass";

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_TIMER:
        if (g_timerManager) {
            g_timerManager->OnTick();
        }
        return 0;

    default:
        if (uMsg == WM_USER + 1 && g_trayIcon)
        {
            return g_trayIcon->HandleTrayMessage(wParam, lParam);
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

bool InitApplication()
{
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = g_hInstance;
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClassExW(&wc))
    {
        return false;
    }

    HWND hwnd = CreateWindowExW(
        0,
        CLASS_NAME,
        L"WorkWell",
        0,
        0, 0, 0, 0,
        HWND_MESSAGE,
        NULL,
        g_hInstance,
        NULL
    );

    if (!hwnd)
    {
        return false;
    }

    g_timerManager = std::make_unique<TimerManager>();

    // 加载保存的配置
    TimerManager::Settings settings;
    if (ConfigManager::Load(settings))
    {
        g_timerManager->SetSettings(settings);
    }

    g_breakWindow = std::make_unique<BreakWindow>(g_hInstance);
    g_breakWindow->SetTimerManager(g_timerManager.get());
    g_mainDialog = std::make_unique<MainDialog>(g_hInstance, g_timerManager.get());
    g_trayIcon = std::make_unique<TrayIcon>(g_hInstance, hwnd, g_timerManager.get());

    g_trayIcon->SetMainDialog(g_mainDialog.get());

    g_timerManager->SetOnTick([trayIcon = g_trayIcon.get(), dialog = g_mainDialog.get()](int seconds, bool isActive) {
        trayIcon->UpdateTimer(seconds, isActive);
        dialog->UpdateCountdown(seconds);
    });

    g_timerManager->SetOnBreakStart([breakWindow = g_breakWindow.get(), trayIcon = g_trayIcon.get(), timerMgr = g_timerManager.get()]() {
        trayIcon->SetBreakState(true);
        // 根据番茄钟数决定休息时长
        int completedPomodoros = timerMgr->GetCompletedPomodoros();
        const auto& settings = timerMgr->GetSettings();
        bool shouldUseLongBreak = (completedPomodoros > 0 && completedPomodoros % settings.pomodorosUntilLongBreak == 0);
        int breakDuration = shouldUseLongBreak ? (settings.longBreakDuration / 60) : (settings.shortBreakDuration / 60);
        breakWindow->Show(breakDuration);
    });

    g_timerManager->SetOnBreakEnd([breakWindow = g_breakWindow.get(), trayIcon = g_trayIcon.get()]() {
        trayIcon->SetBreakState(false);
        breakWindow->Hide();
    });

    g_timerManager->SetOnWarning([trayIcon = g_trayIcon.get()]() {
        trayIcon->ShowWarning();
    });

    // 不自动启动，等待用户点击开始按钮
    // g_timerManager->StartWork(25 * 60);

    SetTimer(hwnd, 1, 1000, NULL);

    return true;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    g_hInstance = hInstance;

    if (!InitApplication())
    {
        return 1;
    }

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    g_trayIcon.reset();
    g_mainDialog.reset();
    g_breakWindow.reset();
    g_timerManager.reset();

    return 0;
}
