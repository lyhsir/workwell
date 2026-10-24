#pragma once
#include <windows.h>
#include <vector>
class TimerManager;
class BreakWindow
{
  public:
    explicit BreakWindow(HINSTANCE);
    ~BreakWindow();
    void Show(int);
    void Hide();
    void Refresh();
    void SetTimerManager(TimerManager *t) { m_timer = t; }
    bool Translate(MSG *);

  private:
    HINSTANCE m_instance;
    TimerManager *m_timer{};
    std::vector<HWND> m_windows;
    bool m_visible = false, m_windowed = false;
    ULONGLONG m_escapeStart = 0;
    void Paint(HWND);
    void Layout(HWND);
    void ToggleFullscreen(HWND);
    static BOOL CALLBACK MonitorProc(HMONITOR, HDC, LPRECT, LPARAM);
    static LRESULT CALLBACK Proc(HWND, UINT, WPARAM, LPARAM);
};
