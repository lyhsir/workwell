#include "ConfigManager.h"
#include <windows.h>
#include <fstream>
#include <sstream>

std::wstring ConfigManager::GetConfigPath()
{
    // 获取程序所在目录
    wchar_t modulePath[MAX_PATH];
    GetModuleFileNameW(NULL, modulePath, MAX_PATH);

    // 获取目录部分
    std::wstring path(modulePath);
    size_t pos = path.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
    {
        path = path.substr(0, pos);
    }

    path += L"\\WorkWell.config";
    return path;
}

bool ConfigManager::Save(const TimerManager::Settings& settings, const std::wstring& filePath)
{
    std::wstring path = filePath;
    if (path.empty())
    {
        path = GetConfigPath();
    }

    // 使用 WriteFile 写入配置文件
    HANDLE hFile = CreateFileW(
        path.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    // 写入配置内容
    char buffer[512];
    int len = sprintf_s(buffer, sizeof(buffer),
        "workDuration=%d\n"
        "shortBreakDuration=%d\n"
        "longBreakDuration=%d\n"
        "pomodorosUntilLongBreak=%d\n"
        "canSkipBreak=%d\n",
        settings.workDuration,
        settings.shortBreakDuration,
        settings.longBreakDuration,
        settings.pomodorosUntilLongBreak,
        settings.canSkipBreak ? 1 : 0
    );

    DWORD bytesWritten;
    BOOL result = WriteFile(hFile, buffer, len, &bytesWritten, NULL);
    CloseHandle(hFile);

    return result && bytesWritten == (DWORD)len;
}

bool ConfigManager::Load(TimerManager::Settings& settings, const std::wstring& filePath)
{
    std::wstring path = filePath;
    if (path.empty())
    {
        path = GetConfigPath();
    }

    // 读取文件
    HANDLE hFile = CreateFileW(
        path.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;  // 文件不存在，使用默认值
    }

    char buffer[512];
    DWORD bytesRead;
    if (!ReadFile(hFile, buffer, sizeof(buffer) - 1, &bytesRead, NULL))
    {
        CloseHandle(hFile);
        return false;
    }
    buffer[bytesRead] = '\0';
    CloseHandle(hFile);

    // 解析配置
    char* context = NULL;
    char* line = strtok_s(buffer, "\n", &context);

    while (line != NULL)
    {
        // 跳过空白行和注释
        while (*line == ' ' || *line == '\t' || *line == '\r') line++;
        if (*line == '\0' || *line == '#')
        {
            line = strtok_s(NULL, "\n", &context);
            continue;
        }

        // 解析 key=value
        char* equals = strchr(line, '=');
        if (equals != NULL)
        {
            *equals = '\0';
            char* key = line;
            char* value = equals + 1;

            // 去除空白
            char* keyEnd = equals - 1;
            while (keyEnd > key && (*keyEnd == ' ' || *keyEnd == '\t' || *keyEnd == '\r')) { *keyEnd = '\0'; keyEnd--; }

            while (*value == ' ' || *value == '\t') value++;

            // 解析具体配置项
            if (strcmp(key, "workDuration") == 0)
            {
                settings.workDuration = atoi(value);
            }
            else if (strcmp(key, "shortBreakDuration") == 0)
            {
                settings.shortBreakDuration = atoi(value);
            }
            else if (strcmp(key, "longBreakDuration") == 0)
            {
                settings.longBreakDuration = atoi(value);
            }
            else if (strcmp(key, "pomodorosUntilLongBreak") == 0)
            {
                settings.pomodorosUntilLongBreak = atoi(value);
            }
            else if (strcmp(key, "canSkipBreak") == 0)
            {
                settings.canSkipBreak = (atoi(value) != 0);
            }
        }

        line = strtok_s(NULL, "\n", &context);
    }

    return true;
}
