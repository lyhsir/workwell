import SwiftUI
import AppKit
import AVFoundation
import Combine

extension Notification.Name {
    static let breakDidEnd = Notification.Name("breakDidEnd")
    static let showBreakWarning = Notification.Name("showBreakWarning")
    static let mouseMovedNotification = Notification.Name("mouseMovedNotification")
}

class BreakWindow: NSWindow {
    private var eventMonitor: Any?
    
    init(breakDuration: TimeInterval, intensity: BreakIntensity = .exitable) {
        super.init(
            contentRect: NSScreen.main?.frame ?? NSRect(x: 0, y: 0, width: 1920, height: 1080),
            styleMask: [.borderless, .fullSizeContentView],
            backing: .buffered,
            defer: false
        )

        guard let screenFrame = NSScreen.main?.frame else { return }

        self.level = .screenSaver + 1
        self.isOpaque = false
        self.backgroundColor = .clear
        self.hasShadow = false
        self.collectionBehavior = [.fullScreenAuxiliary, .canJoinAllSpaces, .ignoresCycle, .stationary, .fullScreenAllowsTiling]

        self.setFrame(screenFrame, display: true, animate: false)
        self.setFrameOrigin(NSPoint(x: 0, y: 0))

        let contentView = BreakScreenView(breakDuration: breakDuration, intensity: intensity)
            .frame(width: screenFrame.width, height: screenFrame.height)

        self.contentViewController = NSHostingController(rootView: contentView)

        self.isMovable = false
        self.isMovableByWindowBackground = false
        standardWindowButton(.closeButton)?.isHidden = true
        standardWindowButton(.miniaturizeButton)?.isHidden = true
        standardWindowButton(.zoomButton)?.isHidden = true

        // 监听鼠标移动事件（仅在可退出模式下需要）
        eventMonitor = NSEvent.addLocalMonitorForEvents(matching: .mouseMoved) { [weak self] event in
            if self?.isVisible == true {
                NotificationCenter.default.post(name: .mouseMovedNotification, object: nil)
            }
            return event
        }

        playNotificationSound()
    }
    
    deinit {
        if let monitor = eventMonitor {
            NSEvent.removeMonitor(monitor)
        }
    }

    override func close() {
        // 淡出效果 - 2秒
        NSAnimationContext.runAnimationGroup { context in
            context.duration = 2.0
            context.timingFunction = CAMediaTimingFunction(name: .easeInEaseOut)
            self.animator().alphaValue = 0.0
        } completionHandler: {
            super.close()
        }
    }

    private func playNotificationSound() {
        NSSound.beep()
    }

    override var canBecomeKey: Bool {
        return true
    }

    override var canBecomeMain: Bool {
        return true
    }

    override func keyDown(with event: NSEvent) {
        let modifiers = event.modifierFlags.intersection(.deviceIndependentFlagsMask)

        if modifiers.contains(.command) {
            switch event.charactersIgnoringModifiers {
            case "w", "q", "W", "Q":
                return
            default:
                break
            }
        }

        if event.keyCode == 53 {
            return
        }

        super.keyDown(with: event)
    }

    override var acceptsFirstResponder: Bool {
        return true
    }
}

struct BreakScreenView: View {
    let breakDuration: TimeInterval
    let intensity: BreakIntensity
    @State private var currentTime: TimeInterval = 0
    @State private var breathingPhase: Double = 0
    @State private var stretchIndex: Int = 0
    @State private var showingTip = true
    @State private var backgroundOpacity: Double = 0.0
    @State private var contentOpacity: Double = 0.0
    @State private var scale: CGFloat = 0.8
    @State private var showButton: Bool = false  // 控制按钮显示/隐藏
    @State private var hideTimer: Timer?
    @State private var lastMouseMoveTime: Date = Date()

    private let stretchTips = [
        "💧 喝杯水，保持水分",
        "👀 远眺 20 英尺外物体 20 秒",
        "🤲 揉搓双手，促进血液循环",
        "🧘 深呼吸：吸气 4 秒，屏息 7 秒，呼气 8 秒",
        "🦒 转动颈部，缓解颈椎压力",
        "🤏 手腕旋转，预防鼠标手",
        "🚶 站起来走动几分钟",
        "🙆 举手伸懒腰，放松肩背",
        "👂 按摩耳朵，促进头部血液循环",
        "👃 闭眼深呼吸，让眼睛休息"
    ]

