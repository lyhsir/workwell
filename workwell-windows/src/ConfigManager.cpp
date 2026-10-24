#include "ConfigManager.h"
#include <windows.h>
#include <fstream>
#include <sstream>
#include <shlobj.h>
#include <algorithm>

std::wstring ConfigManager::GetDataPath(const wchar_t *name)
{
    wchar_t folder[MAX_PATH]{};
    if (FAILED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, folder)))
        return name;
    std::wstring path = std::wstring(folder) + L"\\WorkWell";
    CreateDirectoryW(path.c_str(), NULL);
    return path + L"\\" + name;
}

std::wstring ConfigManager::GetConfigPath()
{
    return GetDataPath(L"WorkWell.config");
}

bool ConfigManager::Save(const TimerManager::Settings &settings, const std::wstring &filePath)
{
    std::wstring path = filePath;
    if (path.empty())
    {
        path = GetConfigPath();
    }

    // 使用 WriteFile 写入配置文件
    HANDLE hFile =
        CreateFileW(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

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
                        settings.workDuration, settings.shortBreakDuration, settings.longBreakDuration,
                        settings.pomodorosUntilLongBreak, settings.canSkipBreak ? 1 : 0);

    DWORD bytesWritten;
    BOOL result = WriteFile(hFile, buffer, len, &bytesWritten, NULL);
    CloseHandle(hFile);

    return result && bytesWritten == (DWORD)len;
}

bool ConfigManager::Load(TimerManager::Settings &settings, const std::wstring &filePath)
{
    std::wstring path = filePath;
    if (path.empty())
    {
        path = GetConfigPath();
    }

    // 读取文件
    HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false; // 文件不存在，使用默认值
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
    char *context = NULL;
    char *line = strtok_s(buffer, "\n", &context);

    while (line != NULL)
    {
        // 跳过空白行和注释
        while (*line == ' ' || *line == '\t' || *line == '\r')
            line++;
        if (*line == '\0' || *line == '#')
        {
            line = strtok_s(NULL, "\n", &context);
            continue;
        }

        // 解析 key=value
        char *equals = strchr(line, '=');
        if (equals != NULL)
        {
            *equals = '\0';
            char *key = line;
            char *value = equals + 1;

            // 去除空白
            char *keyEnd = equals - 1;
            while (keyEnd > key && (*keyEnd == ' ' || *keyEnd == '\t' || *keyEnd == '\r'))
            {
                *keyEnd = '\0';
                keyEnd--;
            }

            while (*value == ' ' || *value == '\t')
                value++;

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

    settings.workDuration = std::clamp(settings.workDuration, 60, 7200);
    settings.shortBreakDuration = std::clamp(settings.shortBreakDuration, 60, 1800);
    settings.longBreakDuration = std::clamp(settings.longBreakDuration, 60, 3600);
    settings.pomodorosUntilLongBreak = std::clamp(settings.pomodorosUntilLongBreak, 2, 10);
    return true;
}
