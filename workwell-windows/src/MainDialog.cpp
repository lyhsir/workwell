#define _WIN32_WINNT 0x0601
#define _WIN32_IE 0x0601

#include "MainDialog.h"
#include "TimerManager.h"
#include "ConfigManager.h"
#include "SettingsDialog.h"
#include <strsafe.h>
#include <cmath>

static MainDialog* g_dialogPtr = nullptr;
static int g_currentSeconds = 25 * 60;
static int g_totalSeconds = 25 * 60;  // 总时间，用于计算进度
static bool g_isRunning = false;
static bool g_isPaused = false;

MainDialog::MainDialog(HINSTANCE hInstance, TimerManager* timerManager)
    : m_hInstance(hInstance), m_timerManager(timerManager), m_hwnd(NULL),
      m_stateLabel(NULL), m_countdownLabel(NULL), m_pauseButton(NULL),
      m_stopButton(NULL), m_settingsButton(NULL), m_statsLabel(NULL), m_isVisible(false)
{
    RegisterWindowClass();
    CreateMainWindow();

    // 创建设置对话框
    SettingsDialog::Settings initialSettings;
    initialSettings.workDuration = timerManager->GetSettings().workDuration / 60;
    initialSettings.shortBreakDuration = timerManager->GetSettings().shortBreakDuration / 60;
    initialSettings.longBreakDuration = timerManager->GetSettings().longBreakDuration / 60;
    initialSettings.pomodorosUntilLongBreak = timerManager->GetSettings().pomodorosUntilLongBreak;
    initialSettings.canSkipBreak = timerManager->GetSettings().canSkipBreak;

    m_settingsDialog = std::make_unique<SettingsDialog>(hInstance, m_hwnd, initialSettings);
    m_settingsDialog->SetOnSettingsChanged([this](const SettingsDialog::Settings& settings) {
        OnSettingsChanged(settings.workDuration, settings.shortBreakDuration, settings.longBreakDuration, settings.pomodorosUntilLongBreak, settings.canSkipBreak);
    });

    // 初始化当前时间
    g_totalSeconds = timerManager->GetSettings().workDuration;
    g_currentSeconds = g_totalSeconds;
}

MainDialog::~MainDialog()
{
}

bool MainDialog::RegisterWindowClass()
{
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = m_hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(250, 250, 255));
    wc.lpszClassName = L"WorkWellMainDialog";

    return RegisterClassExW(&wc) != 0;
}

