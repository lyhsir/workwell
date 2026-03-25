import Foundation
import Combine
import AppKit

enum TimerState {
    case idle
    case working
    case paused
    case breaking
    case skipRequested
}

enum BreakIntensity: String, CaseIterable {
    case exitable = "可退出"
    case forced = "不可退出"

    var description: String {
        switch self {
        case .exitable: return "可以随时退出休息"
        case .forced: return "必须休息完整时间"
        }
    }
}

class TimerManager: ObservableObject {
    @Published var workDuration: TimeInterval = 25 * 60
    @Published var breakDuration: TimeInterval = 5 * 60
    @Published var longBreakDuration: TimeInterval = 15 * 60
    @Published var longBreakInterval: Int = 4

    @Published var currentState: TimerState = .idle
    @Published var currentTime: TimeInterval = 0
    @Published var completedPomodoros: Int = 0  // 累计总番茄数
    @Published var todayPomodoros: Int = 0      // 今日番茄数
    @Published var todayFocusMinutes: Int = 0
    @Published var consecutiveDays: Int = 1

    @Published var breakIntensity: BreakIntensity = .exitable
    @Published var allowSkipBreak: Bool = false
    @Published var skipBreakRequiresReason: Bool = false
    @Published var skipBreakDelaySeconds: Int = 0
    @Published var autoStartWorkAfterBreak: Bool = false  // 新增：休息结束后自动开始工作

    private var timer: Timer?
    private var startTime: Date?
    private var savedTimeBeforeBreak: TimeInterval = 0
    private var pendingBreak: Bool = false

    var onBreakStart: (() -> Void)?
    var onWorkStart: (() -> Void)?
    var onTick: ((String, Bool) -> Void)?
    var onComplete: (() -> Void)?

    private var cancellables = Set<AnyCancellable>()

    init() {
        loadSettings()
        loadStats()
        setupAutoSave()
    }

    private func setupAutoSave() {
        $workDuration
            .debounce(for: .milliseconds(500), scheduler: RunLoop.main)
            .sink { [weak self] _ in self?.saveSettings() }
            .store(in: &cancellables)

        $breakDuration
            .debounce(for: .milliseconds(500), scheduler: RunLoop.main)
            .sink { [weak self] _ in self?.saveSettings() }
            .store(in: &cancellables)

        $longBreakDuration
            .debounce(for: .milliseconds(500), scheduler: RunLoop.main)
            .sink { [weak self] _ in self?.saveSettings() }
            .store(in: &cancellables)

        $longBreakInterval
            .debounce(for: .milliseconds(500), scheduler: RunLoop.main)
            .sink { [weak self] _ in self?.saveSettings() }
            .store(in: &cancellables)

        $breakIntensity
            .debounce(for: .milliseconds(500), scheduler: RunLoop.main)
            .sink { [weak self] _ in self?.saveSettings() }
            .store(in: &cancellables)

        $allowSkipBreak
            .debounce(for: .milliseconds(500), scheduler: RunLoop.main)
            .sink { [weak self] _ in self?.saveSettings() }
            .store(in: &cancellables)

        $skipBreakRequiresReason
            .debounce(for: .milliseconds(500), scheduler: RunLoop.main)
            .sink { [weak self] _ in self?.saveSettings() }
            .store(in: &cancellables)

        $skipBreakDelaySeconds
            .debounce(for: .milliseconds(500), scheduler: RunLoop.main)
            .sink { [weak self] _ in self?.saveSettings() }
            .store(in: &cancellables)

        $autoStartWorkAfterBreak
            .debounce(for: .milliseconds(500), scheduler: RunLoop.main)
            .sink { [weak self] _ in self?.saveSettings() }
            .store(in: &cancellables)
    }

