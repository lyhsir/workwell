#!/bin/bash

echo "🎨 使用新图片创建应用图标..."

# 使用新的图片文件
SOURCE_IMAGE="tiimeout-iOS-Default-1024x1024@1x.png"

# 检查文件是否存在
if [ ! -f "$SOURCE_IMAGE" ]; then
    echo "❌ 错误: 找不到 $SOURCE_IMAGE"
    exit 1
fi

echo "📷 使用图片: $SOURCE_IMAGE"

# 创建 iconset 目录
ICONSET="Timeout.iconset"
rm -rf "$ICONSET"
mkdir -p "$ICONSET"

# 使用 sips 工具生成不同尺寸的图标
echo "🔨 生成不同尺寸的图标..."
sips -z 16 16     "$SOURCE_IMAGE" --out "$ICONSET/icon_16x16.png"
sips -z 32 32     "$SOURCE_IMAGE" --out "$ICONSET/icon_16x16@2x.png"
sips -z 32 32     "$SOURCE_IMAGE" --out "$ICONSET/icon_32x32.png"
sips -z 64 64     "$SOURCE_IMAGE" --out "$ICONSET/icon_32x32@2x.png"
sips -z 128 128   "$SOURCE_IMAGE" --out "$ICONSET/icon_128x128.png"
sips -z 256 256   "$SOURCE_IMAGE" --out "$ICONSET/icon_128x128@2x.png"
sips -z 256 256   "$SOURCE_IMAGE" --out "$ICONSET/icon_256x256.png"
sips -z 512 512   "$SOURCE_IMAGE" --out "$ICONSET/icon_256x256@2x.png"
sips -z 512 512   "$SOURCE_IMAGE" --out "$ICONSET/icon_512x512.png"
sips -z 1024 1024 "$SOURCE_IMAGE" --out "$ICONSET/icon_512x512@2x.png"

# 生成 icns 文件
echo "📦 生成 icns 文件..."
iconutil -c icns "$ICONSET"

if [ -f "Timeout.icns" ]; then
    echo "✅ 图标创建成功: Timeout.icns"
    ls -lh Timeout.icns
else
    echo "❌ 图标创建失败"
    exit 1
fi
