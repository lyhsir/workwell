#include "TrayIcon.h"
#include "MainDialog.h"
#include "Ui.h"
#include <strsafe.h>

namespace
{
HICON CreateCountdownIcon(int value, bool seconds, TimerState state)
{
    int size = std::max(32, GetSystemMetrics(SM_CXSMICON) * 2);
    Gdiplus::Bitmap bitmap(size, size, PixelFormat32bppARGB);
    Gdiplus::Graphics g(&bitmap);
    Ui::Quality(g);
    g.Clear(Gdiplus::Color(0, 0, 0, 0));
    g.ScaleTransform(size / 32.f, size / 32.f);
    DWORD background = state == TimerState::Paused  ? 0xff98620d
                       : state == TimerState::Break ? 0xff326ba1
                                                    : 0xff107d76;
    Ui::Round(g, 0, 0, 32, 32, 6, background);
    float fontSize = value >= 100 ? 18.f : seconds ? 21.f : 25.f;
    Gdiplus::Font font(L"Segoe UI", fontSize, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    Gdiplus::SolidBrush ink(Gdiplus::Color(255, 255, 255, 255));
    Gdiplus::StringFormat format(Gdiplus::StringFormat::GenericTypographic());
    format.SetAlignment(Gdiplus::StringAlignmentCenter);
    format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    format.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap | Gdiplus::StringFormatFlagsNoClip);
    auto label = std::to_wstring(value);
    g.DrawString(label.c_str(), -1, &font, Gdiplus::RectF(0, 0, seconds ? 25.f : 32.f, 31),
                 &format, &ink);
    if (seconds)
        Ui::Text(g, L"s", 24, 17, 8, 13, 10, 0xffffffff, true, 1, L"Segoe UI");
    HICON icon{};
    bitmap.GetHICON(&icon);
    return icon;
}
} // namespace
TrayIcon::TrayIcon(HINSTANCE, HWND hwnd, TimerManager *timer) : m_hwnd(hwnd), m_timer(timer)
{
    m_nid.cbSize = sizeof(m_nid);
    m_nid.hWnd = hwnd;
    m_nid.uID = 1;
    m_nid.uCallbackMessage = WM_USER + 1;
    m_nid.hIcon = Ui::Icon();
    m_nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    StringCchCopyW(m_nid.szTip, 128, L"WorkWell · 准备开始");
    Restore();
    UpdateTimer(timer->Remaining(), false);
}
void TrayIcon::Restore()
{
    m_nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    Shell_NotifyIconW(NIM_ADD, &m_nid);
    m_nid.uVersion = NOTIFYICON_VERSION;
    Shell_NotifyIconW(NIM_SETVERSION, &m_nid);
}
TrayIcon::~TrayIcon()
{
    Shell_NotifyIconW(NIM_DELETE, &m_nid);
    if (m_countdownIcon)
        DestroyIcon(m_countdownIcon);
}
void TrayIcon::UpdateTimer(int seconds, bool)
{
    seconds = std::max(0, seconds);
    std::wstring tooltip = L"WorkWell · " +
                           std::wstring(m_timer->IsInBreak()    ? L"休息中"
                                        : m_timer->IsPaused()   ? L"已暂停"
                                        : m_timer->IsStopped()  ? L"准备开始"
                                        : m_timer->IsDeferred() ? L"休息已延后"
                                                                : L"专注中") +
                           L" · " + Ui::Time(seconds);
    StringCchCopyW(m_nid.szTip, 128, tooltip.c_str());
    m_nid.uFlags = NIF_TIP;

    // The tray is too small for MM:SS. Show rounded-up minutes, then seconds
    // with an 's' marker during the last minute; the tooltip is always exact.
    bool showSeconds = seconds < 60;
    int value = showSeconds ? seconds : (seconds + 59) / 60;
    int key = m_timer->IsStopped()
                  ? -1
                  : value | (showSeconds ? 0x100 : 0) | (static_cast<int>(m_timer->GetState()) << 12);
    HICON next{};
    if (key != m_iconKey)
    {
        if (key != -1)
            next = CreateCountdownIcon(value, showSeconds, m_timer->GetState());
        if (key == -1 || next)
        {
            m_nid.hIcon = next ? next : Ui::Icon();
            m_nid.uFlags |= NIF_ICON;
        }
    }
    if (Shell_NotifyIconW(NIM_MODIFY, &m_nid))
    {
        if (m_nid.uFlags & NIF_ICON)
        {
            if (m_countdownIcon)
                DestroyIcon(m_countdownIcon);
            m_countdownIcon = next;
            m_iconKey = key;
        }
    }
    else
    {
        if (next)
            DestroyIcon(next);
        m_nid.hIcon = m_countdownIcon ? m_countdownIcon : Ui::Icon();
    }
}
void TrayIcon::SetBreakState(bool)
{
    UpdateTimer(m_timer->Remaining(), false);
}
void TrayIcon::ShowWarning()
{
    m_nid.uFlags = NIF_INFO;
    m_nid.dwInfoFlags = NIIF_INFO;
    StringCchCopyW(m_nid.szInfoTitle, 64, L"30 秒后休息");
    StringCchCopyW(m_nid.szInfo, 256, L"本轮专注即将完成，请保存当前工作。起身活动一下吧。");
    Shell_NotifyIconW(NIM_MODIFY, &m_nid);
}
void TrayIcon::ShowPopupMenu()
{
    HMENU menu = CreatePopupMenu();
    AppendMenuW(menu, MF_STRING, 1000, L"打开专注面板");
    const wchar_t *label = m_timer->IsStopped()  ? L"开始专注"
                           : m_timer->IsPaused() ? L"继续专注"
                                                 : L"暂停专注";
    AppendMenuW(menu, MF_STRING | (m_timer->IsInBreak() ? MF_GRAYED : 0), 1001, label);
    AppendMenuW(menu, MF_STRING | (m_timer->IsInBreak() ? MF_GRAYED : 0), 1002, L"立即休息");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, 1003, L"退出 WorkWell");
    POINT p;
    GetCursorPos(&p);
    SetForegroundWindow(m_hwnd);
    UINT cmd = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON, p.x, p.y, 0, m_hwnd, nullptr);
    DestroyMenu(menu);
    PostMessageW(m_hwnd, WM_NULL, 0, 0);
    if (cmd == 1000 && m_dialog)
        m_dialog->Show();
    else if (cmd == 1001)
    {
        if (m_timer->IsStopped())
            m_timer->StartWork(m_timer->GetSettings().workDuration);
        else if (m_timer->IsPaused())
            m_timer->Resume();
        else
            m_timer->Pause();
    }
    else if (cmd == 1002)
        m_timer->StartBreak();
    else if (cmd == 1003)
        PostMessageW(m_hwnd, WM_CLOSE, 0, 0);
}
LRESULT TrayIcon::HandleTrayMessage(WPARAM wp, LPARAM lp)
{
    if (wp != 1)
        return 0;
    switch (LOWORD(lp))
    {
    case WM_LBUTTONUP:
    case NIN_KEYSELECT:
        if (m_dialog)
            m_dialog->Show();
        break;
    case WM_RBUTTONUP:
    case WM_CONTEXTMENU:
        ShowPopupMenu();
        break;
    }
    return 0;
}
