#pragma once

#include <string>
#include "TimerManager.h"

class ConfigManager
{
  public:
    // 保存配置到文件
    static bool Save(const TimerManager::Settings &settings, const std::wstring &filePath = L"");

    // 从文件加载配置
    static bool Load(TimerManager::Settings &settings, const std::wstring &filePath = L"");

    // 配置和统计保存在当前用户的 LocalAppData/WorkWell 目录。
    static std::wstring GetConfigPath();
    static std::wstring GetDataPath(const wchar_t *name);
};
