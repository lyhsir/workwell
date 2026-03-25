#include "TrayIcon.h"
#include "MainDialog.h"
#include <strsafe.h>
#include <shellapi.h>
#include <iostream> // 调试用，可根据项目移除

#define WM_TRAYICON (WM_USER + 1)

static MainDialog* g_mainDialog = nullptr;
static UINT g_trayIconID = 1;

// 调试宏（方便定位问题）
#define LOG_DEBUG(msg) \
    do { \
        wchar_t buf[256]; \
        StringCchPrintfW(buf, ARRAYSIZE(buf), L"[TrayIcon] %s (Line:%d)\n", msg, __LINE__); \
        OutputDebugStringW(buf); \
    } while(0)

void TrayIcon::SetMainDialog(MainDialog* dialog)
{
    g_mainDialog = dialog;
    LOG_DEBUG(L"MainDialog set");
}

TrayIcon::TrayIcon(HINSTANCE hInstance, HWND hwnd, TimerManager* timerManager)
    : m_hInstance(hInstance), m_hwnd(hwnd), m_timerManager(timerManager), m_inBreak(false)
{
    ZeroMemory(&m_nid, sizeof(m_nid));

    // 关键：先验证窗口句柄有效性
    if (m_hwnd == NULL)
    {
        LOG_DEBUG(L"Error: HWND is NULL!");
        return;
    }

    BOOL isWindow = IsWindow(m_hwnd);
    LOG_DEBUG(isWindow ? L"HWND is valid" : L"Error: HWND is invalid!");

    if (AddIcon())
        LOG_DEBUG(L"Tray icon added successfully");
    else
        LOG_DEBUG(L"Error: Failed to add tray icon!");
}

TrayIcon::~TrayIcon()
{
    RemoveIcon();
    LOG_DEBUG(L"Tray icon removed");
}

