#pragma once

// FixBuddy — 好友申请接受修复
// 客户端对好友名字节长度校验过严，中文名（GBK 多字节）会被误判；
// 在 accept 路径调用原字符串处理函数后，用 [ebp-33h]==9 识别中文名并跳过重检。

DWORD fixBuddyAcceptFunc = 0x00A3FA51;  // 原字符串/编码处理例程
DWORD fixBuddyAcceptJmp = 0x005312D5;   // 中文名分支：跳过错误校验
DWORD fixBuddyAcceptRtn = 0x005312A3;   // 默认返回路径
__declspec(naked) void fixBuddyAccept() {
	__asm {
		call fixBuddyAcceptFunc
		cmp byte ptr[ebp - 33h], 9   // 9 = 中文名字符串类型标识
		jz label_jmp                 // 中文名 → 跳过客户端错误校验
		jmp fixBuddyAcceptRtn

		label_jmp :
		jmp fixBuddyAcceptJmp
	}
}

class FixBuddy {
public:
	// 在 0x0053129E 处 CodeCave 注入 fixBuddyAccept
	static void Hook() {
		Memory::CodeCave(fixBuddyAccept, 0x0053129E, 5);
	}
};