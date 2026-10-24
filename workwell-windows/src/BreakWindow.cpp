#include "BreakWindow.h"
#include "TimerManager.h"
#include "Ui.h"

namespace
{
constexpr BYTE FullscreenOpacity = 179; // 70% opacity; retain normal mouse input.
}
BreakWindow::BreakWindow(HINSTANCE instance) : m_instance(instance)
{
    WNDCLASSW wc{};
    wc.hInstance = instance;
    wc.lpfnWndProc = Proc;
    wc.lpszClassName = L"WorkWellBreakV2";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);
}
BreakWindow::~BreakWindow()
{
    Hide();
    for (HWND w : m_windows)
        DestroyWindow(w);
}
BOOL CALLBACK BreakWindow::MonitorProc(HMONITOR monitor, HDC, LPRECT, LPARAM data)
{
    auto *self = reinterpret_cast<BreakWindow *>(data);
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(monitor, &mi);
    RECT r = mi.rcMonitor;
    HWND w = CreateWindowExW(WS_EX_TOPMOST | WS_EX_LAYERED, L"WorkWellBreakV2", L"WorkWell · 休息", WS_POPUP, r.left, r.top,
                             r.right - r.left, r.bottom - r.top, nullptr, nullptr, self->m_instance, self);
    SetLayeredWindowAttributes(w, 0, FullscreenOpacity, LWA_ALPHA);
    Ui::Button(w, 1401, L"跳过本次休息", 0, 0, 180, 44, 5);
    Ui::Button(w, 1402, L"延后 5 分钟", 0, 0, 140, 32, 5);
    Ui::Button(w, 1400, L"退出全屏", 0, 0, 104, 36, 5);
    self->m_windows.push_back(w);
    self->Layout(w);
    return TRUE;
}
void BreakWindow::Show(int)
{
    // Re-enumerate monitors for each break to handle docking and display changes.
    for (HWND w : m_windows)
        DestroyWindow(w);
    m_windows.clear();
    m_windowed = false;
    m_escapeStart = 0;
    EnumDisplayMonitors(nullptr, nullptr, MonitorProc, reinterpret_cast<LPARAM>(this));
    m_visible = true;
    for (HWND w : m_windows)
    {
        ShowWindow(w, SW_SHOW);
        InvalidateRect(w, nullptr, FALSE);
        SetTimer(w, 1, 100, nullptr);
    }
    if (!m_windows.empty())
    {
        SetForegroundWindow(m_windows[0]);
        SetFocus(m_windows[0]);
    }
}
void BreakWindow::Hide()
{
    m_visible = false;
    m_escapeStart = 0;
    for (HWND w : m_windows)
    {
        KillTimer(w, 1);
        ShowWindow(w, SW_HIDE);
    }
}
void BreakWindow::Refresh()
{
    if (m_visible)
        for (HWND w : m_windows)
            InvalidateRect(w, nullptr, FALSE);
}
bool BreakWindow::Translate(MSG *msg)
{
    if (!m_visible)
        return false;
    for (HWND w : m_windows)
        if (IsWindowVisible(w) && IsDialogMessageW(w, msg))
            return true;
    return false;
}
void BreakWindow::Layout(HWND w)
{
    if (!m_timer)
        return;
    RECT r;
    GetClientRect(w, &r);
    float s = Ui::Scale(w);
    int width = static_cast<int>(r.right / s), height = static_cast<int>(r.bottom / s);
    int cy = height / 2;
    float zoom = std::clamp(std::min(width / 680.f, height / 490.f), 1.f, 2.f);
    Ui::Move(GetDlgItem(w, 1401), w, width / 2 - static_cast<int>(90 * zoom),
             cy + static_cast<int>(115 * zoom), static_cast<int>(180 * zoom), static_cast<int>(44 * zoom));
    Ui::Move(GetDlgItem(w, 1400), w, width - 132, 24, 104, 36);
    Ui::Move(GetDlgItem(w, 1402), w, width / 2 - static_cast<int>(70 * zoom),
             cy + static_cast<int>(205 * zoom), static_cast<int>(140 * zoom), static_cast<int>(32 * zoom));
    bool soft = m_timer->ActiveSettings().canSkipBreak;
    ShowWindow(GetDlgItem(w, 1402), soft ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(w, 1401), soft ? SW_SHOW : SW_HIDE);
    ShowWindow(GetDlgItem(w, 1400), soft ? SW_SHOW : SW_HIDE);
}
void BreakWindow::ToggleFullscreen(HWND w)
{
    if (!m_timer->ActiveSettings().canSkipBreak)
        return;
    m_windowed = !m_windowed;
    if (m_windowed)
    {
        SetLayeredWindowAttributes(w, 0, 255, LWA_ALPHA);
        for (HWND other : m_windows)
            if (other != w)
                ShowWindow(other, SW_HIDE);
        SetWindowLongPtrW(w, GWL_STYLE, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE);
        Ui::Setup(w, 680, 490);
        DWORD caption = RGB(18, 59, 53), text = RGB(237, 247, 243);
        BOOL dark = TRUE;
        DwmSetWindowAttribute(w, 20, &dark, sizeof(dark));
        DwmSetWindowAttribute(w, 35, &caption, sizeof(caption));
        DwmSetWindowAttribute(w, 36, &text, sizeof(text));
        SetWindowPos(w, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
        SetWindowTextW(GetDlgItem(w, 1400), L"进入全屏");
    }
    else
    {
        MONITORINFO mi{sizeof(mi)};
        GetMonitorInfoW(MonitorFromWindow(w, MONITOR_DEFAULTTONEAREST), &mi);
        SetWindowLongPtrW(w, GWL_STYLE, WS_POPUP | WS_VISIBLE);
        SetWindowPos(w, HWND_TOPMOST, mi.rcMonitor.left, mi.rcMonitor.top,
                     mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
                     SWP_FRAMECHANGED);
        SetWindowTextW(GetDlgItem(w, 1400), L"退出全屏");
        for (HWND other : m_windows)
        {
            SetLayeredWindowAttributes(other, 0, FullscreenOpacity, LWA_ALPHA);
            ShowWindow(other, SW_SHOW);
        }
    }
    Layout(w);
    InvalidateRect(w, nullptr, FALSE);
}
void BreakWindow::Paint(HWND w)
{
    Ui::Paint p(w, Ui::Dark);
    auto &g = *p.g;
    RECT r;
    GetClientRect(w, &r);
    float width = r.right / Ui::Scale(w), height = r.bottom / Ui::Scale(w), cx = width / 2, cy = height / 2;
    Ui::Text(g, L"◷  WorkWell", 24, 20, 180, 36, 15, 0xffe5f5f1, true);
    float zoom = std::clamp(std::min(width / 680.f, height / 490.f), 1.f, 2.f);
    g.TranslateTransform(cx, cy);
    g.ScaleTransform(zoom, zoom);
    cx = cy = 0;
    Ui::Round(g, cx - 95, cy - 160, 190, 38, 19, 0xff244b44, 0xff396057);
    Ui::Text(g, L"本轮专注已完成", cx - 90, cy - 160, 180, 38, 14, 0xffe4f3ef, true, 1);
    Ui::Text(g, Ui::Time(m_timer->Remaining()), cx - 240, cy - 99, 480, 112, 82, 0xfff5faf8, true, 1,
             L"Segoe UI");
    Ui::Text(g, L"起身走动，看看远处", cx - 260, cy + 18, 520, 45, 25, 0xffedf7f3, false, 1);
    Ui::Text(g, L"给眼睛和身体一点恢复时间", cx - 260, cy + 65, 520, 30, 15, 0xffc6dbd4, false, 1);
    bool soft = m_timer->ActiveSettings().canSkipBreak;
    Ui::Text(g,
             soft            ? L"温和模式"
             : m_escapeStart ? L"继续按住 Esc，3 秒后结束休息"
                             : L"严格模式 · 紧急情况可长按 Esc 3 秒",
             cx - 280, cy + 170, 560, 30, 12, 0xffa8c6bc, false, 1);
}
LRESULT CALLBACK BreakWindow::Proc(HWND w, UINT msg, WPARAM wp, LPARAM lp)
{
    auto *self = reinterpret_cast<BreakWindow *>(GetWindowLongPtrW(w, GWLP_USERDATA));
    if (msg == WM_NCCREATE)
    {
        self = static_cast<BreakWindow *>(reinterpret_cast<CREATESTRUCTW *>(lp)->lpCreateParams);
        SetWindowLongPtrW(w, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    if (!self)
        return DefWindowProcW(w, msg, wp, lp);
    switch (msg)
    {
    case WM_PAINT:
        self->Paint(w);
        return 0;
    case WM_ERASEBKGND:
        return 1;
    case WM_DRAWITEM:
        Ui::DrawButton(reinterpret_cast<DRAWITEMSTRUCT *>(lp));
        return TRUE;
    case WM_TIMER:
        if (self->m_visible)
        {
            bool wasHolding = self->m_escapeStart != 0;
            if ((GetForegroundWindow() == w || IsChild(w, GetForegroundWindow())) &&
                (GetAsyncKeyState(VK_ESCAPE) & 0x8000))
            {
                if (!self->m_escapeStart)
                    self->m_escapeStart = GetTickCount64();
                if (GetTickCount64() - self->m_escapeStart >= 3000)
                {
                    self->m_timer->EndBreak();
                    return 0;
                }
            }
            else
                self->m_escapeStart = 0;
            if (wasHolding != (self->m_escapeStart != 0))
                InvalidateRect(w, nullptr, FALSE);
        }
        return 0;
    case WM_SIZE:
        if (self->m_timer)
            self->Layout(w);
        return 0;
    case WM_DPICHANGED:
    {
        auto *r = reinterpret_cast<RECT *>(lp);
        SetWindowPos(w, nullptr, r->left, r->top, r->right - r->left, r->bottom - r->top, SWP_NOZORDER);
        self->Layout(w);
        return 0;
    }
    case WM_COMMAND:
        if (LOWORD(wp) == 1401 && self->m_timer->ActiveSettings().canSkipBreak)
            self->m_timer->EndBreak();
        else if (LOWORD(wp) == 1402)
            self->m_timer->DelayBreak();
        else if (LOWORD(wp) == 1400)
            self->ToggleFullscreen(w);
        return 0;
    case WM_CLOSE:
        if (self->m_timer && self->m_timer->ActiveSettings().canSkipBreak)
            self->m_timer->EndBreak();
        return 0;
    }
    return DefWindowProcW(w, msg, wp, lp);
}
