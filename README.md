# WorkWell

<div align="center">

![WorkWell Icon](icon.png)

**🍅 好好工作，好好休息**

一个帮助你规律工作和休息的 macOS 菜单栏应用，具有可配置的"强制休息"功能和精美的界面设计。

[![Swift](https://img.shields.io/badge/Swift-5.9+-orange.svg)](https://swift.org)
[![Platform](https://img.shields.io/badge/platform-macOS%2013.0+-blue.svg)](https://www.apple.com/macos)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

[English](README_EN.md) | 简体中文

</div>

## ✨ 功能特性

### 🍅 番茄钟计时
- ⏱️ 自定义工作时长（默认 25 分钟）
- ☕ 自定义短休息（默认 5 分钟）
- 🌴 自定义长休息（默认 15 分钟）
- 🔄 可配置长休息间隔（每 N 个番茄后）
- ⏸️ 支持暂停和继续功能
- ⏰ 工作时菜单栏实时显示倒计时（系统默认颜色）

### 🎨 精美界面设计
- 🎨 现代化 SwiftUI 界面
- 📊 圆形进度条可视化显示
- 🔔 工作完成时的友好提醒界面
- 🎯 自适应深色/浅色模式

### 🔒 强制休息机制
两种强度级别：
- **可退出**：可以随时退出休息，适合灵活使用
- **不可退出**：必须休息完整时间，强制养成习惯

### 📊 统计功能
- 📈 今日完成的番茄数
- ⏰ 今日专注总时长
- 🔥 连续专注天数追踪
- 💾 所有数据本地持久化存储

### ⚙️ 灵活配置
- ✅ 允许/禁止跳过休息
- 📝 跳过休息是否需要输入原因
- ⏱️ 跳过延迟时间（防止误触，0-30 秒）
- 🎯 完全自定义的时间设置

## 🚀 安装和运行

### 系统要求
- macOS 13.0 (Ventura) 或更高版本
- Swift 5.9 或更高版本（仅开发需要）

### 📦 下载 Release 版本（推荐）

> 即将在 GitHub Releases 提供

### 从源码构建

```bash
# 1. 克隆仓库
git clone https://github.com/lyhsir/workwell.git
cd workwell

# 2. 构建项目
swift build

# 3. 运行应用
swift run WorkWell
```

### 构建 Release 版本

```bash
# 编译 Release 版本
swift build -c release

# 打包为 .app
mkdir -p build/Release/WorkWell.app/Contents/{MacOS,Resources}
cp .build/arm64-apple-macosx/release/WorkWell build/Release/WorkWell.app/Contents/MacOS/
cp Sources/Timeout/Timeout-16.png build/Release/WorkWell.app/Contents/Resources/
cp WorkWell.icns build/Release/WorkWell.app/Contents/Resources/AppIcon.icns

# 创建 Info.plist
cat > build/Release/WorkWell.app/Contents/Info.plist << 'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>WorkWell</string>
    <key>CFBundleIdentifier</key>
    <string>com.workwell.timer</string>
    <key>CFBundleName</key>
    <string>WorkWell</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>1.0</string>
    <key>CFBundleVersion</key>
    <string>1</string>
    <key>LSMinimumSystemVersion</key>
    <string>13.0</string>
    <key>LSUIElement</key>
    <true/>
    <key>NSHighResolutionCapable</key>
    <true/>
</dict>
</plist>
EOF

# 运行应用
open build/Release/WorkWell.app
```

### 使用启动脚本

```bash
# 一键构建并运行
./run.sh
```

### 开发模式

```bash
# 生成 Xcode 项目
swift package generate-xcodeproj

# 在 Xcode 中打开
open WorkWell.xcodeproj
```

## 📖 使用指南

### 基本使用流程

1. **启动应用** 🚀
   - 应用启动后会出现在菜单栏
   - 点击 WorkWell 图标打开控制面板

2. **开始工作** ⏰
   - 点击"开始工作"按钮开始倒计时
   - 菜单栏会实时显示剩余时间（如 `24:59`）

3. **工作中** 🎯
   - 可以随时暂停计时
   - 可以提前结束休息
   - 菜单栏保持倒计时显示

4. **工作完成** ✅
   - 自动显示提醒窗口
   - 可以选择立即休息或继续工作
   - 进入休息模式后全屏显示

5. **休息时间** 💤
   - 全屏显示休息界面
   - 显示剩余休息时间
   - 休息结束后自动恢复

### 配置选项

点击控制面板中的齿轮图标 ⚙️ 打开设置：

#### ⏰ 时间设置
- **工作时长**：1-60 分钟（默认 25 分钟）
- **短休息**：1-30 分钟（默认 5 分钟）
- **长休息**：5-60 分钟（默认 15 分钟）
- **长休息间隔**：2-10 个番茄后触发（默认 4 个）

#### 🔒 休息设置
- **强度模式**：
  - 可退出：可以随时关闭休息窗口
  - 不可退出：必须等待休息时间结束
- **允许跳过**：是否显示"跳过休息"按钮
- **跳过原因**：跳过时是否需要输入原因
- **跳过延迟**：防止误触的延迟时间

## 🛠️ 技术架构

### 技术栈
- **语言**：Swift 5.9+
- **UI 框架**：SwiftUI
- **系统框架**：AppKit（菜单栏集成）
- **数据存储**：UserDefaults
- **构建工具**：Swift Package Manager

### 项目结构
```
workwell/
├── Sources/Timeout/
│   ├── main.swift              # 应用入口和菜单栏管理
│   ├── TimerManager.swift      # 计时器核心逻辑
│   ├── TimerView.swift         # 主界面视图
│   ├── BreakWindow.swift       # 全屏休息窗口
│   ├── BreakWarningWindow.swift # 工作完成提醒
│   └── Timeout-16.png          # 菜单栏图标（16x16）
├── Package.swift               # Swift Package 配置
├── icon.png                    # 应用图标源文件（1024x1024）
├── WorkWell.icns               # 应用图标（60KB）
├── run.sh                      # 启动脚本
└── README.md                   # 项目文档
```

### 核心组件

**TimerManager**：计时器管理
- 状态管理（空闲/工作/暂停/休息）
- 时间计算和进度更新
- 统计数据持久化

**TimerView**：主界面
- 圆形进度条显示
- 控制按钮交互
- 统计信息展示

**AppDelegate**：系统集成
- 菜单栏图标管理
- 工作时显示倒计时（系统默认颜色）
- 窗口生命周期管理

### 应用大小
- 可执行文件：607KB
- 应用图标：60KB
- 总大小：约 680KB

## 🔒 隐私和安全

### 数据隐私
- ✅ 所有数据仅在本地存储
- ✅ 不收集任何使用统计
- ✅ 不需要网络连接
- ✅ 不上传任何信息

### 系统权限
- ✅ 不需要特殊系统权限
- ✅ 不修改系统文件或设置
- ✅ 可以随时通过活动监视器强制退出

### 注意事项
⚠️ **重要提示**：
1. 这不是真正的系统锁，可以通过活动监视器强制退出
2. 遇到紧急情况时，可以通过活动监视器强制退出 WorkWell 进程
3. 统计数据存储在 UserDefaults，卸载应用会丢失数据

## 🔧 故障排除

### 应用不显示在菜单栏
- 检查应用是否正在运行（活动监视器）
- 尝试重启应用

### 休息窗口不显示
- 确认没有其他全屏应用正在运行
- 检查显示设置

### 计时不准确
- 如果电脑进入睡眠，计时器会暂停
- 唤醒后会继续计时

### 图标显示问题
- 清理构建缓存：`rm -rf .build && swift build`
- 确保图标文件在 `Sources/Timeout/Timeout-16.png`

## 📚 文档

- **[设计文档](DESIGN_DOCUMENT.md)** - 完整的设计规范
- **[使用说明](USAGE.md)** - 详细的使用教程
- **[开发文档](DEVELOPMENT.md)** - 开发者指南和架构说明
- **[项目总结](PROJECT_SUMMARY.md)** - 项目概览和技术栈

## 🗺️ 开发路线图

### 已实现 ✅
- [x] 基本番茄钟功能
- [x] 强制休息机制
- [x] 统计数据追踪
- [x] 菜单栏倒计时显示
- [x] 现代化 SwiftUI 界面
- [x] 应用图标集成
- [x] 可配置的设置选项
- [x] 自适应深色/浅色模式

### 未来改进 🚀
- [ ] 数据导出功能（CSV/JSON）
- [ ] iCloud 同步支持
- [ ] 自定义休息提示内容
- [ ] 统计图表和趋势分析
- [ ] 通知提醒集成
- [ ] 音效和声音提示
- [ ] 多语言支持

## 📄 许可证

MIT License - 自由使用和修改

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

### 贡献指南
1. Fork 本项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启 Pull Request

## 🌟 Star History

如果这个项目对你有帮助，请给一个 ⭐️ Star！

---

<div align="center">

**享受高效的工作时间！** 🍅⏰

Made with ❤️ by [lyhsir](https://github.com/lyhsir)

</div>
