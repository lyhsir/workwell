#include "MainDialog.h"
#include "TimerManager.h"
#include "SettingsDialog.h"
#include "ConfigManager.h"
#include "Ui.h"
MainDialog::MainDialog(HINSTANCE instance, TimerManager *timer) : m_instance(instance), m_timer(timer)
{
    WNDCLASSW wc{};
    wc.hInstance = instance;
    wc.lpfnWndProc = Proc;
    wc.lpszClassName = L"WorkWellMainV2";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);
    m_hwnd = CreateWindowExW(0, wc.lpszClassName, L"WorkWell",
                             WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN, 0, 0,
                             420, 760, nullptr, nullptr, instance, this);
    Ui::Setup(m_hwnd, 420, 730);
    HMENU sys = GetSystemMenu(m_hwnd, FALSE);
    AppendMenuW(sys, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(sys, MF_STRING, 0x1ff0, L"立即休息\tCtrl+Shift+B");
    Ui::Button(m_hwnd, 1004, L"⚙  设置", 322, 16, 76, 36, 1);
    m_primary = Ui::Button(m_hwnd, 1002, L"开始专注", 24, 514, 372, 56);
    m_stop = Ui::Button(m_hwnd, 1003, L"结束本轮", 142, 580, 136, 40, 1);
    Ui::Label(m_hwnd, 1005, L"25:00\n剩余专注时间", 95, 166, 230, 114);
    m_complete =
        CreateWindowExW(0, wc.lpszClassName, L"WorkWell · 休息完成", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
                        0, 0, 480, 360, nullptr, nullptr, instance, this);
    Ui::Setup(m_complete, 480, 335, m_hwnd);
    Ui::Button(m_complete, 1010, L"开始下一轮 · 25 分钟", 64, 219, 352, 48);
    Ui::Button(m_complete, 1011, L"稍后开始", 175, 280, 130, 36, 1);
    const auto &s = timer->GetSettings();
    SettingsDialog::Settings initial;
    initial.workDuration = s.workDuration / 60;
    initial.shortBreakDuration = s.shortBreakDuration / 60;
    initial.longBreakDuration = s.longBreakDuration / 60;
    initial.pomodorosUntilLongBreak = s.pomodorosUntilLongBreak;
    initial.canSkipBreak = s.canSkipBreak;
    m_settings = std::make_unique<SettingsDialog>(instance, m_hwnd, initial);
    m_settings->SetOnSettingsChanged(
        [this](const SettingsDialog::Settings &v)
        {
            TimerManager::Settings s;
            s.workDuration = v.workDuration * 60;
            s.shortBreakDuration = v.shortBreakDuration * 60;
            s.longBreakDuration = v.longBreakDuration * 60;
            s.pomodorosUntilLongBreak = v.pomodorosUntilLongBreak;
            s.canSkipBreak = v.canSkipBreak;
            if (!ConfigManager::Save(s))
            {
                MessageBoxW(m_settings->Handle(),
                            L"设置未能保存，请检查磁盘空间或目录权限。你的修改仍保留在设置页中。",
                            L"无法保存设置", MB_OK | MB_ICONERROR);
                return false;
            }
            m_timer->SetSettings(s);
            m_toastUntil = GetTickCount64() + 3500;
            SetTimer(m_hwnd, 2, 3500, nullptr);
            Sync();
            return true;
        });
    Sync();
}
MainDialog::~MainDialog()
{
    m_settings.reset();
    if (IsWindow(m_complete))
        DestroyWindow(m_complete);
    if (IsWindow(m_hwnd))
        DestroyWindow(m_hwnd);
}
void MainDialog::Layout()
{
    Ui::Move(GetDlgItem(m_hwnd, 1005), m_hwnd, 95, 166, 230, 114);
    Ui::Move(GetDlgItem(m_hwnd, 1004), m_hwnd, 322, 16, 76, 36);
    Ui::Move(m_primary, m_hwnd, 24, 514, 372, 56);
    Ui::Move(m_stop, m_hwnd, 142, 580, 136, 40);
    Ui::Move(GetDlgItem(m_complete, 1010), m_complete, 64, 219, 352, 48);
    Ui::Move(GetDlgItem(m_complete, 1011), m_complete, 175, 280, 130, 36);
}
void MainDialog::Show()
{
    Sync();
    ShowWindow(m_hwnd, SW_RESTORE);
    SetForegroundWindow(m_hwnd);
}
void MainDialog::Hide()
{
    if (m_settings)
        m_settings->Hide();
    ShowWindow(m_hwnd, SW_HIDE);
}
void MainDialog::HideForBreak()
{
    Hide();
    ShowWindow(m_complete, SW_HIDE);
}
void MainDialog::ShowCompletion()
{
    std::wstring text =
        L"开始下一轮 · " + std::to_wstring(m_timer->GetSettings().workDuration / 60) + L" 分钟";
    SetWindowTextW(GetDlgItem(m_complete, 1010), text.c_str());
    ShowWindow(m_complete, SW_SHOW);
    SetForegroundWindow(m_complete);
    SetFocus(GetDlgItem(m_complete, 1010));
    InvalidateRect(m_complete, nullptr, FALSE);
}
void MainDialog::Toggle()
{
    if (m_timer->IsInBreak())
        return;
    if (m_timer->IsStopped())
        m_timer->StartWork(m_timer->GetSettings().workDuration);
    else if (m_timer->IsPaused())
        m_timer->Resume();
    else
        m_timer->Pause();
    Sync();
}
void MainDialog::Sync()
{
    if (!m_primary)
        return;
    std::wstring timerLabel = Ui::Time(m_timer->Remaining()) + L"\n" +
                              (m_timer->IsInBreak()    ? L"剩余休息时间"
                               : m_timer->IsDeferred() ? L"距离休息开始"
                                                       : L"剩余专注时间");
    SetWindowTextW(GetDlgItem(m_hwnd, 1005), timerLabel.c_str());
    const wchar_t *label = m_timer->IsStopped()   ? L"开始专注"
                           : m_timer->IsPaused()  ? L"▶  继续专注"
                           : m_timer->IsInBreak() ? L"休息中"
                                                  : L"Ⅱ  暂停专注";
    wchar_t old[100];
    GetWindowTextW(m_primary, old, 100);
    if (wcscmp(old, label))
        SetWindowTextW(m_primary, label);
    EnableWindow(m_primary, !m_timer->IsInBreak());
    EnableWindow(m_stop, !m_timer->IsStopped() && !m_timer->IsInBreak());
    if (IsWindowVisible(m_hwnd))
        InvalidateRect(m_hwnd, nullptr, FALSE);
}
void MainDialog::UpdateCountdown(int)
{
    Sync();
}
bool MainDialog::Translate(MSG *msg)
{
    if (msg->message == WM_KEYDOWN && (GetKeyState(VK_CONTROL) & 0x8000) && (GetKeyState(VK_SHIFT) & 0x8000))
    {
        if (msg->wParam == 'B')
        {
            m_timer->StartBreak();
            return true;
        }
        if (msg->wParam == 'P')
        {
            Toggle();
            return true;
        }
        if (msg->wParam == 'S')
        {
            m_timer->Stop();
            return true;
        }
    }
    if (m_settings && m_settings->IsVisible() && IsDialogMessageW(m_settings->Handle(), msg))
        return true;
    if (IsWindowVisible(m_complete) && IsDialogMessageW(m_complete, msg))
        return true;
    return IsWindowVisible(m_hwnd) && IsDialogMessageW(m_hwnd, msg);
}
void MainDialog::Paint(HWND window)
{
    Ui::Paint p(window);
    auto &g = *p.g;
    if (window == m_complete)
    {
        Ui::Dot(g, 201, 24, 78, 0xffdef3ef);
        Gdiplus::Pen pen(Ui::C(Ui::Teal), 6);
        pen.SetStartCap(Gdiplus::LineCapRound);
        pen.SetEndCap(Gdiplus::LineCapRound);
        g.DrawLine(&pen, 224.f, 63.f, 236.f, 75.f);
        g.DrawLine(&pen, 236.f, 75.f, 257.f, 50.f);
        Ui::Text(g, L"休息完成", 30, 119, 420, 45, 27, Ui::Ink, true, 1);
        Ui::Text(g, L"准备好了，再开始下一轮", 30, 165, 420, 30, 16, Ui::Muted, false, 1);
        return;
    }
    const bool paused = m_timer->IsPaused(), idle = m_timer->IsStopped(), rest = m_timer->IsInBreak();
    const auto &s = m_timer->ActiveSettings();
    DWORD stateColor = paused ? 0xffa86c13 : Ui::Teal;
    Ui::Round(g, 24, 18, 105, 35, 17, paused ? 0xfffff4df : Ui::Soft);
    Ui::Dot(g, 39, 31, 9, stateColor);
    Ui::Text(g,
             rest                    ? L"休息中"
             : paused                ? L"已暂停"
             : idle                  ? L"准备开始"
             : m_timer->IsDeferred() ? L"已延后"
                                     : L"专注中",
             56, 18, 65, 35, 14, stateColor, true);
    float fraction = m_timer->Total() > 0 ? static_cast<float>(m_timer->Remaining()) / m_timer->Total() : 1.f;
    Gdiplus::Pen track(Ui::C(0xffe7ebee), 12);
    g.DrawEllipse(&track, 72.f, 79.f, 276.f, 276.f);
    if (fraction > 0)
    {
        Gdiplus::Pen ring(Ui::C(paused ? 0xff7db5af : 0xff24a49c), 12);
        ring.SetStartCap(Gdiplus::LineCapRound);
        ring.SetEndCap(Gdiplus::LineCapRound);
        if (fraction >= 1)
            g.DrawEllipse(&ring, 72.f, 79.f, 276.f, 276.f);
        else
            g.DrawArc(&ring, 72.f, 79.f, 276.f, 276.f, -90.f, 360.f * fraction);
    }
    // The countdown is a named native STATIC for screen-reader access.
    Ui::Text(g,
             rest ? L"让眼睛和身体恢复一下"
                  : m_timer->IsDeferred() ? L"倒计时结束后开始休息"
                  : L"本轮结束后休息 " + std::to_wstring(m_timer->NextBreakSeconds() / 60) + L" 分钟",
             24, 371, 372, 35, 15, Ui::Muted, false, 1);
    int count = s.pomodorosUntilLongBreak, active = m_timer->GetCompletedPomodoros() % count;
    float start = 210.f - (count * 27.f - 10.f) / 2;
    for (int i = 0; i < count; ++i)
        Ui::Dot(g, start + i * 27.f, 425, 17, i == active ? Ui::Teal : i < active ? 0xff90ccc5 : 0xffcdd5da);
    Ui::Text(g,
             L"第 " + std::to_wstring(active + 1) + L" / " + std::to_wstring(count) + L" 轮 · 第 " +
                 std::to_wstring(count) + L" 轮后长休息",
             24, 454, 372, 30, 14, Ui::Muted, false, 1);
    if (GetTickCount64() < m_toastUntil)
        Ui::Text(g, L"设置已保存", 24, 485, 372, 24, 12, Ui::Teal, false, 1);
    Ui::LineAt(g, 24, 634, 396, 634);
    Ui::LineAt(g, 210, 660, 210, 704);
    Ui::Text(g, L"今日完成", 34, 646, 160, 28, 14, Ui::Muted, false, 1);
    Ui::Text(g, std::to_wstring(m_timer->TodayCount()) + L" 轮", 34, 673, 160, 40, 25, Ui::Ink, true, 1);
    Ui::Text(g, L"累计专注", 226, 646, 160, 28, 14, Ui::Muted, false, 1);
    Ui::Text(g, std::to_wstring(m_timer->TodayMinutes()) + L" 分钟", 226, 673, 160, 40, 25, Ui::Ink, true, 1);
}
LRESULT CALLBACK MainDialog::Proc(HWND w, UINT msg, WPARAM wp, LPARAM lp)
{
    auto *self = reinterpret_cast<MainDialog *>(GetWindowLongPtrW(w, GWLP_USERDATA));
    if (msg == WM_NCCREATE)
    {
        self = static_cast<MainDialog *>(reinterpret_cast<CREATESTRUCTW *>(lp)->lpCreateParams);
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
    case WM_COMMAND:
        switch (LOWORD(wp))
        {
        case 1002:
            self->Toggle();
            break;
        case 1003:
            self->m_timer->Stop();
            self->Sync();
            break;
        case 1004:
            if (!self->m_timer->IsInBreak())
                self->m_settings->Show();
            break;
        case 1010:
            ShowWindow(self->m_complete, SW_HIDE);
            self->m_timer->StartWork(self->m_timer->GetSettings().workDuration);
            self->Show();
            break;
        case 1011:
        case IDCANCEL:
            ShowWindow(w, SW_HIDE);
            break;
        case IDOK:
            if (w == self->m_complete)
                SendMessageW(w, WM_COMMAND, 1010, 0);
            else
                self->Toggle();
            break;
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wp & 0xfff0) == SC_MINIMIZE && w == self->m_hwnd)
        {
            self->Hide();
            return 0;
        }
        if ((wp & 0xfff0) == 0x1ff0)
        {
            self->m_timer->StartBreak();
            return 0;
        }
        break;
    case WM_SIZE:
        if (wp == SIZE_MINIMIZED && w == self->m_hwnd)
        {
            self->Hide();
            return 0;
        }
        break;
    case WM_TIMER:
        KillTimer(w, 2);
        InvalidateRect(w, nullptr, FALSE);
        return 0;
    case WM_DPICHANGED:
    {
        auto *r = reinterpret_cast<RECT *>(lp);
        SetWindowPos(w, nullptr, r->left, r->top, r->right - r->left, r->bottom - r->top,
                     SWP_NOZORDER | SWP_NOACTIVATE);
        self->Layout();
        return 0;
    }
    case WM_CLOSE:
        if (w == self->m_hwnd)
            self->Hide();
        else
            ShowWindow(w, SW_HIDE);
        return 0;
    }
    return DefWindowProcW(w, msg, wp, lp);
}
