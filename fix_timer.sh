#!/bin/bash
# 修复TimerManager.swift中的换行符问题
sed -i '' '/onComplete?()/a\
            NotificationCenter.default.post(name: .showBreakWarning, object: nil)' /Users/richard/Documents/timeout/Sources/Timeout/TimerManager.swift
sed -i '' '/onComplete?()/{
N
s/onComplete?()\\n            NotificationCenter/showComplete?()\
            NotificationCenter/
}' /Users/richard/Documents/timeout/Sources/Timeout/TimerManager.swift
