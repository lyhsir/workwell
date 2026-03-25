# 编译说明

## 已修复的问题

1. ✅ CMakeLists.txt - 添加了 `/utf-8` 编译选项
2. ✅ TrayIcon.cpp - 使用 Unicode 版本的字符串函数（`StringCchCopyW`, `StringCchPrintfW`）
3. ✅ BreakWindow.cpp - 修复了 `CreateWindow` 宏冲突，使用 `::CreateWindowExW` 显式调用

## 在 Windows 上编译

1. **将修复后的文件复制到 Windows**
   - 复制整个 `workwell-windows` 文件夹

2. **在 PowerShell 中编译**
   ```powershell
   cd D:\develop\workwell-windows
   
   # 删除旧构建
   Remove-Item -Recurse -Force build -ErrorAction SilentlyContinue
   
   # 重新编译
   mkdir build
   cd build
   cmake .. -G "Visual Studio 17 2022" -A x64
   cmake --build . --config Release
   ```

3. **运行程序**
   ```powershell
   .\bin\Release\WorkWell.exe
   ```

## 主要修改说明

### Unicode 字符串函数
- `StringCchCopy` → `StringCchCopyW`
- `StringCchPrintf` → `StringCchPrintfW`
- 所有宽字符串函数使用 `W` 后缀

### CreateWindow 宏冲突
- 使用 `::CreateWindowExW` 显式调用 Windows API
- 避免 Windows 宏展开导致的语法错误

## 预期结果

编译成功后：
- `build/bin/Release/WorkWell.exe` 约 300-500 KB
- 系统托盘会显示番茄钟图标
- 鼠标悬停查看倒计时
- 右键菜单控制程序
