#pragma once

#include <windows.h>
#include <functional>

class SettingsDialog
{
public:
    struct Settings
    {
        int workDuration;        // 工作时长（分钟）
        int shortBreakDuration;  // 短休息时长（分钟）
        int longBreakDuration;   // 长休息时长（分钟）
        int pomodorosUntilLongBreak;  // 多少个番茄钟后长休息
        bool canSkipBreak;       // 是否可以跳过休息

        Settings()
            : workDuration(25)
            , shortBreakDuration(5)
            , longBreakDuration(15)
            , pomodorosUntilLongBreak(4)
            , canSkipBreak(false)
        {}
    };

    using OnSettingsChangedCallback = std::function<void(const Settings& settings)>;

    SettingsDialog(HINSTANCE hInstance, HWND parentHWND, const Settings& initialSettings);
    ~SettingsDialog();

    void Show();
    void Hide();
    void SetOnSettingsChanged(OnSettingsChangedCallback callback) { m_onSettingsChanged = callback; }

private:
    HINSTANCE m_hInstance;
    HWND m_parentHWND;
    HWND m_hwnd;
    Settings m_settings;
    OnSettingsChangedCallback m_onSettingsChanged;

    HWND m_workDurationEdit;
    HWND m_shortBreakEdit;
    HWND m_longBreakEdit;
    HWND m_pomodorosEdit;
    HWND m_canSkipBreakCheck;

    bool RegisterWindowClass();
    bool CreateDialogWindow();
    void InitControls();
    void ApplySettings();

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
