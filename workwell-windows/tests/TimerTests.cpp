#include "TimerManager.h"
#include "ConfigManager.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cstdlib>
void Check(bool ok, const char *name)
{
    if (!ok)
    {
        std::cerr << "FAIL: " << name << '\n';
        std::exit(1);
    }
    std::cout << "PASS: " << name << '\n';
}
int main()
{
    auto base = std::filesystem::current_path() / ("workwell-test-" + std::to_string(GetCurrentProcessId()));
    auto stats = base.wstring() + L"-stats.ini", config = base.wstring() + L"-config.ini";
    ULONGLONG now = 1000;
    TimerManager t([&]() { return now; }, stats);
    int ends = 0, starts = 0, warnings = 0, deferred = 0;
    t.SetOnBreakStart([&]() { ++starts; });
    t.SetOnBreakEnd([&]() { ++ends; });
    t.SetOnWarning([&]() { ++warnings; });
    t.SetOnBreakDeferred([&]() { ++deferred; });
    TimerManager::Settings s;
    t.SetSettings(s);
    t.StartWork(1500);
    now += 12000;
    t.OnTick();
    Check(t.Remaining() == 1488, "elapsed time counts delayed message delivery correctly");
    t.Pause();
    now += 100000;
    t.OnTick();
    Check(t.Remaining() == 1488 && t.IsPaused(), "pause freezes countdown");
    t.Resume();
    now += 1000;
    t.OnTick();
    Check(t.Remaining() == 1487, "resume continues countdown");
    s.shortBreakDuration = 600;
    t.SetSettings(s);
    Check(t.ActiveSettings().shortBreakDuration == 300, "settings are deferred until next round");
    now += 1457000;
    t.OnTick();
    Check(warnings == 1 && t.Remaining() == 30, "warning fires at thirty seconds");
    now += 30000;
    t.OnTick();
    Check(t.IsInBreak() && t.Remaining() == 300 && starts == 1,
          "work transitions to a full five minute break");
    Check(t.TodayCount() == 1 && t.TodayMinutes() == 25, "completed work updates real daily statistics");
    t.DelayBreak();
    Check(t.IsDeferred() && t.Remaining() == 300 && deferred == 1,
          "gentle mode postpones break for five minutes");
    now += 300000;
    t.OnTick();
    Check(t.IsInBreak() && t.GetCompletedPomodoros() == 1, "postponement does not count as another pomodoro");
    now += 300000;
    t.OnTick();
    Check(t.IsStopped() && ends == 1, "break ends at exact duration and waits for user");
    t.StartWork(60);
    now += 60000;
    t.OnTick();
    Check(t.Remaining() == 600, "saved short break applies in next round");
    t.EndBreak();
    s.canSkipBreak = false;
    t.SetSettings(s);
    t.StartBreak();
    t.DelayBreak();
    Check(t.IsInBreak() && !t.IsDeferred(), "strict mode rejects postponement");
    t.EndBreak();
    s.pomodorosUntilLongBreak = 4;
    t.SetSettings(s);
    for (int i = 0; i < 2; ++i)
    {
        t.StartWork(60);
        now += 60000;
        t.OnTick();
        if (i == 0)
            t.EndBreak();
    }
    Check(t.GetCompletedPomodoros() == 4 && t.Remaining() == 900,
          "fourth completed round triggers fifteen minute long break");
    t.EndBreak();
    TimerManager reload([&]() { return now; }, stats);
    Check(reload.TodayCount() == 4 && reload.TodayMinutes() == 28, "daily statistics survive restart");
    Check(ConfigManager::Save(s, config), "configuration saves successfully");
    TimerManager::Settings loaded;
    Check(ConfigManager::Load(loaded, config) && loaded.shortBreakDuration == 600 && !loaded.canSkipBreak,
          "configuration round trip");
    {
        std::ofstream out{std::filesystem::path(config)};
        out << "pomodorosUntilLongBreak=0\nworkDuration=-1\nshortBreakDuration=999999\n";
    }
    ConfigManager::Load(loaded, config);
    Check(loaded.pomodorosUntilLongBreak == 2 && loaded.workDuration == 60 &&
              loaded.shortBreakDuration == 1800,
          "malformed configuration is clamped safely");
    DeleteFileW(stats.c_str());
    DeleteFileW(config.c_str());
    std::cout << "All checks passed.\n";
    return 0;
}
