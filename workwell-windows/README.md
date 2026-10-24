# WorkWell for Windows

轻量的 Windows 原生番茄钟：专注有节奏，休息有安排。

## 功能

- 青绿色专注面板：圆环倒计时、当前状态、下一次休息安排和真实的今日统计。
- 可配置专注 / 短休息 / 长休息，以及长休息间隔（默认 25 / 5 / 15 分钟，每 4 轮长休息）。
- 温和模式：跳过休息、延后 5 分钟、全屏或窗口显示。
- 严格模式：全屏休息，不显示跳过和延后；紧急时在休息窗口内长按 Esc 3 秒。
- 休息完成页：主动开始下一轮，或稍后开始。
- 托盘菜单、休息前 30 秒通知、暂停与恢复、配置和按日期统计持久化。
- 启动时显示主面板；最小化或关闭主面板后隐藏任务栏窗口，仅保留系统托盘图标，计时继续。点击托盘图标恢复，托盘菜单可完全退出。
- 单实例运行；设置修改从下一轮开始生效。

## 构建与运行

Windows 10/11，C++17 编译器、Windows SDK、CMake 3.15+。已验证 Visual Studio 2026 x64 Release。

```powershell
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
.\build\bin\Release\WorkWell.exe
```

已有构建目录时应沿用它的生成器。本机使用 `Visual Studio 18 2026`。更换编译器请用新的构建目录。

## 操作

- 主按钮：开始 / 暂停 / 继续专注。
- 结束本轮：清零当前轮并返回准备状态，不计入完成统计。
- 托盘单击：打开主面板；右键：开始、暂停 / 继续、立即休息、退出。
- 托盘图标直接显示剩余分钟（向上取整），最后一分钟显示秒数并带 s；悬停显示精确分秒。专注为青绿底、暂停为琥珀底、休息为蓝底；未开始时显示时钟图标。
- 应用窗口中 `Ctrl+Shift+P` 暂停 / 继续，`Ctrl+Shift+S` 结束本轮，`Ctrl+Shift+B` 立即休息。
- 设置输入支持 Tab、方向键和数字步进。Enter 保存，Esc 取消。
- 完成一次完整专注后计入今日统计；提前休息、延后和未完成的轮次不增加统计。

## 数据位置

`%LOCALAPPDATA%\WorkWell\WorkWell.config` 保存设置，`statistics.ini` 保存按本地日期统计的数据。无需管理员权限。旧版本程序目录中的配置不会被删除。

## 实现

C++17、Win32、Windows GDI+、CMake，无第三方 UI 框架。MSVC 静态链接 C++ 运行库。

- `Ui.h`：颜色、字体、圆角、双缓冲绘制、DPI 和按钮。
- `TimerManager`：统一状态、基于单调时钟的计时、长短休息、按日统计。
- `MainDialog`：专注面板与休息完成页。
- `SettingsDialog`：节奏设置、输入校验和保存反馈。
- `BreakWindow`：休息页、多显示器窗口和全屏切换。
- `TrayIcon` / `ConfigManager`：托盘和本地设置。

## 设计和验收

[设计参考](docs/ui/design-reference.png)、实际截图及逐项比对见 [UI 验收报告](docs/ui/REVIEW.md)。系统标题栏、字体栅格化会受 Windows 版本及缩放影响；多显示器、不同 DPI 与完整读屏体验尚需更多设备验证。
