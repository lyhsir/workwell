#define _WIN32_WINNT 0x0601
#define _WIN32_IE 0x0601

#include "SettingsDialog.h"
#include <strsafe.h>

static SettingsDialog* g_settingsDialogPtr = nullptr;

SettingsDialog::SettingsDialog(HINSTANCE hInstance, HWND parentHWND, const Settings& initialSettings)
    : m_hInstance(hInstance), m_parentHWND(parentHWND), m_hwnd(NULL),
      m_settings(initialSettings), m_workDurationEdit(NULL), m_shortBreakEdit(NULL),
      m_longBreakEdit(NULL), m_pomodorosEdit(NULL), m_canSkipBreakCheck(NULL)
{
    RegisterWindowClass();
    CreateDialogWindow();
}

SettingsDialog::~SettingsDialog()
{
    if (m_hwnd)
    {
        DestroyWindow(m_hwnd);
    }
}

bool SettingsDialog::RegisterWindowClass()
{
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = m_hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(250, 250, 255));
    wc.lpszClassName = L"WorkWellSettingsDialog";

    return RegisterClassExW(&wc) != 0;
}

bool SettingsDialog::CreateDialogWindow()
{
    g_settingsDialogPtr = this;

    m_hwnd = ::CreateWindowExW(
        WS_EX_OVERLAPPEDWINDOW,
        L"WorkWellSettingsDialog",
        L"WorkWell Settings",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT,
        420, 450,
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

    // 计算居中位置
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int windowWidth = 420;
    int windowHeight = 450;
    int x = (screenWidth - windowWidth) / 2;
    int y = (screenHeight - windowHeight) / 2;
    SetWindowPos(m_hwnd, NULL, x, y, windowWidth, windowHeight, SWP_HIDEWINDOW);

    InitControls();

    return true;
}

void SettingsDialog::InitControls()
{
    // 标题字体
    HFONT hTitleFont = CreateFontW(
        18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
    );

    HFONT hLabelFont = CreateFontW(
        14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
    );

    HFONT hButtonFont = CreateFontW(
        14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
    );

    int labelX = 30;
    int editX = 180;
    int controlWidth = 200;
    int editHeight = 26;
    int startY = 20;
    int spacing = 55;

    // 工作时长
    HWND workLabel = ::CreateWindowExW(
        0, L"STATIC", L"工作时长（分钟）:",
        WS_CHILD | WS_VISIBLE,
        labelX, startY, controlWidth, 20,
        m_hwnd, NULL, m_hInstance, NULL
    );
    SendMessage(workLabel, WM_SETFONT, (WPARAM)hLabelFont, TRUE);

    m_workDurationEdit = ::CreateWindowExW(
        WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_AUTOHSCROLL,
        editX, startY - 2, 80, editHeight,
        m_hwnd, (HMENU)1001, m_hInstance, NULL
    );
    SendMessage(m_workDurationEdit, WM_SETFONT, (WPARAM)hLabelFont, TRUE);

    wchar_t workText[16];
    StringCchPrintfW(workText, ARRAYSIZE(workText), L"%d", m_settings.workDuration);
    SetWindowTextW(m_workDurationEdit, workText);

    // 短休息时长
    HWND shortBreakLabel = ::CreateWindowExW(
        0, L"STATIC", L"短休息时长（分钟）:",
        WS_CHILD | WS_VISIBLE,
        labelX, startY + spacing, controlWidth, 20,
        m_hwnd, NULL, m_hInstance, NULL
    );
    SendMessage(shortBreakLabel, WM_SETFONT, (WPARAM)hLabelFont, TRUE);

    m_shortBreakEdit = ::CreateWindowExW(
        WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_AUTOHSCROLL,
        editX, startY + spacing - 2, 80, editHeight,
        m_hwnd, (HMENU)1002, m_hInstance, NULL
    );
    SendMessage(m_shortBreakEdit, WM_SETFONT, (WPARAM)hLabelFont, TRUE);

    wchar_t shortBreakText[16];
    StringCchPrintfW(shortBreakText, ARRAYSIZE(shortBreakText), L"%d", m_settings.shortBreakDuration);
    SetWindowTextW(m_shortBreakEdit, shortBreakText);

    // 长休息时长
    HWND longBreakLabel = ::CreateWindowExW(
        0, L"STATIC", L"长休息时长（分钟）:",
        WS_CHILD | WS_VISIBLE,
        labelX, startY + spacing * 2, controlWidth, 20,
        m_hwnd, NULL, m_hInstance, NULL
    );
    SendMessage(longBreakLabel, WM_SETFONT, (WPARAM)hLabelFont, TRUE);

    m_longBreakEdit = ::CreateWindowExW(
        WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_AUTOHSCROLL,
        editX, startY + spacing * 2 - 2, 80, editHeight,
        m_hwnd, (HMENU)1003, m_hInstance, NULL
    );
    SendMessage(m_longBreakEdit, WM_SETFONT, (WPARAM)hLabelFont, TRUE);

    wchar_t longBreakText[16];
    StringCchPrintfW(longBreakText, ARRAYSIZE(longBreakText), L"%d", m_settings.longBreakDuration);
    SetWindowTextW(m_longBreakEdit, longBreakText);

    // 长休息间隔
    HWND pomodorosLabel = ::CreateWindowExW(
        0, L"STATIC", L"长休息间隔（番茄数）:",
        WS_CHILD | WS_VISIBLE,
        labelX, startY + spacing * 3, controlWidth, 20,
        m_hwnd, NULL, m_hInstance, NULL
    );
    SendMessage(pomodorosLabel, WM_SETFONT, (WPARAM)hLabelFont, TRUE);

    m_pomodorosEdit = ::CreateWindowExW(
        WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_AUTOHSCROLL,
        editX, startY + spacing * 3 - 2, 80, editHeight,
        m_hwnd, (HMENU)1004, m_hInstance, NULL
    );
    SendMessage(m_pomodorosEdit, WM_SETFONT, (WPARAM)hLabelFont, TRUE);

    wchar_t pomodorosText[16];
    StringCchPrintfW(pomodorosText, ARRAYSIZE(pomodorosText), L"%d", m_settings.pomodorosUntilLongBreak);
    SetWindowTextW(m_pomodorosEdit, pomodorosText);

    // 是否可以跳过休息
    m_canSkipBreakCheck = ::CreateWindowExW(
        0, L"BUTTON", L"允许跳过休息",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        labelX, startY + spacing * 4, 200, 24,
        m_hwnd, (HMENU)1005, m_hInstance, NULL
    );
    SendMessage(m_canSkipBreakCheck, WM_SETFONT, (WPARAM)hLabelFont, TRUE);
    SendMessage(m_canSkipBreakCheck, BM_SETCHECK, m_settings.canSkipBreak ? BST_CHECKED : BST_UNCHECKED, 0);

    // 按钮
    int buttonY = startY + spacing * 5 + 10;
    int buttonWidth = 100;
    int buttonHeight = 36;
    int buttonSpacing = 15;
    int totalButtonWidth = buttonWidth * 2 + buttonSpacing;
    int buttonStartX = (420 - totalButtonWidth) / 2;

    HWND saveButton = ::CreateWindowExW(
        0, L"BUTTON", L"保存",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        buttonStartX, buttonY, buttonWidth, buttonHeight,
        m_hwnd, (HMENU)2001, m_hInstance, NULL
    );
    SendMessage(saveButton, WM_SETFONT, (WPARAM)hButtonFont, TRUE);

    HWND cancelButton = ::CreateWindowExW(
        0, L"BUTTON", L"取消",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        buttonStartX + buttonWidth + buttonSpacing, buttonY, buttonWidth, buttonHeight,
        m_hwnd, (HMENU)2002, m_hInstance, NULL
    );
    SendMessage(cancelButton, WM_SETFONT, (WPARAM)hButtonFont, TRUE);
}

void SettingsDialog::Show()
{
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int windowWidth = 420;
    int windowHeight = 450;
    int x = (screenWidth - windowWidth) / 2;
    int y = (screenHeight - windowHeight) / 2;
    SetWindowPos(m_hwnd, HWND_TOPMOST, x, y, windowWidth, windowHeight, SWP_SHOWWINDOW);

    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);
}
void SettingsDialog::Hide()
{
    ShowWindow(m_hwnd, SW_HIDE);
}

