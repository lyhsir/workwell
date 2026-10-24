#pragma once
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <gdiplus.h>
#include <dwmapi.h>
#include <string>
#include <algorithm>
#include <cmath>
namespace Ui
{
inline constexpr DWORD Bg = 0xfffafbfc, Ink = 0xff14212d, Muted = 0xff657181, Teal = 0xff148f87,
                       Line = 0xffe1e6e9, Soft = 0xffeaf6f4, Dark = 0xff123b35;
inline float Scale(HWND w)
{
    using Fn = UINT(WINAPI *)(HWND);
    static auto fn = reinterpret_cast<Fn>(GetProcAddress(GetModuleHandleW(L"user32"), "GetDpiForWindow"));
    return fn ? fn(w) / 96.f : 1.f;
}
inline HWND Label(HWND parent, int id, const wchar_t *text, int x, int y, int width, int height)
{
    return CreateWindowExW(0, L"STATIC", text, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
                           static_cast<int>(x * Scale(parent)), static_cast<int>(y * Scale(parent)),
                           static_cast<int>(width * Scale(parent)), static_cast<int>(height * Scale(parent)),
                           parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
                           GetModuleHandleW(nullptr), nullptr);
}
inline int Px(HWND w, float v)
{
    return static_cast<int>(std::lround(v * Scale(w)));
}
inline Gdiplus::Color C(DWORD c)
{
    return Gdiplus::Color(c);
}
inline void Quality(Gdiplus::Graphics &g)
{
    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    g.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);
    g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
}
inline void Round(Gdiplus::Graphics &g, float x, float y, float w, float h, float r, DWORD fill,
                  DWORD border = 0)
{
    Gdiplus::GraphicsPath p;
    float d = r * 2;
    p.AddArc(x, y, d, d, 180, 90);
    p.AddArc(x + w - d, y, d, d, 270, 90);
    p.AddArc(x + w - d, y + h - d, d, d, 0, 90);
    p.AddArc(x, y + h - d, d, d, 90, 90);
    p.CloseFigure();
    Gdiplus::SolidBrush b(C(fill));
    g.FillPath(&b, &p);
    if (border)
    {
        Gdiplus::Pen pen(C(border), 1);
        g.DrawPath(&pen, &p);
    }
}
inline void Text(Gdiplus::Graphics &g, const std::wstring &t, float x, float y, float w, float h,
                 float size = 14, DWORD color = Ink, bool bold = false, int align = 0,
                 const wchar_t *family = L"Microsoft YaHei UI")
{
    Gdiplus::Font f(family, size, bold ? Gdiplus::FontStyleBold : Gdiplus::FontStyleRegular,
                    Gdiplus::UnitPixel);
    Gdiplus::SolidBrush b(C(color));
    Gdiplus::StringFormat sf;
    sf.SetAlignment(static_cast<Gdiplus::StringAlignment>(align));
    sf.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    sf.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
    sf.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
    g.DrawString(t.c_str(), -1, &f, Gdiplus::RectF(x, y, w, h), &sf, &b);
}
inline void LineAt(Gdiplus::Graphics &g, float x, float y, float x2, float y2, DWORD c = Line,
                   float width = 1)
{
    Gdiplus::Pen p(C(c), width);
    g.DrawLine(&p, x, y, x2, y2);
}
inline void Dot(Gdiplus::Graphics &g, float x, float y, float d, DWORD c)
{
    Gdiplus::SolidBrush b(C(c));
    g.FillEllipse(&b, x, y, d, d);
}
inline std::wstring Time(int secs)
{
    wchar_t s[30];
    swprintf_s(s, L"%02d:%02d", secs / 60, secs % 60);
    return s;
}
inline HICON Icon()
{
    static HICON icon = []()
    {
        Gdiplus::Bitmap bmp(32, 32, PixelFormat32bppARGB);
        Gdiplus::Graphics g(&bmp);
        Quality(g);
        g.Clear(Gdiplus::Color(0, 0, 0, 0));
        Dot(g, 1, 1, 30, Teal);
        Gdiplus::Pen p(C(0xffffffff), 2.4f);
        p.SetStartCap(Gdiplus::LineCapRound);
        p.SetEndCap(Gdiplus::LineCapRound);
        g.DrawLine(&p, 16.f, 7.f, 16.f, 17.f);
        g.DrawLine(&p, 16.f, 17.f, 23.f, 17.f);
        HICON i = nullptr;
        bmp.GetHICON(&i);
        return i;
    }();
    return icon;
}
inline void Setup(HWND w, int width, int height, HWND owner = nullptr)
{
    float s = Scale(w);
    RECT r{0, 0, static_cast<LONG>(width * s), static_cast<LONG>(height * s)};
    using AdjustFn = BOOL(WINAPI *)(LPRECT, DWORD, BOOL, DWORD, UINT);
    static auto adjust =
        reinterpret_cast<AdjustFn>(GetProcAddress(GetModuleHandleW(L"user32"), "AdjustWindowRectExForDpi"));
    if (adjust)
        adjust(&r, static_cast<DWORD>(GetWindowLongPtrW(w, GWL_STYLE)), FALSE,
               static_cast<DWORD>(GetWindowLongPtrW(w, GWL_EXSTYLE)), static_cast<UINT>(s * 96));
    else
        AdjustWindowRectEx(&r, static_cast<DWORD>(GetWindowLongPtrW(w, GWL_STYLE)), FALSE,
                           static_cast<DWORD>(GetWindowLongPtrW(w, GWL_EXSTYLE)));
    HMONITOR mon = MonitorFromWindow(owner ? owner : w, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi{sizeof(mi)};
    GetMonitorInfoW(mon, &mi);
    int ww = r.right - r.left, hh = r.bottom - r.top;
    SetWindowPos(w, nullptr, mi.rcWork.left + (mi.rcWork.right - mi.rcWork.left - ww) / 2,
                 mi.rcWork.top + (mi.rcWork.bottom - mi.rcWork.top - hh) / 2, ww, hh,
                 SWP_NOZORDER | SWP_NOACTIVATE);
    SendMessageW(w, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(Icon()));
    SendMessageW(w, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(Icon()));
    DWORD corner = 2;
    DwmSetWindowAttribute(w, 33, &corner, sizeof(corner));
    DWORD caption = RGB(250, 251, 252);
    DwmSetWindowAttribute(w, 35, &caption, sizeof(caption));
}
struct Paint
{
    HWND w;
    PAINTSTRUCT ps{};
    HDC dc, mem;
    HBITMAP bitmap, old;
    Gdiplus::Graphics *g;
    Paint(HWND win, DWORD bg = Bg) : w(win)
    {
        dc = BeginPaint(w, &ps);
        RECT r;
        GetClientRect(w, &r);
        mem = CreateCompatibleDC(dc);
        bitmap = CreateCompatibleBitmap(dc, std::max(1L, r.right), std::max(1L, r.bottom));
        old = static_cast<HBITMAP>(SelectObject(mem, bitmap));
        g = new Gdiplus::Graphics(mem);
        Quality(*g);
        g->Clear(C(bg));
        g->ScaleTransform(Scale(w), Scale(w));
    }
    ~Paint()
    {
        delete g;
        RECT r;
        GetClientRect(w, &r);
        BitBlt(dc, 0, 0, r.right, r.bottom, mem, 0, 0, SRCCOPY);
        SelectObject(mem, old);
        DeleteObject(bitmap);
        DeleteDC(mem);
        EndPaint(w, &ps);
    }
};
inline LRESULT CALLBACK ButtonProc(HWND w, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR, DWORD_PTR)
{
    if (msg == WM_MOUSEMOVE)
    {
        if (!GetPropW(w, L"hover"))
        {
            SetPropW(w, L"hover", reinterpret_cast<HANDLE>(1));
            TRACKMOUSEEVENT t{sizeof(t), TME_LEAVE, w, 0};
            TrackMouseEvent(&t);
            InvalidateRect(w, nullptr, FALSE);
        }
    }
    if (msg == WM_MOUSELEAVE)
    {
        RemovePropW(w, L"hover");
        InvalidateRect(w, nullptr, FALSE);
    }
    if (msg == WM_NCDESTROY)
    {
        RemoveWindowSubclass(w, ButtonProc, 1);
        RemovePropW(w, L"hover");
    }
    return DefSubclassProc(w, msg, wp, lp);
}
// kind: 0 primary, 1 text, 2 outline, 3 selected mode, 4 unselected mode, 5 dark outline
inline HWND Button(HWND parent, int id, const wchar_t *text, int x, int y, int width, int height,
                   int kind = 0)
{
    HWND b = CreateWindowExW(0, L"BUTTON", text, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
                             Px(parent, static_cast<float>(x)), Px(parent, static_cast<float>(y)),
                             Px(parent, static_cast<float>(width)), Px(parent, static_cast<float>(height)),
                             parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
                             GetModuleHandleW(nullptr), nullptr);
    SetWindowLongPtrW(b, GWLP_USERDATA, kind);
    SetWindowSubclass(b, ButtonProc, 1, 0);
    return b;
}
inline void DrawButton(const DRAWITEMSTRUCT *d)
{
    if (d->CtlType == ODT_STATIC)
    {
        Gdiplus::Graphics g(d->hDC);
        Quality(g);
        float s = Scale(d->hwndItem);
        g.ScaleTransform(s, s);
        g.Clear(C(Bg));
        wchar_t text[150];
        GetWindowTextW(d->hwndItem, text, 150);
        float width = (d->rcItem.right - d->rcItem.left) / s, height = (d->rcItem.bottom - d->rcItem.top) / s;
        std::wstring label = text;
        auto split = label.find(L'\n');
        if (split != std::wstring::npos)
        {
            Text(g, label.substr(0, split), 0, 0, width, 88, 64, Ink, true, 1, L"Segoe UI");
            Text(g, label.substr(split + 1), 0, 82, width, 32, 15, Muted, false, 1);
        }
        else
            Text(g, label, 0, 0, width, height, 15);
        return;
    }
    HWND w = d->hwndItem;
    Gdiplus::Graphics g(d->hDC);
    Quality(g);
    float s = Scale(w);
    g.ScaleTransform(s, s);
    float width = (d->rcItem.right - d->rcItem.left) / s, height = (d->rcItem.bottom - d->rcItem.top) / s;
    int kind = static_cast<int>(GetWindowLongPtrW(w, GWLP_USERDATA));
    bool disabled = (d->itemState & ODS_DISABLED) != 0, pressed = (d->itemState & ODS_SELECTED) != 0,
         hover = GetPropW(w, L"hover") != nullptr;
    DWORD background = kind == 5 ? Dark : Bg, fg = kind == 5 ? 0xffeaf6f4 : Ink;
    g.Clear(C(background));
    if (kind == 0)
    {
        background = disabled ? 0xffcbd6d5 : pressed ? 0xff0c706a : hover ? 0xff0d817a : Teal;
        fg = 0xffffffff;
        Round(g, 1, 1, width - 2, height - 2, 7, background);
    }
    if (kind == 2 || kind == 3 || kind == 4 || kind == 5)
    {
        Round(g, 1, 1, width - 2, height - 2, kind == 5 ? height / 2 - 1 : 7, kind == 3 ? Soft : background,
              kind == 3   ? Teal
              : kind == 5 ? 0xff91b6ae
                          : Line);
    }
    if (kind == 1 && hover)
        Round(g, 1, 1, width - 2, height - 2, 6, Soft);
    wchar_t label[200];
    GetWindowTextW(w, label, 200);
    std::wstring t = label;
    if (kind == 3 || kind == 4)
    {
        Dot(g, 18, 20, 19, kind == 3 ? Teal : 0xffa4afb7);
        Dot(g, 20, 22, 15, kind == 3 ? Soft : Bg);
        if (kind == 3)
            Dot(g, 23, 25, 9, Teal);
        auto split = t.find(L'\n');
        Text(g, t.substr(0, split), 46, 14, width - 52, 28, 15, Ink, true);
        if (split != std::wstring::npos)
            Text(g, t.substr(split + 1), 46, 44, width - 52, 26, 12, Muted);
    }
    else
        Text(g, t, 6, 0, width - 12, height,
             kind == 1   ? 14.f
             : kind == 5 ? 16.f * std::max(1.f, height / 44.f)
                         : 16.f,
             disabled ? 0xff98a4ac : fg, kind == 0, 1);
    if (d->itemState & ODS_FOCUS)
    {
        Gdiplus::Pen p(C(kind == 0 ? 0xffb5e6e0 : Teal), 1);
        p.SetDashStyle(Gdiplus::DashStyleDash);
        g.DrawRectangle(&p, 4.f, 4.f, width - 8, height - 8);
    }
}
inline void Move(HWND w, HWND parent, int x, int y, int width, int height)
{
    MoveWindow(w, Px(parent, static_cast<float>(x)), Px(parent, static_cast<float>(y)),
               Px(parent, static_cast<float>(width)), Px(parent, static_cast<float>(height)), TRUE);
}
} // namespace Ui