    private func saveSettings() {
        let defaults = UserDefaults.standard
        defaults.set(workDuration, forKey: "workDuration")
        defaults.set(breakDuration, forKey: "breakDuration")
        defaults.set(longBreakDuration, forKey: "longBreakDuration")
        defaults.set(longBreakInterval, forKey: "longBreakInterval")
        defaults.set(breakIntensity.rawValue, forKey: "breakIntensity")
        defaults.set(allowSkipBreak, forKey: "allowSkipBreak")
        defaults.set(skipBreakRequiresReason, forKey: "skipBreakRequiresReason")
        defaults.set(skipBreakDelaySeconds, forKey: "skipBreakDelaySeconds")
        defaults.set(autoStartWorkAfterBreak, forKey: "autoStartWorkAfterBreak")
    }

    private func loadSettings() {
        let defaults = UserDefaults.standard
        workDuration = defaults.double(forKey: "workDuration") != 0 ? defaults.double(forKey: "workDuration") : 25 * 60
        breakDuration = defaults.double(forKey: "breakDuration") != 0 ? defaults.double(forKey: "breakDuration") : 5 * 60
        longBreakDuration = defaults.double(forKey: "longBreakDuration") != 0 ? defaults.double(forKey: "longBreakDuration") : 15 * 60
        longBreakInterval = defaults.object(forKey: "longBreakInterval") as? Int ?? 4

        if let intensityString = defaults.string(forKey: "breakIntensity"),
           let intensity = BreakIntensity(rawValue: intensityString) {
            breakIntensity = intensity
        }

        allowSkipBreak = defaults.bool(forKey: "allowSkipBreak")
        skipBreakRequiresReason = defaults.bool(forKey: "skipBreakRequiresReason")
        skipBreakDelaySeconds = defaults.integer(forKey: "skipBreakDelaySeconds")
        autoStartWorkAfterBreak = defaults.bool(forKey: "autoStartWorkAfterBreak")
    }

    func startWork() {
        currentState = .working
        currentTime = workDuration
        startTime = Date()
        onWorkStart?()
        startTimer()
    }

    func pause() {
        guard currentState == .working else { return }
        currentState = .paused
        stopTimer()
    }

    func resume() {
        guard currentState == .paused else { return }
        currentState = .working
        startTime = Date().addingTimeInterval(-currentTime + 1)
        startTimer()
    }

    func skipBreak(reason: String? = nil) {
        guard currentState == .breaking else { return }

        if skipBreakRequiresReason && reason == nil {
            return
        }

        if let reason = reason {
            logSkipReason(reason)
        }

        currentState = .idle
        stopTimer()
        onWorkStart?()
        startWork()
    }

    func breakNow() {
        // 立即休息 - 暂停工作，保存当前进度
        guard currentState == .working else { return }

        print("📥 breakNow: 保存当前时间 \(currentTime)")
        savedTimeBeforeBreak = currentTime
        pendingBreak = true

        // 暂停计时器，但保持工作状态
        stopTimer()

        // 发送通知显示警告窗口，但不改变状态
        NotificationCenter.default.post(name: .showBreakWarning, object: nil)
    }

    func startBreakNow() {
        // 从警告窗口确认开始休息
        print("🌙 startBreakNow: 开始休息")
        guard currentState == .working else { return }

        currentState = .breaking

        // 计算休息时长
        let isLongBreak = completedPomodoros % longBreakInterval == 0
        currentTime = isLongBreak ? longBreakDuration : breakDuration

        onBreakStart?()
        startTimer()
    }

    func cancelBreak() {
        // 取消休息，恢复工作
        print("↩️ cancelBreak: 恢复工作，savedTime=\(savedTimeBeforeBreak)")
        pendingBreak = false

        // 恢复计时器
        if currentState == .working && savedTimeBeforeBreak > 0 {
            currentTime = savedTimeBeforeBreak
            savedTimeBeforeBreak = 0
            startTimer()
            print("✓ 已恢复工作计时")
        }
    }

    private func startTimer() {
        timer = Timer.scheduledTimer(withTimeInterval: 1.0, repeats: true) { [weak self] _ in
            self?.tick()
        }
    }

    private func stopTimer() {
        timer?.invalidate()
        timer = nil
    }

