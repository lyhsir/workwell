#!/bin/bash
# WorkWell for Windows - Linux/WSL 构建脚本（使用 MinGW）

echo "========================================"
echo "  WorkWell for Windows - 构建脚本"
echo "========================================"
echo ""

# 检查 CMake
if ! command -v cmake &> /dev/null; then
    echo "[错误] 未找到 CMake，请先安装 CMake"
    echo "Ubuntu/Debian: sudo apt-get install cmake"
    echo "Fedora: sudo dnf install cmake"
    exit 1
fi

# 检查 MinGW
if ! command -v x86_64-w64-mingw32-g++ &> /dev/null; then
    echo "[错误] 未找到 MinGW-w64，请先安装"
    echo "Ubuntu/Debian: sudo apt-get install mingw-w64"
    echo "Fedora: sudo dnf install mingw64-gcc-c++"
    exit 1
fi

# 创建构建目录
mkdir -p build
cd build

echo "[1/3] 配置项目..."
cmake .. -G "Unix Makefiles" \
    -DCMAKE_TOOLCHAIN_FILE=../cmake/mingw-w64.cmake \
    -DCMAKE_BUILD_TYPE=Release
if [ $? -ne 0 ]; then
    echo "[错误] CMake 配置失败"
    exit 1
fi

echo ""
echo "[2/3] 编译 Release 版本..."
make -j$(nproc)
if [ $? -ne 0 ]; then
    echo "[错误] 编译失败"
    exit 1
fi

echo ""
echo "[3/3] 构建完成！"
echo ""
echo "可执行文件位置:"
echo "  build/bin/WorkWell.exe"
echo ""
echo "文件大小:"
ls -lh bin/WorkWell.exe | awk '{print $5}'
echo ""
