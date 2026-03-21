#!/bin/bash
# Timeout 应用测试脚本

echo "🧪 Timeout 应用测试"
echo "===================="
echo ""

# 颜色定义
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# 检查应用是否在运行
check_running() {
    if pgrep -f "Timeout" > /dev/null; then
        return 0
    else
        return 1
    fi
}

# 测试1：构建
echo "📦 测试 1: 构建应用"
if swift build > /dev/null 2>&1; then
    echo -e "${GREEN}✅ 构建成功${NC}"
else
    echo -e "${RED}❌ 构建失败${NC}"
    exit 1
fi
echo ""

# 测试2：启动应用
echo "🚀 测试 2: 启动应用"
if check_running; then
    echo -e "${YELLOW}⚠️  应用已在运行${NC}"
else
    swift run Timeout > /dev/null 2>&1 &
    sleep 2
    if check_running; then
        echo -e "${GREEN}✅ 应用启动成功${NC}"
    else
        echo -e "${RED}❌ 应用启动失败${NC}"
        exit 1
    fi
fi
echo ""

# 测试3：检查菜单栏
echo "👀 测试 3: 检查菜单栏图标"
echo "请确认菜单栏中是否有 🍅 图标"
read -p "按回车继续..."
echo ""

# 测试4：测试计时器
echo "⏱️  测试 4: 功能测试清单"
echo "请逐项测试以下功能："
echo ""
echo "基础功能："
echo "  [ ] 点击菜单栏图标打开面板"
echo "  [ ] 点击'开始工作'按钮"
echo "  [ ] 观察倒计时显示"
echo "  [ ] 点击'暂停'按钮"
echo "  [ ] 点击'继续'按钮"
echo "  [ ] 点击'停止'按钮"
echo ""
echo "设置功能："
echo "  [ ] 点击设置图标（⚙️）"
echo "  [ ] 调整工作时长"
echo "  [ ] 调整休息时长"
echo "  [ ] 选择不同的强制休息强度"
echo "  [ ] 启用/禁用跳过休息"
echo ""
echo "强制休息："
echo "  [ ] 等待工作时间结束（可以手动设置为 1 分钟测试）"
echo "  [ ] 观察休息窗口是否正确显示"
echo "  [ ] 测试轻度模式（应该可以关闭）"
echo "  [ ] 测试中度模式（应该拦截快捷键）"
echo ""
read -p "完成测试后按回车..."
echo ""

# 测试5：停止应用
echo "🛑 测试 5: 停止应用"
if pkill -f Timeout; then
    echo -e "${GREEN}✅ 应用已停止${NC}"
else
    echo -e "${YELLOW}⚠️  应用未运行${NC}"
fi
echo ""

# 测试6：重新启动
echo "🔄 测试 6: 重新启动应用"
swift run Timeout > /dev/null 2>&1 &
sleep 2
if check_running; then
    echo -e "${GREEN}✅ 应用重新启动成功${NC}"
else
    echo -e "${RED}❌ 应用重新启动失败${NC}"
    exit 1
fi
echo ""

echo "===================="
echo -e "${GREEN}🎉 所有测试完成！${NC}"
echo ""
echo "应用当前正在运行中"
echo "停止应用：pkill -f Timeout"
echo "重新启动：./run.sh"
