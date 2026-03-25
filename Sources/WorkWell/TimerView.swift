import SwiftUI
import AppKit

struct TimerView: View {
    @ObservedObject var timerManager: TimerManager
    @State private var showingSettings = false
    @State private var skipReason = ""
    @State private var showingSkipAlert = false
    @State private var skipDelaySeconds: Int?

    func loadAppIcon() -> NSImage? {
        // 优先从 Bundle.main 的 Resources 目录加载（Release 版本）
        if let iconPath = Bundle.main.path(forResource: "Timeout-16", ofType: "png"),
           let icon = NSImage(contentsOfFile: iconPath) {
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
                icon.size = NSSize(width: 16, height: 16)
                icon.isTemplate = false
                return icon
            }
        }
        return nil
    }

    var body: some View {
        VStack(spacing: 20) {
            HStack {
                if let icon = loadAppIcon() {
                    Image(nsImage: icon)
                        .resizable()
                        .frame(width: 16, height: 16)
                    Text("WorkWell")
                        .font(.headline)
                } else {
                    Text("🍅 WorkWell")
                        .font(.headline)
                }
                Spacer()
                Button(action: { showingSettings = true }) {
                    Image(systemName: "gearshape")
                }
                .buttonStyle(PlainButtonStyle())
            }

            Divider()

            VStack(spacing: 15) {
                ZStack {
                    Circle()
                        .stroke(Color.gray.opacity(0.3), lineWidth: 12)
                        .frame(width: 200, height: 200)

                    Circle()
                        .trim(from: 0, to: timerManager.progress)
                        .stroke(
                            timerColor,
                            style: StrokeStyle(lineWidth: 12, lineCap: .round)
                        )
                        .frame(width: 200, height: 200)
                        .rotationEffect(.degrees(-90))

                    VStack(spacing: 2) {
                        Text(timerDisplay)
                            .font(.system(size: 36, weight: .bold, design: .monospaced))
                            .foregroundColor(timerColor)
                            .monospacedDigit()

                        Text(statusText)
                            .font(.caption)
                            .foregroundColor(.secondary)
                    }
                }
            }

            HStack(spacing: 10) {
                switch timerManager.currentState {
                case .idle:
                    Button("开始工作") {
                        timerManager.startWork()
                    }
                    .buttonStyle(.borderedProminent)
                    .frame(minWidth: 100)

                case .working:
                    Button("暂停") {
                        timerManager.pause()
                    }
                    .buttonStyle(.bordered)
                    .frame(minWidth: 100)

                    Button("休息") {
                        timerManager.breakNow()
                    }
                    .buttonStyle(.bordered)
                    .frame(minWidth: 100)

                case .paused:
                    Button("继续") {
                        timerManager.resume()
                    }
                    .buttonStyle(.borderedProminent)
                    .frame(minWidth: 100)

                    Button("停止") {
                        timerManager.currentState = .idle
                    }
                    .buttonStyle(.bordered)
                    .frame(minWidth: 100)

                case .breaking:
                    if timerManager.allowSkipBreak {
                        Button("跳过休息") {
                            if timerManager.skipBreakRequiresReason {
                                skipReason = ""
                                showingSkipAlert = true
                            } else if timerManager.skipBreakDelaySeconds > 0 {
                                skipDelaySeconds = timerManager.skipBreakDelaySeconds
                                startSkipDelay()
                            } else {
                                timerManager.skipBreak()
                            }
                        }
                        .buttonStyle(.bordered)
                        .frame(minWidth: 100)
                    } else {
                        Text("休息中...")
                            .foregroundColor(.secondary)
                            .frame(minWidth: 100)
                    }

                case .skipRequested:
                    if let delay = skipDelaySeconds {
                        Text("请等待 \(delay) 秒...")
                            .foregroundColor(.secondary)
                            .frame(minWidth: 150)
                    }

                default:
                    EmptyView()
                }
            }
            .frame(height: 30)

            Divider()

            VStack(alignment: .leading, spacing: 10) {
                HStack {
                    Text("今日番茄:")
                    Spacer()
                    Text("\(timerManager.todayPomodoros)")
                        .bold()
                }

                HStack {
                    Text("专注时长:")
                    Spacer()
                    Text("\(formatMinutes(timerManager.todayFocusMinutes))")
                        .bold()
                }

                HStack {
                    Text("连续天数:")
                    Spacer()
                    Text("\(timerManager.consecutiveDays) 天")
                        .bold()
                }
            }
            .font(.subheadline)

            Spacer()
        }
        .padding(20)
            .background(Color(nsColor: .windowBackgroundColor))
            .cornerRadius(10)
        .frame(width: 300, height: 450)
        .sheet(isPresented: $showingSettings) {
            SettingsView(timerManager: timerManager)
        }
        .alert("跳过休息原因", isPresented: $showingSkipAlert) {
            TextField("请输入原因", text: $skipReason)
            Button("确定") {
                timerManager.skipBreak(reason: skipReason.isEmpty ? "未填写" : skipReason)
                skipReason = ""
            }
            Button("取消", role: .cancel) {}
        } message: {
            Text("请说明为什么要跳过这次休息")
        }
    }

    private var timerDisplay: String {
        switch timerManager.currentState {
        case .idle:
            return formatTime(timerManager.workDuration)
        default:
            return timerManager.remainingTimeString
        }
    }

    private var statusText: String {
        switch timerManager.currentState {
        case .idle:
            return "准备开始"
        case .working:
            return "工作中..."
        case .paused:
            return "已暂停"
        case .breaking:
            return "休息时间"
        case .skipRequested:
            return "等待确认..."
        }
    }

    private var timerColor: Color {
        switch timerManager.currentState {
        case .idle:
            return .primary
        case .working, .paused:
            // 使用黑色，简洁清晰
            return .black
        case .breaking:
            return .green
        case .skipRequested:
            return .red
        }
    }

    private func formatMinutes(_ minutes: Int) -> String {
        let hours = minutes / 60
        let mins = minutes % 60
        if hours > 0 {
            return "\(hours)小时\(mins)分钟"
        }
        return "\(mins)分钟"
    }

    private func formatTime(_ seconds: TimeInterval) -> String {
        let minutes = Int(seconds) / 60
        let secs = Int(seconds) % 60
        return String(format: "%02d:%02d", minutes, secs)
    }

    private func startSkipDelay() {
        Timer.scheduledTimer(withTimeInterval: 1.0, repeats: true) { timer in
            if let delay = skipDelaySeconds, delay > 0 {
                skipDelaySeconds = delay - 1
            } else {
                timer.invalidate()
                timerManager.skipBreak()
                skipDelaySeconds = nil
            }
        }
    }
}

