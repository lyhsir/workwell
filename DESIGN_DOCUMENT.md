# WorkWell 应用设计文档
## Windows 移植版本参考

**文档版本**: 1.1
**创建日期**: 2026-03-21
**更新日期**: 2026-03-21
**原项目**: macOS 版本 WorkWell 番茄钟应用（原名 Timeout）

---

## 目录

1. [应用概述](#1-应用概述)
2. [功能需求](#2-功能需求)
3. [用户界面设计](#3-用户界面设计)
4. [数据模型](#4-数据模型)
5. [业务逻辑](#5-业务逻辑)
6. [技术架构](#6-技术架构)
7. [Windows 平台特殊考虑](#7-windows-平台特殊考虑)
8. [实现建议](#8-实现建议)

---

## 1. 应用概述

### 1.1 产品定位
WorkWell 是一个帮助用户规律工作和休息的番茄钟应用，具有可配置的"强制休息"功能。

### 1.2 核心特性
- ✅ 番茄钟计时（工作 25 分钟 / 休息 5 分钟）
- ✅ 可自定义的工作和休息时长
- ✅ 强制休息机制（两种强度：可退出/不可退出）
- ✅ 统计数据追踪
- ✅ 灵活的配置选项
- ✅ 系统托盘集成
- ✅ 菜单栏倒计时显示
- ✅ 自适应深色/浅色模式

### 1.3 目标用户
- 需要专注工作时间的自由职业者
- 需要定时休息提醒的办公室工作者
- 希望养成规律工作习惯的用户

---

## 2. 功能需求

### 2.1 核心功能

#### 2.1.1 番茄钟计时
**优先级**: P0（必须）

**功能描述**:
- 工作计时：默认 25 分钟，可配置 1-60 分钟
- 短休息：默认 5 分钟，可配置 1-30 分钟
- 长休息：默认 15 分钟，可配置 5-60 分钟
- 长休息间隔：每 N 个番茄后触发（默认 4 个）

**交互流程**:
```
[空闲] → [开始工作] → [工作中] → [工作完成] → [休息中] → [休息结束] → [空闲]
   ↓                                                      ↑
   └───────────────── [暂停/继续] ←───────────────────────┘
```

**状态机**:
```typescript
enum TimerState {
  Idle         // 空闲状态
  Working      // 工作中
  Paused       // 已暂停
  Breaking     // 休息中
  SkipRequested // 跳过请求中
}
```

#### 2.1.2 强制休息机制
**优先级**: P0（必须）

**两种强度级别**:

1. **可退出 (Exitable)**
   - 全屏提示窗口
   - 显示休息建议和呼吸节奏
   - 可随时关闭窗口

2. **不可退出 (Forced)**
   - 不透明遮罩覆盖屏幕
   - 限制系统快捷键
   - 显示倒计时
   - 禁止跳过休息
   - 紧急退出需要特殊操作

**跳过控制**:
- 是否允许跳过休息
- 跳过是否需要输入原因
- 跳过延迟时间（防止误触，0-30 秒）

#### 2.1.3 系统托盘集成
**优先级**: P0（必须）

**功能**:
- 常驻系统托盘
- 托盘图标显示：
  - 空闲时：应用图标
  - 工作时：倒计时时间（系统默认颜色）
  - 休息时：休息图标
- 单击打开控制面板
- 右键菜单：
  - 开始/暂停/停止
  - 设置
  - 退出

#### 2.1.4 统计功能
**优先级**: P1（重要）

**统计数据**:
- 今日完成的番茄数
- 今日专注总时长
- 连续专注天数
- 历史累计番茄数

**数据存储**:
- 本地持久化存储
- 跨天自动重置今日统计
- 连续天数自动计算

### 2.2 可选功能

#### 2.2.1 通知提醒
- 工作开始通知
- 休息开始通知
- 工作完成通知

#### 2.2.2 声音效果
- 倒计时结束提示音
- 按钮点击音效
- 可自定义音效文件

#### 2.2.3 主题定制
- 浅色/深色主题
- 自定义配色方案
- 窗口透明度调节

---

## 3. 用户界面设计

### 3.1 主界面（控制面板）

#### 布局结构
```
┌─────────────────────────────────┐
│ 🍅 WorkWell             ⚙️ 设置 │  <- 标题栏
├─────────────────────────────────┤
│                                 │
│         ┌─────────────┐         │
│         │             │  ← 圆形进度条
│         │   25:00     │  ← 时间显示
│         │   准备开始   │  ← 状态文本
│         └─────────────┘         │
│                                 │
│    [开始工作]                   │  <- 控制按钮
│                                 │
├─────────────────────────────────┤
│  今日番茄:     0                │  <- 统计信息
│  专注时长:     0 分钟           │
│  连续天数:     1 天             │
└─────────────────────────────────┘
```

#### 窗口规格
- 尺寸：300 x 450 像素
- 圆角：10 像素
- 背景：系统窗口背景色
- 边框：无边框（macOS 风格）或细边框（Windows 风格）

#### 控件说明

**计时器显示**:
- 圆形进度条：灰色背景，彩色进度
- 时间显示：36pt 等宽字体
- 状态文本：灰色小字
- 进度颜色：
  - 工作中：黑色
  - 暂停：黄色
  - 休息：绿色
  - 跳过请求：红色

**控制按钮**:
- "开始工作"：主要按钮（蓝色）
- "暂停"：次要按钮
- "继续"：主要按钮
- "停止"：次要按钮
- "跳过休息"：次要按钮（可选）

### 3.2 设置界面

#### 布局
```
┌─────────────────────────────────┐
│ 设置                            │
├─────────────────────────────────┤
│ 时间设置                        │
│   工作时长:    [25] 分钟  - +  │
│   短休息:      [5] 分钟   - +  │
│   长休息:      [15] 分钟  - +  │
│   长休息间隔:  [4] 个番茄 - +  │
├─────────────────────────────────┤
│ 休息设置                        │
│   强度: [可退出 ▼]              │
│     ○ 可退出                    │
│     ● 不可退出                  │
│                                 │
│   ☑ 允许跳过休息                │
│   ☐ 跳过需要输入原因            │
│   跳过延迟: [3] 秒  - +         │
├─────────────────────────────────┤
│                        [完成]   │
└─────────────────────────────────┘
```

### 3.3 休息窗口

#### 可退出模式
```
┌─────────────────────────────────┐
│                                 │
│         💤 休息时间              │
│                                 │
│      剩余时间: 05:00             │
│                                 │
│  ┌─────────────────────────┐    │
│  │ 建议活动：                │    │
│  │ • 伸展运动                │
│  │ • 远眺放松                │
│  │ • 喝杯水                  │
│  │ • 深呼吸（4-7-8）         │
│  └─────────────────────────┘    │
│                                 │
│        呼吸节奏：                │
│        吸气 4 秒...             │
│                                 │
│              [跳过休息]          │
└─────────────────────────────────┘
```

#### 不可退出模式
```
┌─────────────────────────────────┐
│ █ 全屏遮罩，完全覆盖             │
│ █                               │
│ █     💤 休息中 05:00            │
│ █                               │
│ █    请放下工作，休息一下        │
│ █                               │
│ █    这是为你好！                │
│ █                               │
│ █   [紧急解锁（长按 3 秒）]      │
└─────────────────────────────────┘
```

### 3.4 警告窗口（工作完成时）

```
┌─────────────────────────────────┐
│  ⏰ 工作时间结束！               │
│                                 │
│  你已经专注工作了 25 分钟        │
│                                 │
│  现在是休息时间了：              │
│  • 短休息：5 分钟                │
│  • 长休息：15 分钟               │
│                                 │
│  ┌─────────────────────────┐    │
│  │ 你可以：                 │    │
│  │ • 立即开始休息            │
│  │ • 继续工作（不推荐）      │
│  └─────────────────────────┘    │
│                                 │
│    [继续工作]  [开始休息]       │
└─────────────────────────────────┘
```

---

## 4. 数据模型

### 4.1 设置数据

```typescript
interface Settings {
  // 时间设置
  workDuration: number;           // 工作时长（秒），默认 1500 (25分钟)
  breakDuration: number;          // 短休息时长（秒），默认 300 (5分钟)
  longBreakDuration: number;      // 长休息时长（秒），默认 900 (15分钟)
  longBreakInterval: number;      // 长休息间隔，默认 4

  // 休息设置
  breakIntensity: BreakIntensity; // 休息强度
  allowSkipBreak: boolean;        // 是否允许跳过休息
  skipBreakRequiresReason: boolean; // 跳过是否需要原因
  skipBreakDelaySeconds: number;  // 跳过延迟（秒）
}

enum BreakIntensity {
  Exitable = "exitable",    // 可退出
  Forced = "forced"         // 不可退出
}
```

### 4.2 统计数据

```typescript
interface Statistics {
  completedPomodoros: number;      // 累计完成番茄数
  todayPomodoros: number;          // 今日番茄数
  todayFocusMinutes: number;       // 今日专注分钟数
  consecutiveDays: number;         // 连续天数
  lastDate: Date;                  // 上次使用日期
}
```

### 4.3 运行时状态

```typescript
interface RuntimeState {
  currentState: TimerState;        // 当前状态
  currentTime: number;             // 当前剩余时间（秒）
  savedTimeBeforeBreak: number;    // 暂停前保存的时间
  pendingBreak: boolean;           // 是否有待处理的休息
}
```

---

## 5. 业务逻辑

### 5.1 状态转换图

```
                    [开始工作]
                       ↓
    ┌─────────────────────────────────┐
    │                                  │
[空闲] ─────→ [工作中] ─────→ [暂停]
    │             ↓                    ↑
    │        [完成] ────┘              │
    │             ↓                    │
    │      [显示警告窗口]               │
    │             ↓                    │
    └────→ [休息中] ──────────────────┘
              ↓
         [休息结束]
              ↓
           [空闲]
```

### 5.2 核心算法

#### 5.2.1 计时器逻辑

```typescript
class TimerManager {
  private timer: Timer | null = null;

  // 开始工作
  startWork() {
    this.currentState = TimerState.Working;
    this.currentTime = this.settings.workDuration;
    this.startTimer();
    this.notifyWorkStart();
  }

  // 开始休息
  startBreak() {
    this.currentState = TimerState.Breaking;

    // 判断是否是长休息
    const isLongBreak =
      (this.stats.completedPomodoros % this.settings.longBreakInterval) === 0;

    this.currentTime = isLongBreak
      ? this.settings.longBreakDuration
      : this.settings.breakDuration;

    this.startTimer();
    this.notifyBreakStart();
  }

  // 计时器心跳（每秒执行）
  private tick() {
    if (this.currentState !== TimerState.Working &&
        this.currentState !== TimerState.Breaking) {
      return;
    }

    this.currentTime--;
    this.notifyTick(this.formatTime(this.currentTime));

    if (this.currentTime <= 0) {
      this.onTimerComplete();
    }
  }

  // 计时器完成
  private onTimerComplete() {
    this.stopTimer();

    if (this.currentState === TimerState.Working) {
      // 工作完成
      this.stats.completedPomodoros++;
      this.stats.todayPomodoros++;
      this.stats.todayFocusMinutes += this.settings.workDuration / 60;
      this.saveStats();

      this.showBreakWarning();
    } else if (this.currentState === TimerState.Breaking) {
      // 休息完成
      this.notifyBreakEnd();
      this.currentState = TimerState.Idle;
    }
  }
}
```

#### 5.2.2 进度计算

```typescript
// 进度百分比（0.0 - 1.0）
get progress(): number {
  if (this.currentState === TimerState.Working) {
    return 1.0 - (this.currentTime / this.settings.workDuration);
  } else if (this.currentState === TimerState.Breaking) {
    const duration = this.isLongBreak()
      ? this.settings.longBreakDuration
      : this.settings.breakDuration;
    return 1.0 - (this.currentTime / duration);
  }
  return 0;
}
```

#### 5.2.3 日期管理

```typescript
// 检查并更新连续天数
private checkAndUpdateConsecutiveDays() {
  const today = new Date();
  const lastDate = this.stats.lastDate;

  if (!this.isSameDay(today, lastDate)) {
    const daysDiff = this.getDaysDiff(lastDate, today);

    if (daysDiff === 1) {
      // 连续使用，增加天数
      this.stats.consecutiveDays++;
    } else if (daysDiff > 1) {
      // 中断了，重置为 1
      this.stats.consecutiveDays = 1;
    }

    // 新的一天，重置今日统计
    this.stats.todayPomodoros = 0;
    this.stats.todayFocusMinutes = 0;
    this.stats.lastDate = today;
  }
}
```

### 5.3 快捷键处理

#### 需要拦截的快捷键

| 快捷键 | 功能 | 拦截级别 |
|--------|------|----------|
| Alt+F4 | 关闭窗口 | 中度/重度 |
| Win+L | 锁定电脑 | 重度 |
| Esc | 关闭对话框 | 中度/重度 |
| Alt+Tab | 切换窗口 | 重度 |
| Ctrl+Alt+Del | 任务管理器 | 重度（尝试） |

**注意**: Windows 系统级快捷键（如 Ctrl+Alt+Del）无法完全拦截。

---

## 6. 技术架构

### 6.1 macOS 版本架构（参考）

```
┌─────────────────────────────────────┐
│         SwiftUI Views               │  ← 用户界面
│  - TimerView (主界面)                │
│  - SettingsView (设置)               │
│  - BreakWindow (休息窗口)            │
└─────────────────────────────────────┘
              ↕
┌─────────────────────────────────────┐
│         TimerManager                │  ← 业务逻辑
│  - 状态管理                          │
│  - 计时器逻辑                        │
│  - 数据持久化                        │
└─────────────────────────────────────┘
              ↕
┌─────────────────────────────────────┐
│         AppKit Framework            │  ← 系统集成
│  - NSStatusItem (菜单栏)            │
│  - NSWindow (窗口管理)              │
│  - UserDefaults (数据存储)          │
└─────────────────────────────────────┘
```

### 6.2 Windows 版本建议架构

```
┌─────────────────────────────────────┐
│         Presentation Layer          │  ← 用户界面
│  - WPF / WinUI3 / Electron          │
│  - 主窗口（控制面板）                │
│  - 设置窗口                          │
│  - 休息窗口                          │
└─────────────────────────────────────┘
              ↕
┌─────────────────────────────────────┐
│         Business Logic Layer        │  ← 业务逻辑
│  - TimerManager                     │
│  - SettingsManager                  │
│  - StatisticsManager                │
└─────────────────────────────────────┘
              ↕
┌─────────────────────────────────────┐
│         Data Access Layer           │  ← 数据访问
│  - JSON / SQLite                    │
│  - 文件 I/O                         │
└─────────────────────────────────────┘
              ↕
┌─────────────────────────────────────┐
│         System Integration          │  ← 系统集成
│  - System Tray (NotifyIcon)         │
│  - Global HotKeys                   │
│  - Window Management                │
└─────────────────────────────────────┘
```

---

## 7. Windows 平台特殊考虑

### 7.1 技术栈选择

#### 方案 1: WPF (.NET)
**优势**:
- 原生 Windows UI，性能好
- 成熟的生态系统
- 支持现代 C# 特性
- 易于打包和分发

**劣势**:
- 仅限 Windows 平台
- UI 设计相对传统

**适用场景**: 追求最佳性能和原生体验

#### 方案 2: WinUI 3 (.NET)
**优势**:
- 最新 Windows UI 框架
- Fluent Design 支持
- 现代化 UI 控件
- 跨平台（Windows 10+）

**劣势**:
- 相对较新，社区资源少
- 学习曲线较陡

**适用场景**: 需要现代化 UI 设计

#### 方案 3: Electron + Web 技术
**优势**:
- 跨平台（Windows/macOS/Linux）
- 使用熟悉的 Web 技术
- 丰富的 UI 库
- 快速开发和迭代

**劣势**:
- 资源占用较高
- 性能略逊于原生应用
- 打包体积大

**适用场景**: 快速开发和跨平台需求

#### 方案 4: Tauri (Rust + Web)
**优势**:
- 轻量级（相比 Electron）
- 安全性高
- 性能好
- 跨平台

**劣势**:
- 需要学习 Rust
- 生态系统相对较小

**适用场景**: 追求性能和跨平台

### 7.2 系统托盘实现

#### WPF 示例

```csharp
// 创建系统托盘图标
var notifyIcon = new NotifyIcon()
{
    Icon = new Icon("app.ico"),
    Text = "WorkWell",
    Visible = true
};

// 添加右键菜单
var contextMenu = new ContextMenuStrip();
contextMenu.Items.Add("开始工作", null, StartWork);
contextMenu.Items.Add("暂停", null, PauseTimer);
contextMenu.Items.Add("-");
contextMenu.Items.Add("设置", null, OpenSettings);
contextMenu.Items.Add("-");
contextMenu.Items.Add("退出", null, ExitApp);
notifyIcon.ContextMenuStrip = contextMenu;

// 单击事件
notifyIcon.Click += (s, e) => ToggleMainWindow();
```

#### WinUI 3 示例

```csharp
// 使用 DesktopNotificationManager
// 或使用第三方库：Hardcodet.NotifyIcon.Wpf
```

### 7.3 全屏遮罩实现

#### WPF 示例

```csharp
// 创建全屏窗口
var maskWindow = new Window()
{
    WindowStyle = WindowStyle.None,
    ResizeMode = ResizeMode.NoResize,
    WindowState = WindowState.Maximized,
    Topmost = true,
    Background = new SolidColorBrush(Color.FromArgb(200, 0, 0, 0)),
    ShowInTaskbar = false
};

// 禁用 Alt+F4 等快捷键
maskWindow.KeyDown += (s, e) => {
    if (e.Key == Key.F4 && Keyboard.Modifiers == ModifierKeys.Alt) {
        e.Handled = true;
    }
};
```

### 7.4 数据持久化

#### 使用 JSON 文件

```typescript
// 配置文件位置
// Windows: %APPDATA%\WorkWell\settings.json

interface AppConfig {
  settings: Settings;
  statistics: Statistics;
}

// 保存配置
function saveConfig(config: AppConfig) {
  const appDataPath = process.env.APPDATA;
  const configPath = path.join(appDataPath, 'WorkWell', 'settings.json');
  fs.writeFileSync(configPath, JSON.stringify(config, null, 2));
}

// 加载配置
function loadConfig(): AppConfig {
  const configPath = getConfigPath();
  if (fs.existsSync(configPath)) {
    const data = fs.readFileSync(configPath, 'utf-8');
    return JSON.parse(data);
  }
  return getDefaultConfig();
}
```

#### 使用 SQLite

```csharp
// 创建数据库
using (var connection = new SQLiteConnection("Data Source=workwell.db"))
{
  connection.Open();

  // 创建设置表
  const createSettingsTable = @"
    CREATE TABLE IF NOT EXISTS Settings (
      Key TEXT PRIMARY KEY,
      Value TEXT
    );
  ";

  // 创建统计表
  const createStatsTable = @"
    CREATE TABLE IF NOT EXISTS Statistics (
      Date TEXT PRIMARY KEY,
      Pomodoros INTEGER,
      FocusMinutes INTEGER
    );
  ";

  // 保存设置
  const insertSetting = @"
    INSERT OR REPLACE INTO Settings (Key, Value)
    VALUES (@key, @value);
  ";
}
```

### 7.5 自动启动

#### Windows 注册表

```csharp
// 添加到启动项
const string keyName = @"Software\Microsoft\Windows\CurrentVersion\Run";
using (RegistryKey key = Registry.CurrentUser.OpenSubKey(keyName, true))
{
    key.SetValue("WorkWell", Application.ExecutablePath);
}

// 从启动项移除
using (RegistryKey key = Registry.CurrentUser.OpenSubKey(keyName, true))
{
    key.DeleteValue("WorkWell", false);
}
```

### 7.6 全局快捷键

#### 使用全局钩子

```csharp
// 注册全局快捷键
var hotKey = new HotKey();
hotKey.Register(Key.Escape, ModifierKeys.None, () => {
  // 阻止快捷键（在强制休息模式下）
  if (isBreakTime) {
    // 不执行任何操作
    return;
  }
});
```

**注意**: 需要使用 Windows API 钩子（SetWindowsHookEx）

---

## 8. 实现建议

### 8.1 开发阶段规划

#### Phase 1: MVP（最小可行产品）
**时间**: 1-2 周

**功能**:
- ✅ 基本番茄钟计时
- ✅ 系统托盘集成
- ✅ 简单的控制面板
- ✅ 基本统计数据
- ✅ 配置持久化

#### Phase 2: 核心功能
**时间**: 2-3 周

**功能**:
- ✅ 强制休息机制
- ✅ 全屏遮罩窗口
- ✅ 快捷键拦截
- ✅ 跳过休息功能
- ✅ 设置界面

#### Phase 3: 增强功能
**时间**: 1-2 周

**功能**:
- ✅ 声音效果
- ✅ 通知提醒
- ✅ 主题定制
- ✅ 数据导出

#### Phase 4: 优化和发布
**时间**: 1 周

**功能**:
- ✅ 性能优化
- ✅ 错误处理
- ✅ 用户文档
- ✅ 安装程序

### 8.2 关键技术难点

#### 8.2.1 快捷键拦截

**挑战**: Windows 不允许完全拦截系统级快捷键

**解决方案**:
- 使用全局钩子（SetWindowsHookEx）
- 设置窗口为 Topmost
- 监听键盘事件并阻止
- 提供紧急退出机制

**代码示例**:
```csharp
private IntPtr KeyboardHookProc(int nCode, IntPtr wParam, IntPtr lParam)
{
    if (nCode >= 0 && isBreakTime)
    {
        // 阻止特定快捷键
        var vkCode = (Keys)(lParam.ToInt32() >> 16 & 0xFFFF);
        if (vkCode == Keys.F4 && Control.ModifierKeys == Keys.Alt)
        {
            return (IntPtr)1; // 阻止
        }
    }
    return CallNextHookEx(_hookID, nCode, wParam, lParam);
}
```

#### 8.2.2 多显示器支持

**挑战**: 全屏遮罩需要覆盖所有显示器

**解决方案**:
- 获取所有显示器信息
- 为每个显示器创建遮罩窗口
- 或使用超大虚拟屏幕

**代码示例**:
```csharp
foreach (Screen screen in Screen.AllScreens)
{
    var maskWindow = new Window();
    maskWindow.Left = screen.Bounds.Left;
    maskWindow.Top = screen.Bounds.Top;
    maskWindow.Width = screen.Bounds.Width;
    maskWindow.Height = screen.Bounds.Height;
    maskWindow.Show();
}
```

#### 8.2.3 自动启动逻辑

**挑战**: 确保应用在系统启动时自动运行

**解决方案**:
- 注册表启动项
- 任务计划程序
- 启动文件夹快捷方式

### 8.3 推荐实现顺序

#### 第一步：选择技术栈
- **推荐**: WPF (.NET 6+) 或 WinUI 3
- **原因**: 原生性能好，生态系统成熟

#### 第二步：搭建项目结构
```
WorkWell.Windows/
├── src/
│   ├── WorkWell.Core/           # 核心业务逻辑
│   ├── WorkWell.UI/             # 用户界面
│   ├── WorkWell.System/         # 系统集成
│   └── WorkWell.Data/           # 数据访问
├── tests/
│   └── WorkWell.Tests/          # 单元测试
├── docs/                        # 文档
└── README.md
```

#### 第三步：实现核心功能
1. 创建主窗口和托盘图标
2. 实现基本的计时器逻辑
3. 添加配置管理
4. 实现统计功能

#### 第四步：实现强制休息
1. 创建全屏遮罩窗口
2. 实现快捷键拦截
3. 添加跳过机制

#### 第五步：完善和优化
1. 添加音效和通知
2. 优化性能
3. 添加主题支持
4. 编写文档

### 8.4 测试要点

#### 功能测试
- [ ] 计时器准确性
- [ ] 状态转换正确性
- [ ] 配置保存和加载
- [ ] 统计数据准确性

#### 界面测试
- [ ] 窗口显示正常
- [ ] 多显示器支持
- [ ] 不同 DPI 适配
- [ ] 主题切换

#### 系统集成测试
- [ ] 系统托盘功能
- [ ] 自动启动
- [ ] 快捷键拦截
- [ ] 跨版本兼容性

### 8.5 打包和分发

#### 使用 MSI 安装程序

```xml
<!-- WiX 安装脚本示例 -->
<Product Id="*"
         Name="WorkWell"
         Language="1033"
         Version="1.0.0.0"
         Manufacturer="Your Company">
  <Package InstallerVersion="200" Compressed="yes" />

  <Directory Id="TARGETDIR" Name="SourceDir">
    <Directory Id="ProgramFilesFolder">
      <Directory Id="APPLICATIONFOLDER" Name="WorkWell">
        <!-- 应用程序文件 -->
      </Directory>
    </Directory>
    <Directory Id="ProgramMenuFolder">
      <Directory Id="ApplicationProgramsFolder" Name="WorkWell"/>
    </Directory>
  </Directory>
</Product>
```

#### 使用 Inno Setup

```inno
[Setup]
AppName=WorkWell
AppVersion=1.0
DefaultDirName={pf}\WorkWell
DefaultGroupName=WorkWell

[Files]
Source: "bin\Release\*"; DestDir: "{app}"; Flags: recursesubdirs

[Icons]
Name: "{group}\WorkWell"; Filename: "{app}\WorkWell.exe"
Name: "{commonstartup}\WorkWell"; Filename: "{app}\WorkWell.exe"
```

---

## 附录

### A. 快捷键参考

| 快捷键 | 功能 | 备注 |
|--------|------|------|
| 双击托盘图标 | 打开/关闭控制面板 | - |
| 开始/暂停 | 无 | 只能通过界面操作 |
| 停止 | 无 | 只能通过界面操作 |

### B. 配置文件示例

```json
{
  "version": "1.0",
  "settings": {
    "workDuration": 1500,
    "breakDuration": 300,
    "longBreakDuration": 900,
    "longBreakInterval": 4,
    "breakIntensity": "exitable",
    "allowSkipBreak": true,
    "skipBreakRequiresReason": false,
    "skipBreakDelaySeconds": 3
  },
  "statistics": {
    "completedPomodoros": 42,
    "todayPomodoros": 5,
    "todayFocusMinutes": 125,
    "consecutiveDays": 7,
    "lastDate": "2026-03-21T12:00:00Z"
  }
}
```

### C. API 接口设计（可选）

如果未来需要 Web 版或移动版：

```typescript
// REST API 设计

GET  /api/status          // 获取当前状态
POST /api/start           // 开始工作
POST /api/pause           // 暂停
POST /api/resume          // 继续
POST /api/skip            // 跳过休息
GET  /api/settings        // 获取设置
PUT  /api/settings        // 更新设置
GET  /api/statistics      // 获取统计
```

### D. 参考资源

#### Windows 开发
- [WPF 官方文档](https://docs.microsoft.com/en-us/dotnet/desktop/wpf/)
- [WinUI 3 官方文档](https://docs.microsoft.com/en-us/windows/apps/winui/)
- [Electron 官方文档](https://www.electronjs.org/)
- [Tauri 官方文档](https://tauri.studio/)

#### 设计参考
- [Fluent Design System](https://www.microsoft.com/design/fluent/)
- [Material Design](https://material.io/design)

#### 相关项目
- [Pomatez](https://github.com/zidoro/pomatez) - 跨平台番茄钟
- [Flowtime](https://github.com/joaomdmoura/Flowtime) - macOS 番茄钟
- [Pomodoro Timer](https://github.com/Godsoul/pomodoro-timer) - Web 番茄钟

---

**文档结束**

如有疑问，请参考 macOS 原版代码或联系开发团队。

祝你开发顺利！🎉
