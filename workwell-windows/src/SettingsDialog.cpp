#include "SettingsDialog.h"
#include "Ui.h"
SettingsDialog::SettingsDialog(HINSTANCE instance, HWND parent, const Settings &s)
    : m_instance(instance), m_parent(parent), m_saved(s), m_canSkip(s.canSkipBreak)
{
    WNDCLASSW wc{};
    wc.hInstance = instance;
    wc.lpfnWndProc = Proc;
    wc.lpszClassName = L"WorkWellSettingsV2";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);
    m_hwnd = CreateWindowExW(0, wc.lpszClassName, L"WorkWell · 设置",
                             WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN, 0, 0, 450, 760,
                             nullptr, nullptr, instance, this);
    Ui::Setup(m_hwnd, 446, 730, parent);
    const int mins[] = {1, 1, 1, 2}, maxs[] = {120, 30, 60, 10};
    const wchar_t *labels[] = {L"专注时长", L"短休息", L"长休息", L"长休息间隔"};
    for (int i = 0; i < 4; ++i)
    {
        Ui::Label(m_hwnd, 2100 + i, labels[i], 48, 119 + i * 55, 185, 38);
        m_edits[i] = CreateWindowExW(
            0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_NUMBER | ES_CENTER | ES_AUTOHSCROLL, 0,
            0, 0, 0, m_hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(1100 + i)), instance, nullptr);
        SendMessageW(m_edits[i], EM_SETLIMITTEXT, 3, 0);
        m_spins[i] = CreateWindowExW(0, UPDOWN_CLASSW, L"",
                                     WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_SETBUDDYINT,
                                     0, 0, 0, 0, m_hwnd, nullptr, instance, nullptr);
        SendMessageW(m_spins[i], UDM_SETBUDDY, reinterpret_cast<WPARAM>(m_edits[i]), 0);
        SendMessageW(m_spins[i], UDM_SETRANGE32, mins[i], maxs[i]);
    }
    m_modes[0] = Ui::Button(m_hwnd, 1200, L"温和模式\n允许延后或跳过", 28, 415, 189, 82, 3);
    m_modes[1] = Ui::Button(m_hwnd, 1201, L"严格模式\n按计划全屏休息", 229, 415, 189, 82, 4);
    Ui::Button(m_hwnd, 1300, L"取消", 214, 663, 88, 44, 2);
    Ui::Button(m_hwnd, 1301, L"保存设置", 314, 663, 104, 44);
    Layout();
    Restore();
}
SettingsDialog::~SettingsDialog()
{
    if (IsWindow(m_hwnd))
        DestroyWindow(m_hwnd);
    if (m_font)
        DeleteObject(m_font);
}
void SettingsDialog::Layout()
{
    if (m_font)
        DeleteObject(m_font);
    m_font = CreateFontW(-Ui::Px(m_hwnd, 16), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 0, 0,
                         CLEARTYPE_QUALITY, 0, L"Microsoft YaHei UI");
    for (int i = 0; i < 4; ++i)
    {
        Ui::Move(GetDlgItem(m_hwnd, 2100 + i), m_hwnd, 48, 119 + i * 55, 185, 38);
        Ui::Move(m_edits[i], m_hwnd, 258, 125 + i * 55, 66, 28);
        Ui::Move(m_spins[i], m_hwnd, 324, 122 + i * 55, 22, 34);
        SendMessageW(m_edits[i], WM_SETFONT, reinterpret_cast<WPARAM>(m_font), TRUE);
    }
    Ui::Move(m_modes[0], m_hwnd, 28, 415, 189, 82);
    Ui::Move(m_modes[1], m_hwnd, 229, 415, 189, 82);
    Ui::Move(GetDlgItem(m_hwnd, 1300), m_hwnd, 214, 663, 88, 44);
    Ui::Move(GetDlgItem(m_hwnd, 1301), m_hwnd, 314, 663, 104, 44);
}
void SettingsDialog::Restore()
{
    int values[] = {m_saved.workDuration, m_saved.shortBreakDuration, m_saved.longBreakDuration,
                    m_saved.pomodorosUntilLongBreak};
    for (int i = 0; i < 4; ++i)
        SetWindowTextW(m_edits[i], std::to_wstring(values[i]).c_str());
    m_canSkip = m_saved.canSkipBreak;
    m_error = -1;
    for (int i = 0; i < 2; ++i)
    {
        SetWindowLongPtrW(m_modes[i], GWLP_USERDATA, (i == 0) == m_canSkip ? 3 : 4);
        InvalidateRect(m_modes[i], nullptr, FALSE);
    }
    InvalidateRect(m_hwnd, nullptr, FALSE);
}
void SettingsDialog::Show()
{
    Restore();
    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);
    SetFocus(m_edits[0]);
}
void SettingsDialog::Hide()
{
    ShowWindow(m_hwnd, SW_HIDE);
}
bool SettingsDialog::Apply()
{
    int v[4];
    const int lo[] = {1, 1, 1, 2}, hi[] = {120, 30, 60, 10};
    for (int i = 0; i < 4; ++i)
    {
        BOOL ok;
        v[i] = GetDlgItemInt(m_hwnd, 1100 + i, &ok, FALSE);
        if (!ok || v[i] < lo[i] || v[i] > hi[i])
        {
            m_error = i;
            SetFocus(m_edits[i]);
            SendMessageW(m_edits[i], EM_SETSEL, 0, -1);
            InvalidateRect(m_hwnd, nullptr, FALSE);
            return false;
        }
    }
    Settings s;
    s.workDuration = v[0];
    s.shortBreakDuration = v[1];
    s.longBreakDuration = v[2];
    s.pomodorosUntilLongBreak = v[3];
    s.canSkipBreak = m_canSkip;
    if (m_changed && !m_changed(s))
        return false;
    m_saved = s;
    Hide();
    return true;
}
void SettingsDialog::Paint()
{
    Ui::Paint p(m_hwnd);
    auto &g = *p.g;
    Ui::Text(g, L"设置", 24, 15, 390, 53, 28, Ui::Ink, true);
    Ui::Round(g, 16, 79, 414, 279, 9, Ui::Bg, Ui::Line);
    Ui::Text(g, L"专注节奏", 32, 87, 370, 35, 17, Ui::Ink, true);
    for (int i = 0; i < 4; ++i)
    {
        float y = 119.f + i * 55;
        Ui::Round(g, 250, y, 98, 38, 5, 0xffffffff, m_error == i ? 0xffbf493f : Ui::Line);
        Ui::Text(g, i == 3 ? L"轮" : L"分钟", 363, y, 50, 38, 14, Ui::Muted);
        if (i < 3)
            Ui::LineAt(g, 32, y + 47, 414, y + 47);
    }
    Ui::Text(g, L"休息方式", 28, 372, 390, 35, 17, Ui::Ink, true);
    Ui::Round(g, 28, 517, 390, 120, 8, Ui::Bg, Ui::Line);
    Ui::Text(g, L"你的专注节奏", 44, 528, 350, 31, 15, Ui::Ink, true);
    int v[4];
    for (int i = 0; i < 4; ++i)
        v[i] = GetDlgItemInt(m_hwnd, 1100 + i, nullptr, FALSE);
    Ui::Text(g, L"专注 " + std::to_wstring(v[0]) + L" 分钟  →  休息 " + std::to_wstring(v[1]) + L" 分钟", 44,
             564, 350, 28, 14, Ui::Muted);
    Ui::Text(g, L"每完成 " + std::to_wstring(v[3]) + L" 轮，长休息 " + std::to_wstring(v[2]) + L" 分钟", 44,
             593, 350, 28, 14, Ui::Muted);
    Ui::Text(g, m_error >= 0 ? L"请输入范围内的整数" : L"ⓘ 新设置从下一轮生效", 20, 664, 190, 44, 12,
             m_error >= 0 ? 0xffbf493f : Ui::Muted);
    if (m_error >= 0)
    {
        const wchar_t *ranges[] = {L"专注时长：1–120 分钟", L"短休息：1–30 分钟", L"长休息：1–60 分钟",
                                   L"长休息间隔：2–10 轮"};
        Ui::Text(g, ranges[m_error], 28, 638, 390, 24, 12, 0xffbf493f);
    }
}
LRESULT CALLBACK SettingsDialog::Proc(HWND w, UINT msg, WPARAM wp, LPARAM lp)
{
    auto *self = reinterpret_cast<SettingsDialog *>(GetWindowLongPtrW(w, GWLP_USERDATA));
    if (msg == WM_NCCREATE)
    {
        self = static_cast<SettingsDialog *>(reinterpret_cast<CREATESTRUCTW *>(lp)->lpCreateParams);
        SetWindowLongPtrW(w, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    }
    if (!self)
        return DefWindowProcW(w, msg, wp, lp);
    switch (msg)
    {
    case WM_PAINT:
        self->Paint();
        return 0;
    case WM_ERASEBKGND:
        return 1;
    case WM_DRAWITEM:
        Ui::DrawButton(reinterpret_cast<DRAWITEMSTRUCT *>(lp));
        return TRUE;
    case WM_CTLCOLOREDIT:
    {
        SetTextColor(reinterpret_cast<HDC>(wp), RGB(20, 33, 45));
        SetBkColor(reinterpret_cast<HDC>(wp), RGB(255, 255, 255));
        return reinterpret_cast<LRESULT>(GetStockObject(WHITE_BRUSH));
    }
    case WM_COMMAND:
    {
        int id = LOWORD(wp);
        if (id == 1301)
        {
            self->Apply();
            return 0;
        }
        if (id == 1300 || id == IDCANCEL)
        {
            self->Hide();
            return 0;
        }
        if (id == IDOK)
        {
            self->Apply();
            return 0;
        }
        if (id == 1200 || id == 1201)
        {
            self->m_canSkip = id == 1200;
            for (int i = 0; i < 2; ++i)
            {
                SetWindowLongPtrW(self->m_modes[i], GWLP_USERDATA, (i == 0) == self->m_canSkip ? 3 : 4);
                InvalidateRect(self->m_modes[i], nullptr, FALSE);
            }
        }
        if (HIWORD(wp) == EN_CHANGE)
            InvalidateRect(w, nullptr, FALSE);
        return 0;
    }
    case WM_DPICHANGED:
    {
        auto *r = reinterpret_cast<RECT *>(lp);
        SetWindowPos(w, nullptr, r->left, r->top, r->right - r->left, r->bottom - r->top,
                     SWP_NOZORDER | SWP_NOACTIVATE);
        self->Layout();
        return 0;
    }
    case WM_CLOSE:
        self->Hide();
        return 0;
    }
    return DefWindowProcW(w, msg, wp, lp);
}
