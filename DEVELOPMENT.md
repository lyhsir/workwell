# Timeout 开发文档

## 🏗️ 项目结构

```
timeout/
├── Package.swift                    # Swift Package Manager 配置
├── README.md                        # 项目说明
├── USAGE.md                         # 使用指南
├── DEVELOPMENT.md                   # 本文件
├── run.sh                          # 启动脚本
└── Sources/
    └── Timeout/
        ├── main.swift              # 应用入口和 AppDelegate
        ├── TimerManager.swift      # 计时器逻辑管理
        ├── TimerView.swift         # 主界面视图
        └── BreakWindow.swift       # 休息窗口和强制休息界面
```

## 🔧 技术栈

- **语言**: Swift 5.9+
- **UI 框架**: SwiftUI
- **系统框架**: AppKit
- **最低系统**: macOS 13.0+

## 📋 核心组件说明

### 1. TimerManager (TimerManager.swift)
负责所有计时逻辑和状态管理。

**主要属性**:
- `workDuration`: 工作时长（秒）
- `breakDuration`: 短休息时长（秒）
- `longBreakDuration`: 长休息时长（秒）
- `currentState`: 当前状态（空闲/工作/暂停/休息）
- `completedPomodoros`: 已完成番茄数

**主要方法**:
- `startWork()`: 开始工作计时
- `pause()` / `resume()`: 暂停/继续
- `skipBreak(reason:)`: 跳过休息

### 2. TimerView (TimerView.swift)
SwiftUI 视图，显示计时器界面和设置。

**布局结构**:
```
TimerView
├── 标题栏（图标 + 设置按钮）
├── 计时器显示（时间 + 状态 + 进度条）
├── 控制按钮（开始/暂停/继续/停止）
└── 统计信息（今日番茄/专注时长/连续天数）
```

### 3. BreakWindow (BreakWindow.swift)
强制休息的全屏窗口。

**强度级别**:
- **Light**: 半透明渐变背景
- **Medium**: 不透明遮罩 + 快捷键拦截
- **Heavy**: 完全锁定界面

### 4. AppDelegate (main.swift)
管理应用生命周期和菜单栏集成。

## 🚀 扩展指南

### 添加新功能

#### 1. 数据持久化改进
当前使用 UserDefaults，可以升级为：
- Core Data（适合复杂查询）
- JSON 文件（简单导出）
- SQLite（跨平台兼容）

#### 2. 自定义休息提示
在 `BreakWindow.swift` 中：
```swift
private let stretchTips = [
    // 添加自定义提示
]
```

或从配置文件读取：
```swift
func loadTipsFromFile() -> [String] {
    // 从 JSON/PLIST 文件加载
}
```

#### 3. 统计图表
使用 Swift Charts 框架：
```swift
import Charts

Chart(statsData) { item in
    BarMark(
        x: .value("日期", item.date),
        y: .value("番茄数", item.count)
    )
}
```

#### 4. 通知集成
添加 UNUserNotification 支持：
```swift
import UserNotifications

func sendNotification(title: String, body: String) {
    let content = UNMutableNotificationContent()
    content.title = title
    content.body = body
    // ...
}
```

### 性能优化

#### 1. 后台保活
```swift
// 在 TimerManager 中添加
private var backgroundTask: NSObjectProtocol?

func enterBackground() {
    backgroundTask = NSApplication.shared.beginBackgroundTask {
        // 清理工作
    }
}
```

#### 2. 内存管理
```swift
// 使用 weak 引用避免循环引用
onBreakStart = { [weak self] in
    self?.showBreakScreen()
}
```

## 🐛 调试技巧

### 查看日志
```swift
print("Timer state: \(currentState)")
print("Time remaining: \(currentTime)")
```

### 测试不同状态
```swift
// 在 TimerManager 中添加测试方法
func testStateTransition() {
    startWork()
    DispatchQueue.main.asyncAfter(deadline: .now() + 2) {
        pause()
        // ...
    }
}
```

### 模拟计时器完成
```swift
// 快速测试休息窗口
func testBreakWindow() {
    currentTime = 2  // 2 秒后触发休息
    startWork()
}
```

## 📦 发布准备

### 1. 创建签名证书
```bash
# 在 Xcode 中配置开发者账号
# Project Settings > Signing & Capabilities
```

### 2. 打包应用
```bash
# 使用 Xcode Archive
# 或使用命令行工具
xcodebuild -scheme Timeout archive
```

### 3. 公测分发
- GitHub Releases
- Homebrew Cask
- 直接下载

### 4. App Store 发布
需要修改：
- 移除某些"强制"功能（审核问题）
- 添加隐私政策
- 完善帮助文档
- 准备截图和预览视频

## 🔒 安全和权限

### 当前权限需求
- 无需特殊权限
- 不访问网络
- 不访问敏感数据

### 未来功能可能需要的权限
- **辅助功能**: 更强的快捷键拦截
- **通知权限**: 系统通知提醒
- **访问控制**: 防止强制退出应用

### 隐私考虑
- 所有数据本地存储
- 不收集使用统计
- 不上传任何信息

## 🤝 贡献指南

### 代码风格
- 使用 4 空格缩进
- 遵循 Swift API 设计指南
- 添加必要的注释

### 提交规范
```
feat: 添加新功能
fix: 修复 bug
docs: 更新文档
style: 代码格式调整
refactor: 重构代码
test: 添加测试
chore: 构建/工具变更
```

### 测试要求
- 测试 macOS 13+ 各版本
- 测试不同屏幕尺寸
- 测试睡眠/唤醒场景
- 测试多显示器场景

## 📚 参考资源

- [SwiftUI 官方文档](https://developer.apple.com/documentation/swiftui)
- [AppKit 官方文档](https://developer.apple.com/documentation/appkit)
- [Swift Package Manager](https://www.swift.org/package-manager/)
- [macOS Human Interface Guidelines](https://developer.apple.com/design/human-interface-guidelines/macos)

## 🎯 路线图

### v1.1（计划中）
- [ ] 数据导出功能
- [ ] 更多休息提示内容
- [ ] 自定义快捷键
- [ ] 音效选择

### v1.2
- [ ] iCloud 同步
- [ ] 统计图表
- [ ] 主题定制
- [ ] 多语言支持

### v2.0
- [ ] Apple Watch 伴侣应用
- [ ] 与日历集成
- [ ] 团队协作功能
- [ ] AI 智能建议

---

欢迎贡献代码和想法！🎉