struct SettingsView: View {
    @ObservedObject var timerManager: TimerManager
    @Environment(\.dismiss) private var dismiss

    var body: some View {
        VStack(alignment: .leading, spacing: 20) {
            Text("设置")
                .font(.headline)

            Group {
                Text("时间设置")
                    .font(.subheadline)
                    .foregroundColor(.secondary)

                HStack {
                    Text("工作时长:")
                    Spacer()
                    Stepper("\(Int(timerManager.workDuration / 60)) 分钟",
                            value: $timerManager.workDuration,
                            in: 60...3600,
                            step: 60)
                }

                HStack {
                    Text("短休息:")
                    Spacer()
                    Stepper("\(Int(timerManager.breakDuration / 60)) 分钟",
                            value: $timerManager.breakDuration,
                            in: 60...1800,
                            step: 60)
                }

                HStack {
                    Text("长休息:")
                    Spacer()
                    Stepper("\(Int(timerManager.longBreakDuration / 60)) 分钟",
                            value: $timerManager.longBreakDuration,
                            in: 300...3600,
                            step: 60)
                }

                HStack {
                    Text("长休息间隔:")
                    Spacer()
                    Stepper("\(timerManager.longBreakInterval) 个番茄",
                            value: $timerManager.longBreakInterval,
                            in: 2...10)
                }
            }

            Divider()

            Group {
                Text("休息设置")
                    .font(.subheadline)
                    .foregroundColor(.secondary)

                Picker("休息模式", selection: $timerManager.breakIntensity) {
                    ForEach(BreakIntensity.allCases, id: \.self) { intensity in
                        VStack(alignment: .leading) {
                            Text(intensity.rawValue)
                            Text(intensity.description)
                                .font(.caption)
                                .foregroundColor(.secondary)
                        }
                        .tag(intensity)
                    }
                }

                Toggle("休息结束后自动开始工作", isOn: $timerManager.autoStartWorkAfterBreak)
                    .help("启用后，休息时间结束时会自动开始下一个工作周期")
            }

            Divider()

            HStack {
                Spacer()
                Button("完成") {
                    dismiss()
                }
                .buttonStyle(.borderedProminent)
            }
        }
        .padding(20)
        .frame(width: 400)
    }
}
