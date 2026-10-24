#pragma once
#include <windows.h>
#include <memory>
class TimerManager;
class SettingsDialog;
class MainDialog
{
  public:
    MainDialog(HINSTANCE, TimerManager *);
    ~MainDialog();
    void Show();
    void Hide();
    bool IsVisible() const { return IsWindowVisible(m_hwnd) != FALSE; }
    void UpdateCountdown(int);
    void UpdateButtonState(bool, bool) { UpdateCountdown(0); }
    void ShowCompletion();
    void HideForBreak();
    bool Translate(MSG *);

  private:
    HINSTANCE m_instance;
    TimerManager *m_timer;
    HWND m_hwnd{}, m_complete{}, m_primary{}, m_stop{};
    std::unique_ptr<SettingsDialog> m_settings;
    ULONGLONG m_toastUntil = 0;
    void Layout();
    void Paint(HWND);
    void Toggle();
    void Sync();
    static LRESULT CALLBACK Proc(HWND, UINT, WPARAM, LPARAM);
};
