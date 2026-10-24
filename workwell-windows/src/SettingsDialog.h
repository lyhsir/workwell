#pragma once
#include <windows.h>
#include <functional>
class SettingsDialog
{
  public:
    struct Settings
    {
        int workDuration = 25, shortBreakDuration = 5, longBreakDuration = 15, pomodorosUntilLongBreak = 4;
        bool canSkipBreak = true;
    };
    using OnSettingsChangedCallback = std::function<bool(const Settings &)>;
    SettingsDialog(HINSTANCE, HWND, const Settings &);
    ~SettingsDialog();
    void Show();
    void Hide();
    bool IsVisible() const { return IsWindowVisible(m_hwnd) != FALSE; }
    HWND Handle() const { return m_hwnd; }
    void SetOnSettingsChanged(OnSettingsChangedCallback cb) { m_changed = cb; }

  private:
    HINSTANCE m_instance;
    HWND m_parent, m_hwnd{}, m_edits[4]{}, m_spins[4]{}, m_modes[2]{};
    HFONT m_font{};
    Settings m_saved;
    bool m_canSkip;
    int m_error = -1;
    OnSettingsChangedCallback m_changed;
    void Layout();
    void Paint();
    bool Apply();
    void Restore();
    static LRESULT CALLBACK Proc(HWND, UINT, WPARAM, LPARAM);
};
