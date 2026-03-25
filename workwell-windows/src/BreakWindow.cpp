#define _WIN32_WINNT 0x0601
#define _WIN32_IE 0x0601

#include "BreakWindow.h"
#include "TimerManager.h"
#include <strsafe.h>
#include <sstream>

static BreakWindow* g_breakWindowPtr = nullptr;
static TimerManager* g_timerManagerPtr = nullptr;

BreakWindow::BreakWindow(HINSTANCE hInstance)
    : m_hInstance(hInstance), m_hwnd(NULL), m_countdownLabel(NULL),
      m_durationSeconds(0), m_isVisible(false)
{
    RegisterWindowClass();
    CreateBreakWindow();
}

BreakWindow::~BreakWindow()
{
    if (m_hwnd)
    {
        DestroyWindow(m_hwnd);
    }
}

void BreakWindow::SetTimerManager(TimerManager* timerManager)
{
    g_timerManagerPtr = timerManager;
}

bool BreakWindow::RegisterWindowClass()
{
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = m_hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;  // 我们自己绘制背景
    wc.lpszClassName = L"WorkWellBreakWindow";

    return RegisterClassExW(&wc) != 0;
}

bool BreakWindow::CreateBreakWindow()
{
    g_breakWindowPtr = this;

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    m_hwnd = ::CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"WorkWellBreakWindow",
        L"Break Time",
        WS_POPUP,
        0, 0, screenWidth, screenHeight,
        NULL, NULL, m_hInstance, this
    );

    if (!m_hwnd)
    {
        return false;
    }

    // 倒计时标签（用于显示文字）
    m_countdownLabel = ::CreateWindowExW(
        0, L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        0, 0, screenWidth, screenHeight,
        m_hwnd, NULL, m_hInstance, NULL
    );

    HFONT hFont = CreateFontW(
        96, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
    );

    if (hFont)
    {
        SendMessage(m_countdownLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
    }

    return true;
}

void BreakWindow::Show(int durationSeconds)
{
    m_durationSeconds = durationSeconds;
    m_isVisible = true;

    UpdateCountdown();

    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);

    SetTimer(m_hwnd, 1, 1000, NULL);
}

void BreakWindow::Hide()
{
    m_isVisible = false;
    KillTimer(m_hwnd, 1);
    ShowWindow(m_hwnd, SW_HIDE);
}

void BreakWindow::UpdateCountdown()
{
    if (!m_countdownLabel) return;

    int minutes = m_durationSeconds / 60;
    int seconds = m_durationSeconds % 60;

    wchar_t buffer[256];
    StringCchPrintfW(buffer, ARRAYSIZE(buffer),
        L"休息时间\n\n%02d:%02d\n\n放松一下，喝杯水吧！",
        minutes, seconds);

    SetWindowTextW(m_countdownLabel, buffer);
    InvalidateRect(m_countdownLabel, NULL, TRUE);
}

LRESULT CALLBACK BreakWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_CREATE)
    {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        g_breakWindowPtr = reinterpret_cast<BreakWindow*>(pCreate->lpCreateParams);
        return 0;
    }

    if (!g_breakWindowPtr)
    {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    switch (uMsg)
    {
    case WM_ERASEBKGND:
    {
        HDC hdc = (HDC)wParam;
        RECT rect;
        GetClientRect(hwnd, &rect);

        // 创建更漂亮的渐变背景
        TRIVERTEX vertex[2];
        GRADIENT_RECT gradientRect = {0, 0};

        // 顶部颜色 - 深蓝色
        vertex[0].x = rect.left;
        vertex[0].y = rect.top;
        vertex[0].Red = 20 << 8;
        vertex[0].Green = 30 << 8;
        vertex[0].Blue = 80 << 8;
        vertex[0].Alpha = 255 << 8;

        // 底部颜色 - 深紫色
        vertex[1].x = rect.right;
        vertex[1].y = rect.bottom;
        vertex[1].Red = 40 << 8;
        vertex[1].Green = 20 << 8;
        vertex[1].Blue = 70 << 8;
        vertex[1].Alpha = 255 << 8;

        GradientFill(hdc, vertex, 2, &gradientRect, 1, GRADIENT_FILL_RECT_V);
        return TRUE;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rect;
        GetClientRect(hwnd, &rect);

        // 重新绘制渐变背景
        TRIVERTEX vertex[2];
        GRADIENT_RECT gradientRect = {0, 0};

        vertex[0].x = rect.left;
        vertex[0].y = rect.top;
        vertex[0].Red = 20 << 8;
        vertex[0].Green = 30 << 8;
        vertex[0].Blue = 80 << 8;
        vertex[0].Alpha = 255 << 8;

        vertex[1].x = rect.right;
        vertex[1].y = rect.bottom;
        vertex[1].Red = 40 << 8;
        vertex[1].Green = 20 << 8;
        vertex[1].Blue = 70 << 8;
        vertex[1].Alpha = 255 << 8;

        GradientFill(hdc, vertex, 2, &gradientRect, 1, GRADIENT_FILL_RECT_V);

        // 绘制装饰圆圈 - 更大更醒目
        HPEN hPen = CreatePen(PS_SOLID, 4, RGB(100, 180, 255));
        HGDIOBJ hOldPen = SelectObject(hdc, hPen);

        int centerX = rect.right / 2;
        int centerY = rect.top + 280;
        int radius = 200;

        // 绘制多个同心圆
        for (int i = 0; i < 3; i++)
        {
            int currentRadius = radius - i * 30;
            Ellipse(hdc, centerX - currentRadius, centerY - currentRadius,
                    centerX + currentRadius, centerY + currentRadius);
        }

        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_CTLCOLORSTATIC:
    {
        HDC hdcStatic = (HDC)wParam;
        SetBkMode(hdcStatic, TRANSPARENT);
        SetTextColor(hdcStatic, RGB(255, 255, 255));
        return (LRESULT)GetStockObject(NULL_BRUSH);
    }

    case WM_TIMER:
        if (g_breakWindowPtr->m_durationSeconds > 0)
        {
            g_breakWindowPtr->m_durationSeconds--;
            g_breakWindowPtr->UpdateCountdown();
        }
        else
        {
            KillTimer(hwnd, 1);
            // 休息结束，通知 TimerManager 继续工作
            if (g_timerManagerPtr)
            {
                g_timerManagerPtr->EndBreak();
            }
        }
        return 0;

    case WM_CLOSE:
        return 0;

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
        {
            return 0;
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