    private func tick() {
        guard currentState == .working || currentState == .breaking else { return }

        currentTime -= 1

        let timeString = formatTime(currentTime)
        onTick?(timeString, true)

        if currentTime <= 0 {
            timerComplete()
        }
    }

    private func timerComplete() {
        stopTimer()

        if currentState == .working {
            completedPomodoros += 1
            todayPomodoros += 1
            todayFocusMinutes += Int(workDuration / 60)
            saveStats()
            onComplete?()
            // 发送通知显示警告窗口
            NotificationCenter.default.post(name: .showBreakWarning, object: nil)
        } else if currentState == .breaking {
            onTick?("休息结束", false)
        }
    }

    func startBreak() {
        // 从警告窗口确认开始休息（工作完成后的休息）
        print("🌙 startBreak: 开始休息（工作已完成）")
        currentState = .breaking

        let isLongBreak = completedPomodoros % longBreakInterval == 0
        currentTime = isLongBreak ? longBreakDuration : breakDuration

        onBreakStart?()
        startTimer()
    }

    func resumeFromBreak() {
        // 从休息恢复
        guard currentState == .breaking else { return }

        print("🔙 resumeFromBreak: 从休息恢复，autoStartWorkAfterBreak=\(autoStartWorkAfterBreak)")
        stopTimer()

        // 恢复之前的工作时间（如果有的话）
        if savedTimeBeforeBreak > 0 {
            // 中断工作后休息，恢复工作
            currentState = .working
            currentTime = savedTimeBeforeBreak
            savedTimeBeforeBreak = 0
            onWorkStart?()
            startTimer()
        } else if autoStartWorkAfterBreak {
            // 用户启用了自动开始，直接开始新的工作
            print("✅ 自动开始新的工作周期")
            startWork()
        } else {
            // 正常完成工作后休息，回到空闲状态等待用户操作
            currentState = .idle
            onTick?("🍅", false)
        }
    }

    private func formatTime(_ seconds: TimeInterval) -> String {
        let minutes = Int(seconds) / 60
        let secs = Int(seconds) % 60
        return String(format: "%02d:%02d", minutes, secs)
    }

    private func loadStats() {
        let defaults = UserDefaults.standard
        completedPomodoros = defaults.integer(forKey: "completedPomodoros")
        todayPomodoros = defaults.integer(forKey: "todayPomodoros")
        todayFocusMinutes = defaults.integer(forKey: "todayFocusMinutes")
        consecutiveDays = defaults.integer(forKey: "consecutiveDays")

        if let lastDate = defaults.object(forKey: "lastDate") as? Date {
            let calendar = Calendar.current
            if !calendar.isDateInToday(lastDate) {
                let daysBetween = calendar.dateComponents([.day], from: lastDate, to: Date()).day ?? 0
                if daysBetween == 1 {
                    consecutiveDays += 1
                } else if daysBetween > 1 {
                    consecutiveDays = 1
                }
                // 新的一天，重置今日统计
                todayPomodoros = 0
                todayFocusMinutes = 0
                defaults.set(Date(), forKey: "lastDate")
            }
        } else {
            defaults.set(Date(), forKey: "lastDate")
        }
    }

    private func saveStats() {
        let defaults = UserDefaults.standard
        defaults.set(completedPomodoros, forKey: "completedPomodoros")
        defaults.set(todayPomodoros, forKey: "todayPomodoros")
        defaults.set(todayFocusMinutes, forKey: "todayFocusMinutes")
        defaults.set(consecutiveDays, forKey: "consecutiveDays")
        defaults.set(Date(), forKey: "lastDate")
    }

    private func logSkipReason(_ reason: String) {
        print("Break skipped. Reason: \(reason)")
    }

    var remainingTimeString: String {
        return formatTime(currentTime)
    }

    var progress: Double {
        switch currentState {
        case .working, .paused:
            return 1.0 - (currentTime / workDuration)
        case .breaking:
            let duration = (completedPomodoros % longBreakInterval == 0) ? longBreakDuration : breakDuration
            return 1.0 - (currentTime / duration)
        default:
            return 0
        }
    }
}