    var body: some View {
        GeometryReader { geometry in
            ZStack {
                // 背景层 - 先淡入（2秒）
                backgroundView
                    .frame(width: geometry.size.width, height: geometry.size.height)
                    .ignoresSafeArea(.all)
                    .opacity(backgroundOpacity)
                    .animation(.easeInOut(duration: 2.0), value: backgroundOpacity)

                VStack(spacing: 40) {
                    Spacer()

                    // 圆形进度条
                    ZStack {
                        Circle()
                            .stroke(Color.white.opacity(0.3 * contentOpacity), lineWidth: 20)
                            .frame(width: 350, height: 350)

                        Circle()
                            .trim(from: 0, to: progress)
                            .stroke(
                                AngularGradient(
                                    gradient: Gradient(colors: [.blue, .green, .blue]),
                                    center: .center,
                                    startAngle: .degrees(0),
                                    endAngle: .degrees(360)
                                ),
                                style: StrokeStyle(lineWidth: 20, lineCap: .round)
                            )
                            .frame(width: 350, height: 350)
                            .rotationEffect(.degrees(-90))
                            .shadow(color: .blue.opacity(0.5 * contentOpacity), radius: 10)

                        VStack(spacing: 8) {
                            Text("休息时间")
                                .font(.title2)
                                .foregroundColor(.white)
                                .fontWeight(.medium)
                                .shadow(color: .black.opacity(0.5), radius: 3)

                            Text(formatTime(currentTime))
                                .font(.system(size: 72, weight: .bold, design: .monospaced))
                                .foregroundColor(.white)
                                .shadow(color: .black.opacity(0.5), radius: 3)
                        }
                    }
                    .padding(.bottom, 20)
                    .scaleEffect(scale)
                    .opacity(contentOpacity)

                    // 休息建议
                    VStack(spacing: 15) {
                        HStack(spacing: 10) {
                            Image(systemName: "lightbulb.fill")
                                .foregroundColor(.yellow)
                                .shadow(color: .black.opacity(0.3), radius: 2)
                            Text("休息建议")
                                .font(.headline)
                                .foregroundColor(.white)
                                .shadow(color: .black.opacity(0.5), radius: 2)
                        }

                        Text(stretchTips[stretchIndex])
                            .font(.title3)
                            .foregroundColor(.white)
                            .multilineTextAlignment(.center)
                            .padding()
                            .background(
                                RoundedRectangle(cornerRadius: 15)
                                    .fill(.ultraThinMaterial)
                                    .shadow(color: .black.opacity(0.2), radius: 5)
                            )
                            .shadow(color: .black.opacity(0.3), radius: 5)
                            .onTapGesture {
                                nextTip()
                            }
                    }
                    .padding(.horizontal, 40)
                    .opacity(contentOpacity)

                    Spacer()

                    // 底部按钮区域 - 固定高度避免布局跳动
                    bottomButtons
                        .frame(height: 60) // 固定高度
                }
            }
        }
        .onAppear {
            currentTime = breakDuration
            showButton = false  // 确保按钮默认隐藏
            startTimer()
            rotateTips()
            
            // 分阶段动画 - 总共2秒
            withAnimation(.easeInOut(duration: 2.0)) {
                backgroundOpacity = 1.0
            }
            
            // 延迟0.5秒后，内容开始淡入（持续1.5秒）
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
                withAnimation(.easeOut(duration: 1.5)) {
                    contentOpacity = 1.0
                    scale = 1.0
                }
            }
        }
        .onDisappear {
            // 重置状态
            backgroundOpacity = 0.0
            contentOpacity = 0.0
            scale = 0.8
            hideTimer?.invalidate()
        }
        .onReceive(NotificationCenter.default.publisher(for: .mouseMovedNotification)) { _ in
            handleMouseMove()
        }
    }

    private func handleMouseMove() {
        // 仅在可退出模式下启用
        guard intensity == .exitable else { return }
        
        lastMouseMoveTime = Date()
        
        // 显示按钮
        if !showButton {
            withAnimation(.easeInOut(duration: 0.3)) {
                showButton = true
            }
        }
        
        // 取消之前的计时器
        hideTimer?.invalidate()
        
        // 设置新的计时器，10秒后隐藏按钮
        hideTimer = Timer.scheduledTimer(withTimeInterval: 10.0, repeats: false) { _ in
            let timeSinceLastMove = Date().timeIntervalSince(lastMouseMoveTime)
            if timeSinceLastMove >= 10.0 {
                withAnimation(.easeOut(duration: 0.5)) {
                    showButton = false
                }
            }
        }
    }

    @ViewBuilder
    private var bottomButtons: some View {
        if intensity == .exitable {
            // 可退出模式：鼠标移动时显示"返回工作"按钮
            HStack(spacing: 30) {
                Spacer()
                
                Button("返回工作") {
                    NotificationCenter.default.post(name: .breakDidEnd, object: nil)
                }
                .buttonStyle(.borderedProminent)
                .font(.headline)
                .frame(minWidth: 150)
                .opacity(showButton ? 1.0 : 0.0) // 只改变透明度，不改变布局
                .animation(.easeInOut(duration: 0.3), value: showButton)
                
                Spacer()
            }
            .padding(.bottom, 30)
            .opacity(contentOpacity)
        } else {
            // 不可退出模式：显示提示文字
            VStack(spacing: 10) {
                Text("⚠️ 强制休息模式")
                    .font(.caption)
                    .foregroundColor(.white)
                    .shadow(color: .black.opacity(0.5), radius: 2)
                Text("休息时间结束后才能继续工作")
                    .font(.caption)
                    .foregroundColor(.white.opacity(0.8))
                    .shadow(color: .black.opacity(0.5), radius: 2)
            }
            .padding(.bottom, 20)
            .opacity(contentOpacity)
        }
    }

    private var progress: Double {
        guard breakDuration > 0 else { return 0 }
        return currentTime / breakDuration
    }

    private var backgroundView: some View {
        Group {
            if intensity == .exitable {
                // 可退出：蓝绿色渐变，清新
                ZStack {
                    LinearGradient(
                        colors: [Color.blue.opacity(0.7), Color.green.opacity(0.7)],
                        startPoint: .topLeading,
                        endPoint: .bottomTrailing
                    )
                    
                    // 添加一些装饰性圆圈
                    Circle()
                        .fill(
                            RadialGradient(
                                colors: [Color.blue.opacity(0.1), Color.clear],
                                center: .center,
                                startRadius: 50,
                                endRadius: 300
                            )
                        )
                        .frame(width: 400, height: 400)
                        .position(x: 200, y: 200)
                }
            } else {
                // 不可退出：深色背景，严肃
                Color.black.opacity(0.9)
                    .overlay(
                        VStack {
                            Text("🛑")
                                .font(.system(size: 80))
                            Text("强制休息时间")
                                .font(.largeTitle)
                                .foregroundColor(.white)
                                .bold()
                            Text("请放下鼠标和键盘，让身体放松")
                                .font(.title2)
                                .foregroundColor(.white.opacity(0.8))
                        }
                        .opacity(0.3)
                    )
            }
        }
    }

    private func formatTime(_ seconds: TimeInterval) -> String {
        let fromMinutes = Int(seconds) / 60
        let fromSecs = Int(seconds) % 60
        return String(format: "%02d:%02d", fromMinutes, fromSecs)
    }

    private func startTimer() {
        Timer.scheduledTimer(withTimeInterval: 1.0, repeats: true) { timer in
            if currentTime > 0 {
                currentTime -= 1
            } else {
                timer.invalidate()
                NotificationCenter.default.post(name: .breakDidEnd, object: nil)
            }
        }
    }

    private func startBreathingAnimation() {
        withAnimation {
            breathingPhase = 1.0
        }
    }

    private func nextTip() {
        stretchIndex = (stretchIndex + 1) % stretchTips.count
    }

    private func rotateTips() {
        Timer.scheduledTimer(withTimeInterval: 3.0, repeats: true) { _ in
            nextTip()
        }
    }
}
