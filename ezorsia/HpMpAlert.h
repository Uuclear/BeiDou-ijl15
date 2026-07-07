#pragma once

// HpMpAlert — HP/MP 低血量/低蓝量告警阈值与服务器双向同步
// 自定义发包 opcode 0x1000；SaveGlobal 时上报，ProcessPacket 时接收并写回状态栏。

// Hook 全局设置保存：玩家修改告警阈值后向服务器发送 0x1000 包
void HookSaveGlobal(bool enable);

// Hook 收包处理：拦截 opcode 0x1000，将服务器下发的阈值写入 UIStatusBar
void HookHpMpAlertRecv(bool enable);
