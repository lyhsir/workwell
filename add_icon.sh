#!/bin/bash

echo "📱 更新应用图标..."

# 创建 Resources 目录
mkdir -p build/Release/Timeout.app/Contents/Resources

# 复制图标文件
cp Timeout.icns build/Release/Timeout.app/Contents/Resources/AppIcon.icns

echo "✅ 图标已更新"
echo ""
ls -lh build/Release/Timeout.app/Contents/Resources/