bool TrayIcon::AddIcon()
{
    // 强制使用兼容所有Windows版本的结构体大小（避免版本问题）
    m_nid.cbSize = NOTIFYICONDATA_V2_SIZE; // 替代sizeof，兼容XP/Vista/Win7+
    m_nid.hWnd = m_hwnd;
    m_nid.uID = g_trayIconID;
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.uCallbackMessage = WM_TRAYICON; // 必须确保这个消息ID唯一且未被占用

    // 改用应用程序图标（避免系统图标加载失败）
    m_nid.hIcon = LoadIcon(m_hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    if (!m_nid.hIcon) // 兜底：用系统默认图标
    {
        m_nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        LOG_DEBUG(L"Use system default icon");
    }

    StringCchCopyW(m_nid.szTip, ARRAYSIZE(m_nid.szTip), L"WorkWell");

    // 第一步：添加托盘图标
    BOOL retAdd = Shell_NotifyIconW(NIM_ADD, &m_nid);
    if (!retAdd)
    {
        DWORD err = GetLastError();
        wchar_t errBuf[256];
        StringCchPrintfW(errBuf, ARRAYSIZE(errBuf), L"Shell_NotifyIcon NIM_ADD failed, Error: %d", err);
        LOG_DEBUG(errBuf);
        return false;
    }

    // 第二步：强制设置版本（关键！否则Win7+可能不响应消息）
    m_nid.uVersion = NOTIFYICON_VERSION; // 兼容模式，比4更稳定
    BOOL retVer = Shell_NotifyIconW(NIM_SETVERSION, &m_nid);
    if (!retVer)
    {
        LOG_DEBUG(L"Warning: NIM_SETVERSION failed (compatible mode)");
    }

    return true;
}

bool TrayIcon::RemoveIcon()
{
    m_nid.uFlags = 0;
    BOOL ret = Shell_NotifyIconW(NIM_DELETE, &m_nid);
    LOG_DEBUG(ret ? L"Tray icon deleted" : L"Error: Failed to delete tray icon");
    return ret;
}

void TrayIcon::UpdateTooltip(const wchar_t* text)
{
    if (!text) return;
    StringCchCopyW(m_nid.szTip, ARRAYSIZE(m_nid.szTip), text);
    m_nid.uFlags = NIF_TIP;
    Shell_NotifyIconW(NIM_MODIFY, &m_nid);
}

void TrayIcon::UpdateTimer(int seconds, bool isActive)
{
    if (!isActive || m_inBreak)
    {
        m_nid.hIcon = LoadIcon(NULL, m_inBreak ? IDI_INFORMATION : IDI_APPLICATION);
        m_nid.uFlags = NIF_ICON;
        Shell_NotifyIconW(NIM_MODIFY, &m_nid);

        if (!m_inBreak)
            UpdateTooltip(L"WorkWell");
    }
    else
    {
        int minutes = seconds / 60;
        int secs = seconds % 60;
        wchar_t buffer[256];
        StringCchPrintfW(buffer, ARRAYSIZE(buffer), L"%02d:%02d", minutes, secs);
        UpdateTooltip(buffer);
    }
}

void TrayIcon::SetBreakState(bool inBreak)
{
    m_inBreak = inBreak;
    m_nid.hIcon = LoadIcon(NULL, inBreak ? IDI_INFORMATION : IDI_APPLICATION);
    UpdateTooltip(inBreak ? L"Resting" : L"WorkWell");
    m_nid.uFlags = NIF_ICON | NIF_TIP;
    Shell_NotifyIconW(NIM_MODIFY, &m_nid);
}

void TrayIcon::ShowWarning()
{
    m_nid.uFlags = NIF_INFO;
    m_nid.dwInfoFlags = NIIF_INFO;
    StringCchCopyW(m_nid.szInfoTitle, ARRAYSIZE(m_nid.szInfoTitle), L"Break");
    StringCchCopyW(m_nid.szInfo, ARRAYSIZE(m_nid.szInfo), L"Time to rest!");
    m_nid.uTimeout = 5000;
    Shell_NotifyIconW(NIM_MODIFY, &m_nid);
}

void TrayIcon::ShowPopupMenu()
{
    LOG_DEBUG(L"ShowPopupMenu called");
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();
    if (!hMenu)
    {
        LOG_DEBUG(L"Error: CreatePopupMenu failed");
        return;
    }

    AppendMenuW(hMenu, MF_STRING, 1001, L"Pause/Resume");
    AppendMenuW(hMenu, MF_STRING, 1002, L"Break Now");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenu, MF_STRING, 1003, L"Exit");

    // 修复Windows 10/11菜单不显示的问题
    SetForegroundWindow(m_hwnd);
    UINT cmd = TrackPopupMenu(
        hMenu,
        TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_NOANIMATION,
        pt.x,
        pt.y,
        0,
        m_hwnd,
        NULL
    );

    LOG_DEBUG(cmd > 0 ? L"Menu command selected" : L"No menu command selected");

    switch (cmd)
    {
    case 1001:
        if (m_timerManager)
            m_timerManager->IsPaused() ? m_timerManager->Resume() : m_timerManager->Pause();
        break;
    case 1002:
        if (m_timerManager) m_timerManager->StartBreak();
        break;
    case 1003:
        PostMessage(m_hwnd, WM_CLOSE, 0, 0);
        break;
    }

    PostMessage(m_hwnd, WM_NULL, 0, 0); // 必须加这行，否则菜单会卡死
    DestroyMenu(hMenu);
}

LRESULT TrayIcon::HandleTrayMessage(WPARAM wParam, LPARAM lParam)
{
    // 打印接收到的消息（调试关键）
    wchar_t msgBuf[256];
    StringCchPrintfW(msgBuf, ARRAYSIZE(msgBuf),
        L"HandleTrayMessage: wParam=%u, lParam=0x%X (msg=%u)",
        wParam, lParam, LOWORD(lParam));
    LOG_DEBUG(msgBuf);

    // 仅处理当前托盘图标的消息
    if (wParam != g_trayIconID)
    {
        LOG_DEBUG(L"Skip: wParam is not tray icon ID");
        return 0;
    }

    UINT msg = LOWORD(lParam);
    switch (msg)
    {
    case WM_LBUTTONDOWN:
        LOG_DEBUG(L"WM_LBUTTONDOWN detected");
        if (g_mainDialog)
            g_mainDialog->IsVisible() ? g_mainDialog->Hide() : g_mainDialog->Show();
        break;

    case WM_LBUTTONDBLCLK:
        LOG_DEBUG(L"WM_LBUTTONDBLCLK detected");
        if (m_timerManager)
            m_timerManager->IsPaused() ? m_timerManager->Resume() : m_timerManager->Pause();
        break;

    case WM_RBUTTONUP:
        LOG_DEBUG(L"WM_RBUTTONUP detected");
        ShowPopupMenu();
        break;

    case WM_CONTEXTMENU:
        LOG_DEBUG(L"WM_CONTEXTMENU detected");
        ShowPopupMenu();
        break;

    default:
        StringCchPrintfW(msgBuf, ARRAYSIZE(msgBuf), L"Unhandled tray msg: %u", msg);
        LOG_DEBUG(msgBuf);
        break;
    }

    return 1; // 返回非0表示消息已处理
}