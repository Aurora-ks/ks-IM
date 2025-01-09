package utils

import (
	"fmt"
	"sync"
	"time"
)

const (
	epoch          int64 = 1704067200000 // 自定义的起始时间戳（2021-01-01 00:00:00 UTC）
	nodeBits       uint8 = 10            // 机器ID的位数
	sequenceBits   uint8 = 12            // 序列号的位数
	nodeMax        int64 = -1 ^ (-1 << nodeBits)
	sequenceMask   int64 = -1 ^ (-1 << sequenceBits)
	nodeShift      uint8 = sequenceBits
	timestampShift uint8 = sequenceBits + nodeBits
)

var (
	sequence int64 = 0
	lastTime int64 = -1
	mutex    sync.Mutex
)

// GenID 生成消息ID
// 返回生成的ID和可能的错误
func GenID(nid int64) (int64, error) {
	// 检查节点ID是否在有效范围内
	if nid < 0 || nid > nodeMax {
		return 0, fmt.Errorf("节点ID必须在0和%d之间", nodeMax)
	}
	nodeID := nid
	mutex.Lock()
	defer mutex.Unlock()

	// 获取当前时间戳，精确到毫秒
	currentTime := time.Now().UnixNano() / 1e6

	// 如果当前时间戳小于上一次生成ID的时间戳，说明时间回退，拒绝生成ID
	if currentTime < lastTime {
		return 0, fmt.Errorf("时间回退，拒绝生成ID")
	}

	// 如果当前时间戳等于上一次生成ID的时间戳，增加序列号
	if currentTime == lastTime {
		sequence = (sequence + 1) & sequenceMask
		// 如果序列号达到最大值，等待下一个毫秒
		if sequence == 0 {
			currentTime = waitNextMillis(lastTime)
		}
	} else {
		// 如果当前时间戳大于上一次生成ID的时间戳，重置序列号
		sequence = 0
	}

	// 更新上一次生成ID的时间戳
	lastTime = currentTime

	// 计算时间戳的偏移量
	timestamp := currentTime - epoch

	// 组合成最终的ID
	id := (timestamp << timestampShift) | (nodeID << nodeShift) | sequence

	return id, nil
}

// waitNextMillis 等待下一个毫秒
// last: 上一次生成ID的时间戳
// 返回下一个毫秒的时间戳
func waitNextMillis(last int64) int64 {
	currentTime := time.Now().UnixNano() / 1e6
	for currentTime <= last {
		currentTime = time.Now().UnixNano() / 1e6
	}
	return currentTime
}
