# WorkWell for Windows - 快速构建指南

## 方法 1：使用 PowerShell（推荐）

在项目目录打开 PowerShell，运行：

```powershell
.\build.ps1
```

如果提示脚本执行策略错误，先运行：

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\build.ps1
```

## 方法 2：手动命令（CMD/PowerShell）

```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

## 方法 3：使用 Visual Studio

1. 打开 Visual Studio
2. 选择 "打开本地文件夹"
3. 选择 `workwell-windows` 文件夹
4. Visual Studio 会自动识别 CMakeLists.txt
5. 选择 "Release" 配置
6. 点击 "生成" -> "全部生成"

## 方法 4：修复 BUILD.bat 后使用

如果 BUILD.bat 仍然报错，最简单的解决方案是：

1. 用记事本打开 `BUILD.bat`
2. 选择 "文件" -> "另存为"
3. 编码选择 "ANSI"
4. 保存
5. 运行 `BUILD.bat`

## 常见问题

### Q: 提示 "Visual Studio 17 2022" 生成器未找到
**A**: 安装了其他版本的 Visual Studio，需要修改生成器名称：
- VS 2019: `"Visual Studio 16 2019"`
- VS 2022: `"Visual Studio 17 2022"`

### Q: CMake 找不到
**A**: 
1. 安装 CMake: https://cmake.org/download/
2. 或安装 Visual Studio 时勾选 "C++ CMake tools"

### Q: 编译后找不到 WorkWell.exe
**A**: 可执行文件在：
```
build\bin\Release\WorkWell.exe
```

## 编译成功后

运行程序：
```cmd
.\build\bin\Release\WorkWell.exe
```

或双击 `WorkWell.exe` 文件。
