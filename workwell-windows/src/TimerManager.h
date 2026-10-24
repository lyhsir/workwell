#pragma once
#include <windows.h>
#include <functional>
#include <algorithm>
#include <string>
enum class TimerState
{
    Working,
    Break,
    Paused,
    Stopped
};
class TimerManager
{
  public:
    struct Settings
    {
        int workDuration = 1500, shortBreakDuration = 300, longBreakDuration = 900,
            pomodorosUntilLongBreak = 4;
        bool canSkipBreak = true;
    };
    using OnTickCallback = std::function<void(int, bool)>;
    using OnEventCallback = std::function<void()>;
    explicit TimerManager(std::function<ULONGLONG()> clock = {}, std::wstring statsPath = L"");
    void StartWork(int seconds);
    void StartBreak();
    void Pause();
    void Resume();
    void Stop();
    void OnTick();
    void EndBreak();
    void DelayBreak();
    bool IsDeferred() const { return m_deferred; }
    bool IsPaused() const { return m_state == TimerState::Paused; }
    bool IsStopped() const { return m_state == TimerState::Stopped; }
    bool IsInBreak() const { return m_state == TimerState::Break; }
    TimerState GetState() const { return m_state; }
    int Remaining() const { return m_remaining; }
    int Total() const { return m_total; }
    void SetSettings(const Settings &settings);
    const Settings &GetSettings() const { return m_settings; }
    const Settings &ActiveSettings() const { return IsStopped() ? m_settings : m_active; }
    int GetCompletedPomodoros() const { return m_completed; }
    int TodayCount();
    int TodayMinutes();
    int NextBreakSeconds() const;
    void SetOnTick(OnTickCallback cb) { m_onTick = cb; }
    void SetOnBreakStart(OnEventCallback cb) { m_onBreakStart = cb; }
    void SetOnBreakEnd(OnEventCallback cb) { m_onBreakEnd = cb; }
    void SetOnWarning(OnEventCallback cb) { m_onWarning = cb; }
    void SetOnBreakDeferred(OnEventCallback cb) { m_onBreakDeferred = cb; }

  private:
    TimerState m_state = TimerState::Stopped;
    Settings m_settings, m_active;
    int m_remaining = 1500, m_total = 1500, m_completed = 0, m_todayCount = 0, m_todaySeconds = 0;
    std::wstring m_day, m_statsPath;
    std::function<ULONGLONG()> m_clock;
    bool m_deferred = false;
    ULONGLONG m_lastTick = 0;
    bool m_warned = false;
    OnTickCallback m_onTick;
    OnEventCallback m_onBreakStart, m_onBreakEnd, m_onWarning, m_onBreakDeferred;
    void Notify();
    void RefreshDay();
    void SaveStats();
};
