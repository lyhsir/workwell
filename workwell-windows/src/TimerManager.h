#pragma once

#include <functional>
#include <windows.h>

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
        int workDuration;        // 工作时长（秒）
        int shortBreakDuration;  // 短休息时长（秒）
        int longBreakDuration;   // 长休息时长（秒）
        int pomodorosUntilLongBreak;  // 多少个番茄钟后长休息
        bool canSkipBreak;       // 是否可以跳过休息

        Settings()
            : workDuration(25 * 60)
            , shortBreakDuration(5 * 60)
            , longBreakDuration(15 * 60)
            , pomodorosUntilLongBreak(4)
            , canSkipBreak(false)
        {}
    };

    using OnTickCallback = std::function<void(int seconds, bool isActive)>;
    using OnEventCallback = std::function<void()>;

    TimerManager();
    ~TimerManager();

    void StartWork(int seconds);
    void StartBreak();
    void Pause();
    void Resume();
    void Stop();
    void OnTick();
    void EndBreak();  // 结束休息，不自动开始新工作
    bool IsPaused() const { return m_state == TimerState::Paused; }
    bool IsStopped() const { return m_state == TimerState::Stopped; }
    bool IsInBreak() const { return m_state == TimerState::Break; }

    void SetSettings(const Settings& settings) { m_settings = settings; }
    const Settings& GetSettings() const { return m_settings; }
    int GetCompletedPomodoros() const { return m_completedPomodoros; }

    void SetOnTick(OnTickCallback callback) { m_onTick = callback; }
    void SetOnBreakStart(OnEventCallback callback) { m_onBreakStart = callback; }
    void SetOnBreakEnd(OnEventCallback callback) { m_onBreakEnd = callback; }
    void SetOnWarning(OnEventCallback callback) { m_onWarning = callback; }

private:
    TimerState m_state;
    int m_remainingSeconds;
    int m_totalSeconds;
    bool m_warningShown;
    Settings m_settings;
    int m_completedPomodoros;  // 已完成的番茄钟数

    OnTickCallback m_onTick;
    OnEventCallback m_onBreakStart;
    OnEventCallback m_onBreakEnd;
    OnEventCallback m_onWarning;
};
