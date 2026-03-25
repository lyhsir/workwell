# WorkWell for Windows - 开发文档

## 项目概述

**WorkWell for Windows** 是强制休息番茄钟应用的 Windows 原生版本，使用纯 C++17 和 Win32 API 开发，完全无第三方依赖。

## 技术栈

- **语言**: C++17
- **框架**: 纯 Win32 API (无 MFC、无 Qt、无其他框架)
- **构建工具**: CMake 3.15+
- **编译器**:
  - MSVC 2019+ (推荐)
  - MinGW-w64 (支持交叉编译)
  - Clang/LLVM

## 核心功能

### 1. 系统托盘图标
- ✅ 托盘图标显示
- ✅ 鼠标悬停显示倒计时
- ✅ 双击暂停/继续
- ✅ 右键菜单（暂停/继续、立即休息、退出）
- ✅ 气泡提示休息提醒

### 2. 番茄钟计时器
- ✅ 工作时间：25 分钟
- ✅ 休息时间：5 分钟
- ✅ 暂停/继续功能
- ✅ 剩余 30 秒时显示警告

### 3. 休息窗口
- ✅ 全屏显示（覆盖所有显示器）
- ✅ 置顶显示（无法切换到其他窗口）
- ✅ 不可关闭（禁用 Alt+F4、ESC 等）
- ✅ 倒计时显示
- ✅ 渐变背景

## 文件结构

```
workwell-windows/
├── src/
│   ├── main.cpp              # 程序入口 (126 行)
│   ├── TrayIcon.cpp          # 系统托盘实现 (161 行)
│   ├── TrayIcon.h            # 托盘头文件 (42 行)
│   ├── TimerManager.cpp      # 计时器逻辑 (78 行)
│   ├── TimerManager.h        # 计时器头文件 (46 行)
│   ├── BreakWindow.cpp       # 休息窗口 (172 行)
│   ├── BreakWindow.h         # 休息窗口头文件 (30 行)
│   ├── resource.h            # 资源头文件
│   ├── resource.rc           # 资源脚本
│   └── ICON_README.md        # 图标说明
├── cmake/
│   └── mingw-w64.cmake       # MinGW 交叉编译工具链
├── CMakeLists.txt            # CMake 构建配置
├── build.bat                 # Windows 快速构建脚本
├── build.sh                  # Linux/WSL 构建脚本
├── README.md                 # 用户文档
└── DEVELOPMENT.md            # 本文档
```

## 编译指南

### 在 Windows 上编译（Visual Studio）

**要求：**
- Windows 10/11
- Visual Studio 2019 或更高版本
- CMake 3.15+

**步骤：**
```cmd
# 方法 1: 使用快速构建脚本
build.bat

# 方法 2: 手动编译
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### 在 Windows 上编译（MinGW）

**要求：**
- MinGW-w64 8.0+
- CMake 3.15+
- MSYS2 或独立安装的 MinGW

**步骤：**
```cmd
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
mingw32-make
```

### 在 Linux/WSL 上交叉编译

**要求：**
- Ubuntu/Debian: `sudo apt-get install mingw-w64 cmake`
- Fedora: `sudo dnf install mingw64-gcc-c++ cmake`

**步骤：**
```bash
./build.sh
```

## 性能特性

### 体积优化
- 静态链接运行时（无需 VC++ Redistributable）
- 编译优化：`/O2 /GL /LTCG`
- 预计最终大小：**< 500KB**

### 依赖项
- 仅依赖 Windows 系统自带 DLL：
  - `kernel32.dll`
  - `user32.dll`
  - `gdi32.dll`
  - `shell32.dll`
  - `comctl32.dll`

## 测试清单

### 功能测试
- [ ] 托盘图标显示
- [ ] 倒计时显示（悬停提示）
- [ ] 双击托盘图标暂停/继续
- [ ] 右键菜单功能
- [ ] 25 分钟后进入休息
- [ ] 休息前 30 秒气泡提醒
- [ ] 全屏休息窗口
- [ ] 休息窗口不可关闭
- [ ] 5 分钟后休息结束
- [ ] 循环工作/休息

### 兼容性测试
- [ ] Windows 10 (21H2+)
- [ ] Windows 11 (最新版本)
- [ ] 多显示器环境
- [ ] 高 DPI 显示器
- [ ] 触屏设备

## 已知限制

1. **图标文件**：需要手动添加 `WorkWell.ico`（参考 `src/ICON_README.md`）
2. **多语言**：当前仅支持中文，后续可添加本地化
3. **配置持久化**：暂不支持保存用户配置
4. **自定义时间**：工作/休息时间固定为 25/5 分钟

## 未来改进

### 短期
- [ ] 添加托盘图标（自定义图标）
- [ ] 添加配置文件（JSON/INI）
- [ ] 添加开机自启动选项
- [ ] 添加音效提醒

### 长期
- [ ] 多语言支持（英文、日文等）
- [ ] 自定义工作/休息时间
- [ ] 统计数据（工作时长、休息次数）
- [ ] 云同步配置

## 故障排除

### 编译错误

**问题：** `fatal error: windows.h: No such file or directory`
- **解决**：确保在 Windows 平台编译，或安装 MinGW-w64

**问题：** `LINK : fatal error LNK1561: entry point must be defined`
- **解决**：检查 CMakeLists.txt 中的 `SUBSYSTEM:WINDOWS` 设置

### 运行时错误

**问题：** 托盘图标不显示
- **解决**：检查 Windows 版本（需要 Win7+），检查通知区域设置

**问题：** 休息窗口无法全屏
- **解决**：检查 DPI 缩放设置，尝试以管理员身份运行

## 许可证

MIT License - 与 macOS 版本保持一致

## 贡献

欢迎提交 Issue 和 Pull Request！
