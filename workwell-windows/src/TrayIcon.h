#pragma once
#include <windows.h>
#include <shellapi.h>
#include "TimerManager.h"
class MainDialog;
class TrayIcon
{
  public:
    TrayIcon(HINSTANCE, HWND, TimerManager *);
    ~TrayIcon();
    void SetMainDialog(MainDialog *dialog) { m_dialog = dialog; }
    void UpdateTimer(int, bool);
    void SetBreakState(bool);
    void ShowWarning();
    LRESULT HandleTrayMessage(WPARAM, LPARAM);
    void Restore();

  private:
    HWND m_hwnd;
    TimerManager *m_timer;
    MainDialog *m_dialog{};
    NOTIFYICONDATAW m_nid{};
    HICON m_countdownIcon{};
    int m_iconKey = -2;
    void ShowPopupMenu();
};
