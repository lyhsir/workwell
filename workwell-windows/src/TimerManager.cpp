#include "TimerManager.h"
#include "ConfigManager.h"
TimerManager::TimerManager(std::function<ULONGLONG()> clock, std::wstring statsPath)
    : m_statsPath(statsPath), m_clock(clock ? clock : []() { return GetTickCount64(); })
{
    RefreshDay();
}
void TimerManager::SetSettings(const Settings &value)
{
    m_settings = value;
    m_settings.workDuration = std::clamp(value.workDuration, 60, 7200);
    m_settings.shortBreakDuration = std::clamp(value.shortBreakDuration, 60, 1800);
    m_settings.longBreakDuration = std::clamp(value.longBreakDuration, 60, 3600);
    m_settings.pomodorosUntilLongBreak = std::clamp(value.pomodorosUntilLongBreak, 2, 10);
    if (IsStopped())
        m_remaining = m_total = m_settings.workDuration;
    Notify();
}
void TimerManager::Notify()
{
    if (m_onTick)
        m_onTick(m_remaining, m_state == TimerState::Working);
}
void TimerManager::StartWork(int seconds)
{
    if (!IsStopped())
        return;
    m_active = m_settings;
    m_deferred = false;
    m_remaining = m_total = std::max(1, seconds);
    m_state = TimerState::Working;
    m_warned = false;
    m_lastTick = m_clock();
    Notify();
}
int TimerManager::NextBreakSeconds() const
{
    const auto &s = ActiveSettings();
    int next = m_completed + (!m_deferred && (m_state == TimerState::Working || IsPaused()) ? 1 : 0);
    return next > 0 && next % s.pomodorosUntilLongBreak == 0 ? s.longBreakDuration : s.shortBreakDuration;
}
void TimerManager::StartBreak()
{
    if (IsInBreak())
        return;
    if (IsStopped())
        m_active = m_settings;
    m_remaining = m_total = m_completed > 0 && m_completed % m_active.pomodorosUntilLongBreak == 0
                                ? m_active.longBreakDuration
                                : m_active.shortBreakDuration;
    m_state = TimerState::Break;
    m_lastTick = m_clock();
    Notify();
    if (m_onBreakStart)
        m_onBreakStart();
}
void TimerManager::Pause()
{
    if (m_state != TimerState::Working)
        return;
    OnTick();
    if (m_state == TimerState::Working)
    {
        m_state = TimerState::Paused;
        Notify();
    }
}
void TimerManager::Resume()
{
    if (!IsPaused())
        return;
    m_state = TimerState::Working;
    m_lastTick = m_clock();
    Notify();
}
void TimerManager::Stop()
{
    if (IsInBreak())
        return;
    m_state = TimerState::Stopped;
    m_deferred = false;
    m_remaining = m_total = m_settings.workDuration;
    Notify();
}
void TimerManager::OnTick()
{
    RefreshDay();
    if (IsStopped() || IsPaused())
        return;
    ULONGLONG now = m_clock();
    int elapsed = static_cast<int>((now - m_lastTick) / 1000);
    if (elapsed < 1)
        return;
    m_lastTick += static_cast<ULONGLONG>(elapsed) * 1000;
    m_remaining = std::max(0, m_remaining - elapsed);
    if (m_state == TimerState::Working && !m_warned && m_remaining <= 30 && m_remaining > 0)
    {
        m_warned = true;
        if (m_onWarning)
            m_onWarning();
    }
    Notify();
    if (m_remaining != 0)
        return;
    if (IsInBreak())
    {
        EndBreak();
        return;
    }
    if (!m_deferred)
    {
        ++m_completed;
        ++m_todayCount;
        m_todaySeconds += m_total;
        SaveStats();
    }
    m_deferred = false;
    StartBreak();
}
void TimerManager::EndBreak()
{
    if (!IsInBreak())
        return;
    m_state = TimerState::Stopped;
    m_deferred = false;
    m_remaining = m_total = m_settings.workDuration;
    Notify();
    if (m_onBreakEnd)
        m_onBreakEnd();
}
void TimerManager::RefreshDay()
{
    SYSTEMTIME t{};
    GetLocalTime(&t);
    wchar_t day[20];
    swprintf_s(day, L"%04u-%02u-%02u", t.wYear, t.wMonth, t.wDay);
    if (m_day == day)
        return;
    m_day = day;
    auto path = m_statsPath.empty() ? ConfigManager::GetDataPath(L"statistics.ini") : m_statsPath;
    m_todayCount = std::max(0, static_cast<int>(GetPrivateProfileIntW(day, L"count", 0, path.c_str())));
    m_todaySeconds = std::max(0, static_cast<int>(GetPrivateProfileIntW(day, L"seconds", 0, path.c_str())));
}
void TimerManager::SaveStats()
{
    auto path = m_statsPath.empty() ? ConfigManager::GetDataPath(L"statistics.ini") : m_statsPath;
    WritePrivateProfileStringW(m_day.c_str(), L"count", std::to_wstring(m_todayCount).c_str(), path.c_str());
    WritePrivateProfileStringW(m_day.c_str(), L"seconds", std::to_wstring(m_todaySeconds).c_str(),
                               path.c_str());
}
int TimerManager::TodayCount()
{
    RefreshDay();
    return m_todayCount;
}
int TimerManager::TodayMinutes()
{
    RefreshDay();
    return m_todaySeconds / 60;
}

void TimerManager::DelayBreak()
{
    if (!IsInBreak() || !m_active.canSkipBreak)
        return;
    m_state = TimerState::Working;
    m_remaining = m_total = 300;
    m_deferred = true;
    m_warned = false;
    m_lastTick = m_clock();
    Notify();
    if (m_onBreakDeferred)
        m_onBreakDeferred();
}