void SettingsDialog::ApplySettings()
{
    wchar_t buffer[32];

    // 读取工作时长
    GetWindowTextW(m_workDurationEdit, buffer, ARRAYSIZE(buffer));
    int workDuration = _wtoi(buffer);
    if (workDuration < 1) workDuration = 1;
    if (workDuration > 120) workDuration = 120;
    m_settings.workDuration = workDuration;

    // 读取短休息时长
    GetWindowTextW(m_shortBreakEdit, buffer, ARRAYSIZE(buffer));
    int shortBreak = _wtoi(buffer);
    if (shortBreak < 1) shortBreak = 1;
    if (shortBreak > 30) shortBreak = 30;
    m_settings.shortBreakDuration = shortBreak;

    // 读取长休息时长
    GetWindowTextW(m_longBreakEdit, buffer, ARRAYSIZE(buffer));
    int longBreak = _wtoi(buffer);
    if (longBreak < 1) longBreak = 1;
    if (longBreak > 60) longBreak = 60;
    m_settings.longBreakDuration = longBreak;

    // 读取长休息间隔
    GetWindowTextW(m_pomodorosEdit, buffer, ARRAYSIZE(buffer));
    int pomodoros = _wtoi(buffer);
    if (pomodoros < 2) pomodoros = 2;
    if (pomodoros > 10) pomodoros = 10;
    m_settings.pomodorosUntilLongBreak = pomodoros;

    // 读取是否可以跳过休息
    m_settings.canSkipBreak = (SendMessage(m_canSkipBreakCheck, BM_GETCHECK, 0, 0) == BST_CHECKED);

    // 通知设置已更改
    if (m_onSettingsChanged)
    {
        m_onSettingsChanged(m_settings);
    }
}

LRESULT CALLBACK SettingsDialog::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_CREATE)
    {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        g_settingsDialogPtr = reinterpret_cast<SettingsDialog*>(pCreate->lpCreateParams);
        return 0;
    }

    if (!g_settingsDialogPtr)
    {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    switch (uMsg)
    {
    case WM_CTLCOLORSTATIC:
    {
        HDC hdcStatic = (HDC)wParam;
        SetBkMode(hdcStatic, TRANSPARENT);
        SetTextColor(hdcStatic, RGB(33, 33, 33));
        return (LRESULT)GetStockObject(NULL_BRUSH);
    }

    case WM_COMMAND:
    {
        WORD cmd = LOWORD(wParam);

        switch (cmd)
        {
        case 2001:  // 保存按钮
            g_settingsDialogPtr->ApplySettings();
            g_settingsDialogPtr->Hide();
            return 0;

        case 2002:  // 取消按钮
            g_settingsDialogPtr->Hide();
            return 0;
        }
        break;
    }

    case WM_CLOSE:
        ShowWindow(hwnd, SW_HIDE);
        return 0;

    case WM_DESTROY:
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
