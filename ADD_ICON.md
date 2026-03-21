# 添加应用图标

## 方法一：使用在线图标生成器

1. 访问图标网站：
   - https://www.flaticon.com/search?word=tomato
   - https://www.iconfinder.com/search?q=tomato

2. 选择并下载 PNG 图标（建议 1024x1024 或更大）

3. 将下载的图标重命名为 `icon.png` 并放在项目根目录

4. 运行图标生成脚本：
```bash
bash create_icon.sh
```

## 方法二：使用 macOS 自带工具

1. 使用截图工具（Shift+Cmd+4）截取一个番茄图标

2. 运行脚本生成图标集

## 方法三：手动创建图标集

如果你有设计好的图标，可以手动创建：

```bash
# 创建图标集目录
mkdir -p build/Timeout.iconset

# 创建不同尺寸的图标（需要 icon.png）
sips -z 16 16 icon.png --out build/Timeout.iconset/icon_16x16.png
sips -z 32 32 icon.png --out build/Timeout.iconset/icon_32x32.png
sips -z 64 64 icon.png --out build/Timeout.iconset/icon_64x64.png
sips -z 128 128 icon.png --out build/Timeout.iconset/icon_128x128.png
sips -z 256 256 icon.png --out build/Timeout.iconset/icon_256x256.png
sips -z 512 512 icon.png --out build/Timeout.iconset/icon_512x512.png
sips -z 1024 1024 icon.png --out build/Timeout.iconset/icon_1024x1024.png

# Retina 版本
sips -z 16 16 icon.png --out build/Timeout.iconset/icon_16x16@2x.png
sips -z 32 32 icon.png --out build/Timeout.iconset/icon_32x32@2x.png
sips -z 64 64 icon.png --out build/Timeout.iconset/icon_64x64@2x.png
sips -z 256 256 icon.png --out build/Timeout.iconset/icon_256x256@2x.png
sips -z 512 512 icon.png --out build/Timeout.iconset/icon_512x512@2x.png
```

## 快速开始

1. 下载任意番茄图标（PNG格式，至少 512x512）
2. 保存为 `icon.png`
3. 运行 `bash create_icon.sh`

图标会自动添加到应用中并重新打包 DMG。