bool MainDialog::CreateMainWindow()
{
    g_dialogPtr = this;

    // 固定大小窗口，不可调整大小
    m_hwnd = ::CreateWindowExW(
        WS_EX_OVERLAPPEDWINDOW,
        L"WorkWellMainDialog",
        L"WorkWell",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT,
        420, 540,
        NULL, NULL, m_hInstance, this
    );

    if (!m_hwnd)
    {
        return false;
    }

    // 禁止调整窗口大小
    LONG_PTR style = GetWindowLongPtr(m_hwnd, GWL_STYLE);
    style &= ~WS_SIZEBOX;
    SetWindowLongPtr(m_hwnd, GWL_STYLE, style);

    // 设置窗口大小
    SetWindowPos(m_hwnd, NULL, 0, 0, 420, 540, SWP_NOZORDER | SWP_NOMOVE);

    // 计算居中位置
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int windowWidth = 420;
    int windowHeight = 540;
    int x = (screenWidth - windowWidth) / 2;
    int y = (screenHeight - windowHeight) / 2;
    SetWindowPos(m_hwnd, HWND_TOPMOST, x, y, windowWidth, windowHeight, SWP_HIDEWINDOW);  // 初始隐藏

    // 状态指示器 - 顶部显示
    m_stateLabel = ::CreateWindowExW(
        0, L"STATIC", L"● 准备开始",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        0, 10, 420, 30,
        m_hwnd, NULL, m_hInstance, NULL
    );

    // 倒计时区域 - 仅用于接收消息，实际绘制在整个窗口上进行
    m_countdownLabel = ::CreateWindowExW(
        0, L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | SS_CENTER | SS_NOTIFY,
        0, 0, 0, 0,  // 不占用实际空间
        m_hwnd, (HMENU)1001, m_hInstance, NULL
    );

    // 按钮区域 - 三个按钮并排
    int buttonWidth = 110;
    int buttonHeight = 38;
    int buttonSpacing = 12;
    int totalButtonWidth = buttonWidth * 3 + buttonSpacing * 2;
    int startX = (420 - totalButtonWidth) / 2;

    m_pauseButton = ::CreateWindowExW(
        0, L"BUTTON", L"Start Work",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        startX, 390, buttonWidth, buttonHeight,
        m_hwnd, (HMENU)1002, m_hInstance, NULL
    );

    m_stopButton = ::CreateWindowExW(
        0, L"BUTTON", L"Stop",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        startX + buttonWidth + buttonSpacing, 390, buttonWidth, buttonHeight,
        m_hwnd, (HMENU)1003, m_hInstance, NULL
    );

    m_settingsButton = ::CreateWindowExW(
        0, L"BUTTON", L"Settings",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        startX + buttonWidth * 2 + buttonSpacing * 2, 390, buttonWidth, buttonHeight,
        m_hwnd, (HMENU)1004, m_hInstance, NULL
    );

    // 统计文本 - 居中显示在按钮下方
    m_statsLabel = ::CreateWindowExW(
        0, L"STATIC", L"Today: 0 pomodoros",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        60, 445, 300, 40,
        m_hwnd, NULL, m_hInstance, NULL
    );

    // 字体设置
    HFONT hFont = CreateFontW(
        20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
    );

    HFONT hStateFont = CreateFontW(
        18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
    );

    HFONT hSmallFont = CreateFontW(
        16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
    );

    if (hStateFont)
    {
        SendMessage(m_stateLabel, WM_SETFONT, (WPARAM)hStateFont, TRUE);
    }

    if (hFont)
    {
        SendMessage(m_pauseButton, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(m_stopButton, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(m_settingsButton, WM_SETFONT, (WPARAM)hFont, TRUE);
    }

    if (hSmallFont)
    {
        SendMessage(m_statsLabel, WM_SETFONT, (WPARAM)hSmallFont, TRUE);
    }

    return true;
}

void MainDialog::Show()
{
    m_isVisible = true;

    // 每次显示时重新居中窗口
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int windowWidth = 420;
    int windowHeight = 540;
    int x = (screenWidth - windowWidth) / 2;
    int y = (screenHeight - windowHeight) / 2;
    SetWindowPos(m_hwnd, HWND_TOPMOST, x, y, windowWidth, windowHeight, SWP_SHOWWINDOW);

    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);
}

void MainDialog::Hide()
{
    m_isVisible = false;
    ShowWindow(m_hwnd, SW_HIDE);
}
void MainDialog::UpdateCountdown(int seconds)
{
    g_currentSeconds = seconds;

    // 更新总时间（用于计算进度）
    if (!g_isRunning && !g_isPaused)
    {
        g_totalSeconds = m_timerManager->GetSettings().workDuration;
    }

    UpdateButtons();
    UpdateState();

    // 强制重绘
    if (m_hwnd && m_isVisible)
    {
        InvalidateRect(m_hwnd, NULL, TRUE);
        UpdateWindow(m_hwnd);
    }
}

void MainDialog::UpdateButtonState(bool isRunning, bool isPaused)
{
    g_isRunning = isRunning;
    g_isPaused = isPaused;
    UpdateButtons();
    UpdateState();
}

void MainDialog::UpdateStats()
{
    if (!m_statsLabel) return;

    wchar_t buffer[256];
    StringCchPrintfW(buffer, ARRAYSIZE(buffer), L"Today: %d pomodoros", 0);
    SetWindowTextW(m_statsLabel, buffer);
}

void MainDialog::UpdateButtons()
{
    if (!m_pauseButton || !m_stopButton) return;

    if (g_isRunning)
    {
        if (g_isPaused)
        {
            SetWindowTextW(m_pauseButton, L"Resume");
            EnableWindow(m_stopButton, TRUE);
        }
        else
        {
            SetWindowTextW(m_pauseButton, L"Pause");
            EnableWindow(m_stopButton, TRUE);
        }
    }
    else
    {
        SetWindowTextW(m_pauseButton, L"Start Work");
        EnableWindow(m_stopButton, FALSE);
    }
}

void MainDialog::UpdateState()
{
    if (!m_stateLabel) return;

    const wchar_t* stateText = L"● 准备开始";

    if (g_isRunning)
    {
        if (g_isPaused)
        {
            stateText = L"⏸ 已暂停";
        }
        else
        {
            stateText = L"● 工作中";
        }
    }
    else if (g_currentSeconds < g_totalSeconds)
    {
        stateText = L"⏹ 已停止";
    }

    SetWindowTextW(m_stateLabel, stateText);
    InvalidateRect(m_stateLabel, NULL, TRUE);
}

void MainDialog::OnSettingsChanged(int workDuration, int shortBreakDuration, int longBreakDuration, int pomodorosUntilLongBreak, bool canSkipBreak)
{
    // 更新 TimerManager 的设置
    TimerManager::Settings timerSettings;
    timerSettings.workDuration = workDuration * 60;
    timerSettings.shortBreakDuration = shortBreakDuration * 60;
    timerSettings.longBreakDuration = longBreakDuration * 60;
    timerSettings.pomodorosUntilLongBreak = pomodorosUntilLongBreak;
    timerSettings.canSkipBreak = canSkipBreak;
    m_timerManager->SetSettings(timerSettings);

    // 保存配置到文件
    ConfigManager::Save(timerSettings);

    // 如果当前没有运行，更新显示的时间
    if (!g_isRunning && !g_isPaused)
    {
        g_totalSeconds = timerSettings.workDuration;
        g_currentSeconds = g_totalSeconds;
        UpdateCountdown(g_currentSeconds);
    }
}

void MainDialog::DrawCountdown(HDC hdc, int seconds)
{
    RECT rect;
    GetClientRect(m_hwnd, &rect);

    HBRUSH hBgBrush = CreateSolidBrush(RGB(250, 250, 255));
    FillRect(hdc, &rect, hBgBrush);
    DeleteObject(hBgBrush);

    int totalSeconds = g_totalSeconds;
    double progress = (totalSeconds > 0) ? (double)seconds / totalSeconds : 0.0;

    // 在整个窗口中居中，稍微偏上一点以避开按钮区域
    int centerX = rect.left + (rect.right - rect.left) / 2;
    int centerY = 210;  // 固定垂直位置，在状态标签和按钮之间
    int radius = 130;

    // 背景圆圈 - 更粗
    HPEN hPenBg = CreatePen(PS_SOLID, 10, RGB(230, 230, 235));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPenBg);
    Ellipse(hdc, centerX - radius, centerY - radius,
            centerX + radius, centerY + radius);
    DeleteObject(SelectObject(hdc, hOldPen));

    // 进度圆弧 - 更粗更大
    if (seconds > 0 && seconds < totalSeconds)
    {
        HPEN hPenProgress = CreatePen(PS_SOLID, 10, RGB(66, 133, 244));
        SelectObject(hdc, hPenProgress);

        double startAngle = -3.14159265 / 2;
        double endAngle = startAngle + (2.0 * 3.14159265 * progress);

        int startX = centerX + (int)(radius * cos(startAngle));
        int startY = centerY - (int)(radius * sin(startAngle));
        int endX = centerX + (int)(radius * cos(endAngle));
        int endY = centerY - (int)(radius * sin(endAngle));

        Arc(hdc, centerX - radius, centerY - radius,
             centerX + radius, centerY + radius,
             startX, startY, endX, endY);

        DeleteObject(SelectObject(hdc, hOldPen));
    }

    // 主时间显示 - 更大更醒目
    int minutes = seconds / 60;
    int secs = seconds % 60;

    wchar_t timeText[32];
    StringCchPrintfW(timeText, ARRAYSIZE(timeText), L"%02d:%02d", minutes, secs);

    HFONT hFont = CreateFontW(
        52, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
    );

    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(33, 33, 33));

    RECT textRect;
    textRect.left = centerX - 90;
    textRect.top = centerY - 30;
    textRect.right = centerX + 90;
    textRect.bottom = centerY + 30;

    DrawTextW(hdc, timeText, -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    DeleteObject(SelectObject(hdc, hOldFont));

    // 显示剩余时间和进度信息
    if (seconds < totalSeconds && seconds > 0)
    {
        int percentage = (int)(progress * 100);

        HFONT hSmallFont = CreateFontW(
            16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
        );

        wchar_t infoText[64];
        if (g_isRunning && !g_isPaused)
        {
            StringCchPrintfW(infoText, ARRAYSIZE(infoText), L"%d%% · 工作中", percentage);
        }
        else if (g_isPaused)
        {
            StringCchPrintfW(infoText, ARRAYSIZE(infoText), L"%d%% · 已暂停", percentage);
        }
        else
        {
            StringCchPrintfW(infoText, ARRAYSIZE(infoText), L"准备开始");
        }

        SelectObject(hdc, hSmallFont);
        SetTextColor(hdc, RGB(120, 120, 120));

        RECT infoRect;
        infoRect.left = centerX - 110;
        infoRect.top = centerY + 45;
        infoRect.right = centerX + 110;
        infoRect.bottom = centerY + 70;

        DrawTextW(hdc, infoText, -1, &infoRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        DeleteObject(SelectObject(hdc, hSmallFont));
    }
}

LRESULT CALLBACK MainDialog::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_CREATE)
    {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        g_dialogPtr = reinterpret_cast<MainDialog*>(pCreate->lpCreateParams);
        return 0;
    }

    if (!g_dialogPtr)
    {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    switch (uMsg)
    {
    case WM_CTLCOLORSTATIC:
        if ((HWND)lParam == g_dialogPtr->m_stateLabel)
        {
            HDC hdcStatic = (HDC)wParam;
            SetBkMode(hdcStatic, TRANSPARENT);

            // 根据状态设置颜色
            if (g_isRunning && !g_isPaused)
            {
                SetTextColor(hdcStatic, RGB(76, 175, 80));
            }
            else if (g_isPaused)
            {
                SetTextColor(hdcStatic, RGB(255, 152, 0));
            }
            else
            {
                SetTextColor(hdcStatic, RGB(120, 120, 120));
            }

            return (LRESULT)GetStockObject(NULL_BRUSH);
        }
        break;

    case WM_KEYDOWN:
        // 键盘快捷键
        if (GetKeyState(VK_CONTROL) < 0 && GetKeyState(VK_SHIFT) < 0)
        {
            switch (wParam)
            {
            case 'P': // Ctrl+Shift+P: 暂停/继续
                if (g_dialogPtr->m_timerManager && g_isRunning)
                {
                    if (g_isPaused)
                    {
                        g_isPaused = false;
                        g_dialogPtr->m_timerManager->Resume();
                    }
                    else
                    {
                        g_isPaused = true;
                        g_dialogPtr->m_timerManager->Pause();
                    }
                    g_dialogPtr->UpdateButtons();
                    g_dialogPtr->UpdateState();
                }
                return 0;

            case 'S': // Ctrl+Shift+S: 停止
                if (g_dialogPtr->m_timerManager && g_isRunning)
                {
                    g_isRunning = false;
                    g_isPaused = false;
                    g_dialogPtr->m_timerManager->Stop();
                    g_dialogPtr->UpdateCountdown(g_dialogPtr->m_timerManager->GetSettings().workDuration);
                }
                return 0;

            case 'H': // Ctrl+H: 隐藏窗口
                g_dialogPtr->Hide();
                return 0;
            }
        }
        else if (wParam == VK_ESCAPE)
        {
            // ESC: 隐藏窗口
            g_dialogPtr->Hide();
            return 0;
        }
        break;

    case WM_COMMAND:
        {
            WORD cmd = LOWORD(wParam);

            switch (cmd)
            {
            case 1002: // Start/Pause/Resume
                if (g_dialogPtr->m_timerManager)
                {
                    if (!g_isRunning)
                    {
                        g_isRunning = true;
                        g_isPaused = false;
                        int workDuration = g_dialogPtr->m_timerManager->GetSettings().workDuration;
                        g_dialogPtr->m_timerManager->StartWork(workDuration);
                        g_dialogPtr->UpdateButtons();
                        g_dialogPtr->UpdateState();
                    }
                    else if (g_isPaused)
                    {
                        g_isPaused = false;
                        g_dialogPtr->m_timerManager->Resume();
                        g_dialogPtr->UpdateButtons();
                        g_dialogPtr->UpdateState();
                    }
                    else
                    {
                        g_isPaused = true;
                        g_dialogPtr->m_timerManager->Pause();
                        g_dialogPtr->UpdateButtons();
                        g_dialogPtr->UpdateState();
                    }
                }
                break;

            case 1003: // Stop
                if (g_dialogPtr->m_timerManager)
                {
                    g_isRunning = false;
                    g_isPaused = false;
                    g_dialogPtr->m_timerManager->Stop();
                    g_dialogPtr->UpdateCountdown(g_dialogPtr->m_timerManager->GetSettings().workDuration);
                }
                break;

            case 1004: // Settings
                if (g_dialogPtr->m_settingsDialog)
                {
                    g_dialogPtr->m_settingsDialog->Show();
                }
                break;
            }
            return 0;
        }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        g_dialogPtr->DrawCountdown(hdc, g_currentSeconds);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        g_dialogPtr->m_isVisible = false;
        return 0;

    case WM_CLOSE:
        g_dialogPtr->Hide();
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
