#!/bin/bash
echo "=== 视觉系统监控面板 ==="
echo ""

while true; do
    clear
    echo "========================================"
    echo "  RoboMaster视觉系统监控"
    echo "  时间: $(date)"
    echo "========================================"
    
    # 检查服务状态
    echo "[1] 服务状态:"
    systemctl --user status rm-vision.service 2>/dev/null || sudo systemctl status rm-vision.service
    
    echo ""
    echo "[2] 系统资源:"
    echo "  CPU使用: $(top -bn1 | grep "Cpu(s)" | awk '{print $2}')%"
    echo "  内存使用: $(free -h | awk '/^Mem:/ {print $3"/"$2}')"
    
    echo ""
    echo "[3] 进程信息:"
    ps aux | grep -E "rm_vision|opencv" | grep -v grep
    
    echo ""
    echo "[4] 日志最后5行:"
    journalctl -u rm-vision.service -n 5 --no-pager 2>/dev/null || echo "无日志记录"
    
    echo ""
    echo "========================================"
    echo "按 Ctrl+C 退出监控"
    echo "========================================"
    sleep 5
done
