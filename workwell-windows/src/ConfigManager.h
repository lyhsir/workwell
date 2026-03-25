#pragma once

#include <string>
#include "TimerManager.h"

class ConfigManager
{
public:
    // 保存配置到文件
    static bool Save(const TimerManager::Settings& settings, const std::wstring& filePath = L"WorkWell.config");

    // 从文件加载配置
    static bool Load(TimerManager::Settings& settings, const std::wstring& filePath = L"WorkWell.config");

    // 获取默认配置文件路径（在程序目录）
    static std::wstring GetConfigPath();
};
