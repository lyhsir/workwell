#pragma once

#include <windows.h>

class TimerManager;

class BreakWindow
{
public:
    BreakWindow(HINSTANCE hInstance);
    ~BreakWindow();

    void Show(int durationSeconds);
    void Hide();
    void SetTimerManager(TimerManager* timerManager);

private:
    HINSTANCE m_hInstance;
    HWND m_hwnd;
    HWND m_countdownLabel;
    int m_durationSeconds;
    bool m_isVisible;

    bool RegisterWindowClass();
    bool CreateBreakWindow();
    void UpdateCountdown();

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
