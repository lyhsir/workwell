#pragma once

#include <windows.h>
#include <memory>

class TimerManager;
class SettingsDialog;

class MainDialog
{
public:
    MainDialog(HINSTANCE hInstance, TimerManager* timerManager);
    ~MainDialog();

    void Show();
    void Hide();
    bool IsVisible() const { return m_isVisible; }
    void UpdateCountdown(int seconds);
    void UpdateButtonState(bool isRunning, bool isPaused);
    void OnSettingsChanged(int workDuration, int shortBreakDuration, int longBreakDuration, int pomodorosUntilLongBreak, bool canSkipBreak);

private:
    HINSTANCE m_hInstance;
    TimerManager* m_timerManager;
    std::unique_ptr<SettingsDialog> m_settingsDialog;
    HWND m_hwnd;
    HWND m_stateLabel;
    HWND m_countdownLabel;
    HWND m_pauseButton;
    HWND m_stopButton;
    HWND m_settingsButton;
    HWND m_statsLabel;
    bool m_isVisible;

    bool RegisterWindowClass();
    bool CreateMainWindow();
    void DrawCountdown(HDC hdc, int seconds);
    void UpdateStats();
    void UpdateButtons();
    void UpdateState();

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
