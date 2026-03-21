#!/bin/bash

echo "🔨 开始构建 Timeout 应用..."
echo ""

# 清理旧的构建文件
echo "🧹 清理旧的构建文件..."
rm -rf build

# 编译发布版本
echo "📦 编译发布版本..."
swift build -c release --arch arm64 --arch x86_64

# 创建应用包结构
echo "📱 创建应用包..."
mkdir -p build/Release/Timeout.app/Contents/MacOS
mkdir -p build/Release/Timeout.app/Contents/Resources
mkdir -p build/Release/Timeout.app/Contents/_CodeSignature

# 复制可执行文件
echo "📋 复制可执行文件..."
cp .build/release/Timeout build/Release/Timeout.app/Contents/MacOS/

# 创建 Info.plist
echo "📝 创建 Info.plist..."
cat > build/Release/Timeout.app/Contents/Info.plist << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>Timeout</string>
    <key>CFBundleIdentifier</key>
    <string>com.timeout.app</string>
    <key>CFBundleInfoDictionaryVersion</key>
    <string>6.0</string>
    <key>CFBundleName</key>
    <string>Timeout</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>CFBundleVersion</key>
    <string>1</string>
    <key>LSMinimumSystemVersion</key>
    <string>10.15</string>
    <key>LSUIElement</key>
    <true/>
    <key>NSHighResolutionCapable</key>
    <true/>
    <key>NSPrincipalClass</key>
    <string>NSApplication</string>
</dict>
</plist>
