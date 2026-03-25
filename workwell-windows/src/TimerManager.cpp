#include "TimerManager.h"

TimerManager::TimerManager()
    : m_state(TimerState::Stopped), m_remainingSeconds(0), m_totalSeconds(0),
      m_warningShown(false), m_completedPomodoros(0)
{
}

TimerManager::~TimerManager()
{
}

void TimerManager::StartWork(int seconds)
{
    m_state = TimerState::Working;
    m_remainingSeconds = seconds;
    m_totalSeconds = seconds;
    m_warningShown = false;
}

void TimerManager::StartBreak()
{
    m_state = TimerState::Break;

    // 根据已完成的番茄钟数决定休息时长
    bool shouldUseLongBreak = (m_completedPomodoros > 0 && m_completedPomodoros % m_settings.pomodorosUntilLongBreak == 0);

    m_remainingSeconds = shouldUseLongBreak ? m_settings.longBreakDuration : m_settings.shortBreakDuration;
    m_totalSeconds = m_remainingSeconds;
    m_warningShown = false;

    if (m_onBreakStart)
    {
        m_onBreakStart();
    }
}

void TimerManager::Pause()
{
    if (m_state == TimerState::Working)
    {
        m_state = TimerState::Paused;
    }
}

void TimerManager::Resume()
{
    if (m_state == TimerState::Paused)
    {
        m_state = TimerState::Working;
    }
}

void TimerManager::Stop()
{
    m_state = TimerState::Stopped;
    m_remainingSeconds = m_settings.workDuration;
    m_totalSeconds = m_settings.workDuration;
    m_warningShown = false;
}

void TimerManager::OnTick()
{
    // 如果是停止状态，不执行任何操作
    if (m_state == TimerState::Stopped)
    {
        // 通知 UI 重置到初始状态
        if (m_onTick)
        {
            m_onTick(m_remainingSeconds, false);
        }
        return;
    }

    if (m_state == TimerState::Paused)
    {
        return;
    }

    // 如果在休息状态，完全不执行倒计时（由 BreakWindow 自己管理）
    if (m_state == TimerState::Break)
    {
        return;
    }

    // 只在工作状态下执行倒计时
    if (m_state == TimerState::Working)
    {
        // 倒计时回调
        if (m_onTick)
        {
            m_onTick(m_remainingSeconds, true);
        }

        // 检查是否需要显示警告（剩余30秒）
        if (!m_warningShown && m_remainingSeconds <= 30 && m_remainingSeconds > 0)
        {
            m_warningShown = true;
            if (m_onWarning)
            {
                m_onWarning();
            }
        }

        // 倒计时
        if (m_remainingSeconds > 0)
        {
            m_remainingSeconds--;
        }
        else
        {
            // 工作结束，增加完成的番茄钟数
            m_completedPomodoros++;

            // 开始休息
            StartBreak();
        }
    }
}

void TimerManager::EndBreak()
{
    // 休息结束回调
    if (m_onBreakEnd)
    {
        m_onBreakEnd();
    }

    // 设置为停止状态，不自动开始新工作
    m_state = TimerState::Stopped;
    m_remainingSeconds = m_settings.workDuration;
    m_totalSeconds = m_settings.workDuration;
    m_warningShown = false;
}
