import SwiftUI
import AppKit

@main
struct WorkWellApp: App {
    @NSApplicationDelegateAdaptor(AppDelegate.self) var appDelegate

    var body: some Scene {
        Settings {
            EmptyView()
        }
    }
}

class AppDelegate: NSObject, NSApplicationDelegate {
    var statusItem: NSStatusItem?
    var timerManager = TimerManager()
    var popover: NSPopover?
    var breakWindow: BreakWindow?
    var warningWindow: BreakWarningWindow?
    var isClosingBreakWindow: Bool = false

    func loadStatusIcon() -> NSImage? {
        // 优先从 Bundle.main 的 Resources 目录加载（Release 版本）
        if let iconPath = Bundle.main.path(forResource: "Timeout-16", ofType: "png"),
           let icon = NSImage(contentsOfFile: iconPath) {
            print("✅ 图标加载成功（从 Resources）: \(iconPath)")
            icon.size = NSSize(width: 16, height: 16)
            icon.isTemplate = false
            return icon
        }

        // 备选：从 Debug bundle 加载
        let bundlePaths = [
            Bundle.main.bundleURL.appendingPathComponent("WorkWell_WorkWell.bundle").path,
            "/Users/richard/Documents/timeout/.build/arm64-apple-macosx/debug/WorkWell_WorkWell.bundle"
        ]

        for bundlePath in bundlePaths {
            if let bundle = Bundle(path: bundlePath),
               let iconPath = bundle.path(forResource: "Timeout-16", ofType: "png"),
               let icon = NSImage(contentsOfFile: iconPath) {
                print("✅ 图标加载成功（从 Bundle）: \(iconPath)")
                icon.size = NSSize(width: 16, height: 16)
                icon.isTemplate = false
                return icon
            }
        }

        print("⚠️ 图标加载失败")
        return nil
    }

    func createAttributedTimeString(_ timeString: String) -> NSAttributedString {
        // 使用系统默认颜色（自动适应深色/浅色模式）
        let attributes: [NSAttributedString.Key: Any] = [
            .font: NSFont.monospacedSystemFont(ofSize: 13, weight: .medium)
        ]
        return NSAttributedString(string: timeString, attributes: attributes)
    }

    func applicationDidFinishLaunching(_ notification: Notification) {
        print("✅ 应用启动成功")
        NSApp.setActivationPolicy(.accessory)

        statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)
        
        if let button = statusItem?.button {
            button.title = ""
            
            if let icon = loadStatusIcon() {
                button.image = icon
            } else {
                button.title = "🍅"
            }
            
            button.action = #selector(togglePopover)
            button.target = self
        }

        popover = NSPopover()
        popover?.contentSize = NSSize(width: 300, height: 400)
        popover?.behavior = .transient
        popover?.contentViewController = NSHostingController(rootView: TimerView(timerManager: timerManager))

        NotificationCenter.default.addObserver(
            forName: .breakDidEnd,
            object: nil,
            queue: .main
        ) { [weak self] _ in
            print("🔔 收到休息结束通知")
            self?.handleBreakEnd()
        }

        NotificationCenter.default.addObserver(
            forName: .showBreakWarning,
            object: nil,
            queue: .main
        ) { [weak self] _ in
            print("⚠️ 收到显示警告通知")
            self?.showBreakWarning()
        }

        timerManager.onTick = { [weak self] timeString, isActive in
            guard let button = self?.statusItem?.button else { return }
            
            // 工作状态时显示倒计时，其他状态显示图标
            if self?.timerManager.currentState == .working && isActive {
                // 显示带颜色的倒计时时间（柔和的蓝色）
                button.image = nil
                button.attributedTitle = self?.createAttributedTimeString(timeString) ?? NSAttributedString(string: timeString)
            } else {
                // 显示图标
                if let icon = self?.loadStatusIcon() {
                    button.title = ""
                    button.image = icon
                } else {
                    button.image = nil
                    button.title = "🍅"
                }
            }
        }
    }

    func applicationShouldOpenUntitledFile(_ sender: NSApplication) -> Bool {
        print("🚫 阻止自动打开空窗口")
        return false
    }

    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        print("🚪 应用收到终止询问 - 返回 false")
        return false
    }

    func applicationWillTerminate(_ notification: Notification) {
        print("💀 应用即将终止")
    }

    @objc func togglePopover() {
        if let popover = popover {
            if popover.isShown {
                popover.performClose(nil)
            } else {
                if let button = statusItem?.button {
                    popover.show(relativeTo: button.bounds, of: button, preferredEdge: .minY)
                }
            }
        }
    }

    func showBreakWarning() {
        print("📢 显示警告窗口")
        guard warningWindow == nil else { return }
        
        warningWindow = BreakWarningWindow(
            onCancel: { [weak self] in
                print("✓ 用户点击继续工作")
                self?.handleCancelBreak()
            },
            onSkip: { [weak self] in
                print("✓ 用户点击立即休息")
                self?.handleSkipCountdown()
            }
        )
        warningWindow?.makeKeyAndOrderFront(nil)
        NSApp.activate(ignoringOtherApps: true)
    }

    func showBreakScreen() {
        print("💤 显示休息界面，强度：\(timerManager.breakIntensity.rawValue)")
        let isLongBreak = timerManager.completedPomodoros % timerManager.longBreakInterval == 0
        let breakDuration = isLongBreak ? timerManager.longBreakDuration : timerManager.breakDuration
        
        // 传递用户设置的强度
        breakWindow = BreakWindow(breakDuration: breakDuration, intensity: timerManager.breakIntensity)

        breakWindow?.makeKeyAndOrderFront(nil)
        NSApp.activate(ignoringOtherApps: true)
    }

    func hideBreakScreen() {
        print("🙈 隐藏休息界面")
        guard let window = breakWindow, !isClosingBreakWindow else { return }
        isClosingBreakWindow = true
        
        // 立即隐藏窗口，不使用 close() 的动画
        window.orderOut(nil)
        breakWindow = nil
        isClosingBreakWindow = false
    }

    func handleBreakEnd() {
        print("🏁 处理休息结束")
        // 先恢复状态，再关闭窗口
        timerManager.resumeFromBreak()
        hideBreakScreen()
    }

    func handleCancelBreak() {
        print("↩️ 处理取消休息 - 开始")
        // 先取消休息，恢复计时器
        timerManager.cancelBreak()
        
        // 延迟关闭窗口，避免在回调中直接关闭
        DispatchQueue.main.async { [weak self] in
            print("↩️ 关闭警告窗口")
            self?.warningWindow?.orderOut(nil)
            self?.warningWindow = nil
            print("↩️ 处理取消休息 - 完成")
        }
    }

    func handleSkipCountdown() {
        print("⏩ 处理跳过倒计时")
        warningWindow?.orderOut(nil)
        warningWindow = nil
        
        if timerManager.currentState == .working {
            timerManager.startBreakNow()
        } else {
            timerManager.startBreak()
        }
        showBreakScreen()
    }

    deinit {
        NotificationCenter.default.removeObserver(self)
    }
}
