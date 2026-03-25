# WorkWell for Windows

强制休息番茄钟 - 纯 C++ + Win32 API 实现

## 功能特性

- ✅ 系统托盘图标
- ✅ 番茄钟计时器（25分钟工作 + 5分钟休息）
- ✅ 全屏休息窗口（不可关闭）
- ✅ 倒计时显示
- ✅ 右键菜单（暂停/继续、立即休息、退出）
- ✅ 性能高，体积小，无第三方依赖

## 编译要求

- Windows 10 或更高版本
- Visual Studio 2019 或更高版本（包含 C++ 构建工具）
- CMake 3.15 或更高版本

## 编译步骤

### 使用 Visual Studio

```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### 使用 MinGW-w64

```cmd
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
mingw32-make
```

### 使用 CMake（其他编译器）

```cmd
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

## 运行

编译完成后，可执行文件位于：
- `build/bin/Release/WorkWell.exe` (Visual Studio)
- `build/bin/WorkWell.exe` (MinGW/其他)

直接运行即可，无需安装任何依赖。

## 使用方法

1. **启动应用**：双击 WorkWell.exe
2. **查看倒计时**：鼠标悬停在托盘图标上
3. **暂停/继续**：双击托盘图标或右键菜单
4. **立即休息**：右键菜单 → 立即休息
5. **退出应用**：右键菜单 → 退出

## 系统要求

- Windows 10/11 (x64)
- 约 500KB 磁盘空间
- 无需额外运行时依赖

## 技术栈

- **语言**：C++17
- **框架**：纯 Win32 API（无第三方库）
- **编译器**：MSVC / MinGW-w64 / Clang
- **构建工具**：CMake

## 许可证

MIT License
