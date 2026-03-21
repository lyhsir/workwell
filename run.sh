#!/bin/bash
# Timeout 番茄钟应用启动脚本

echo "🍅 启动 Timeout 番茄钟应用..."

# 检查是否已经在运行
if pgrep -f "Timeout" > /dev/null; then
    echo "⚠️  Timeout 已在运行中"
    echo "提示：使用以下命令停止应用"
    echo "  pkill -f Timeout"
    exit 1
fi

# 构建并运行
cd "$(dirname "$0")"
swift build

if [ $? -eq 0 ]; then
    echo "✅ 构建成功，启动应用..."
    swift run Timeout &
    echo "🎉 应用已启动！点击菜单栏的 🍅 图标开始使用"
    echo ""
    echo "停止应用："
    echo "  pkill -f Timeout"
else
    echo "❌ 构建失败"
    exit 1
fi
