#pragma once

#include <windows.h>
#include <string>
#include <functional>
#include "TimerManager.h"

class MainDialog;

class TrayIcon
{
public:
    TrayIcon(HINSTANCE hInstance, HWND hwnd, TimerManager* timerManager);
    ~TrayIcon();

    void SetMainDialog(MainDialog* dialog);
    void UpdateTimer(int seconds, bool isActive);
    void SetBreakState(bool inBreak);
    void ShowWarning();
    LRESULT HandleTrayMessage(WPARAM wParam, LPARAM lParam);

private:
    HINSTANCE m_hInstance;
    HWND m_hwnd;
    TimerManager* m_timerManager;
    NOTIFYICONDATAW m_nid;
    bool m_inBreak;

    bool AddIcon();
    bool RemoveIcon();
    void UpdateTooltip(const wchar_t* text);
    void ShowPopupMenu();
};
