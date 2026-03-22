import SwiftUI
import AppKit
import AVFoundation
import Combine

class BreakWarningViewModel: ObservableObject {
    @Published var countdownSeconds: Int = 3
    var countdownTimer: Timer?
    
    func startCountdown(onFinish: @escaping () -> Void) {
        countdownSeconds = 3
        countdownTimer = Timer.scheduledTimer(withTimeInterval: 1.0, repeats: true) { [weak self] _ in
            guard let self = self else { return }
            
            if self.countdownSeconds > 0 {
                self.countdownSeconds -= 1
            } else {
                self.countdownTimer?.invalidate()
                onFinish()
            }
        }
    }
    
    func cancelCountdown() {
        print("⏹️ 取消倒计时")
        countdownTimer?.invalidate()
        countdownTimer = nil
    }
}

class BreakWarningWindow: NSWindow {
    var viewModel = BreakWarningViewModel()
    var onCancel: (() -> Void)?
    var onSkip: (() -> Void)?

    init(onCancel: @escaping () -> Void, onSkip: @escaping () -> Void) {
        super.init(
            contentRect: NSRect(x: 0, y: 0, width: 400, height: 200),
            styleMask: [.titled, .closable],
            backing: .buffered,
            defer: false
        )

        self.onCancel = onCancel
        self.onSkip = onSkip

        self.title = "休息提醒"
        self.level = .floating
        self.isOpaque = false
        self.backgroundColor = .clear
        self.center()

        let contentView = BreakWarningView(
            viewModel: viewModel,
            onCancel: { [weak self] in
                print("🔴 用户点击继续工作 - 取消倒计时")
                self?.viewModel.cancelCountdown()
                onCancel()
            },
            onSkip: { [weak self] in
                print("🟢 用户点击立即休息 - 取消倒计时")
                self?.viewModel.cancelCountdown()
                onSkip()
            }
        )
        self.contentViewController = NSHostingController(rootView: contentView)

        // 禁止关闭按钮
        self.standardWindowButton(.closeButton)?.isEnabled = false

        // 开始倒计时
        viewModel.startCountdown { [weak self] in
            print("✓ 倒计时自然结束")
            self?.viewModel.cancelCountdown()
            self?.orderOut(nil)
            onSkip()
        }
    }

    override var canBecomeKey: Bool {
        return true
    }
}

struct BreakWarningView: View {
    @ObservedObject var viewModel: BreakWarningViewModel
    let onCancel: () -> Void
    let onSkip: () -> Void

    var body: some View {
        VStack(spacing: 20) {
            Spacer()

            Image(systemName: "clock.fill")
                .font(.system(size: 50))
                .foregroundColor(.blue)

            Text("休息时间到了")
                .font(.title)
                .bold()

            Text("\(viewModel.countdownSeconds) 秒后开始休息")
                .font(.title2)
                .foregroundColor(.secondary)

            Text("请保存当前工作")
                .font(.body)
                .foregroundColor(.secondary)

            HStack(spacing: 20) {
                Button("继续工作") {
                    print("🔴 按钮点击: 继续工作")
                    onCancel()
                }
                .buttonStyle(.bordered)
                .frame(minWidth: 120)

                Button("立即休息") {
                    print("🟢 按钮点击: 立即休息")
                    onSkip()
                }
                .buttonStyle(.borderedProminent)
                .frame(minWidth: 120)
            }

            Spacer()
        }
        .padding(30)
        .frame(width: 400, height: 250)
        .background(
            RoundedRectangle(cornerRadius: 20)
                .fill(.ultraThinMaterial)
                .shadow(radius: 20)
        )
        .onAppear {
            print("⏰ 警告窗口已显示，开始倒计时")
        }
    }
}

extension Notification.Name {
    static let cancelBreak = Notification.Name("cancelBreak")
    static let skipCountdown = Notification.Name("skipCountdown")
    static let countdownFinished = Notification.Name("countdownFinished")
}
