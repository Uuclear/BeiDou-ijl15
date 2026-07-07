#pragma once
// FixIme — 中文 IME 修复
// 解决非输入法状态下“卡门”、剪贴板/聊天中文、角色名检测等问题；
// 提供 HookOld（旧 Win10）与 HookNew（Win10 后期/Win11）两套 IME 切换方案。

#include <imm.h>
#pragma comment(lib, "imm32.lib")

// 将前台窗口重新关联 IME 上下文（当前未使用，效果未验证）
void EnableIme() {
	HWND hwnd = GetForegroundWindow(); // 获取当前前台窗口的句柄
	if (hwnd) {
		// 获取输入法上下文
		HIMC hImc = ImmGetContext(hwnd);
		if (hImc) {
			// 将输入法上下文重新关联到窗口
			ImmAssociateContext(hwnd, hImc);
			ImmReleaseContext(hwnd, hImc);
		}
	}
}

// 解除前台窗口的 IME 关联（密码框等场景禁用输入法）
void DisableIme() {
	HWND hwnd = GetForegroundWindow(); // 获取当前前台窗口的句柄
	if (hwnd) {
		// 获取输入法上下文
		HIMC hImc = ImmGetContext(hwnd);
		if (hImc) {
			// 解除输入法上下文的关联
			ImmAssociateContext(hwnd, NULL);
			ImmReleaseContext(hwnd, hImc);
		}
	}
}

BYTE enabled = 1; // 标记当前是否已启用 IME，destroyWindow 时据此决定是否 DisableIme

DWORD funcEnableImeAddr = 0x009E85F3; // 客户端 EnableIme(bool) 函数

// --- HookOld 专用：CCtrlEdit 单行输入 OnSetFocus 分支修正 ---
DWORD setOnFocusFirstJudgementRtnAddr = 0x004CA061;
DWORD switchImeAddr = 0x004CA078;
// 原逻辑会跳过 IME 切换；此处强制跳至 switchIme 路径
__declspec(naked) void setOnFocusFirstJudgement() {
	__asm {
		cmp[esp + 0Ch], edi
		jz label_jmp_switch_ime
		jmp setOnFocusFirstJudgementRtnAddr
		
		label_jmp_switch_ime :
		jmp switchImeAddr
	}
}

DWORD enableRtnAddr = 0x004CA08F;
DWORD disableRtnAddr = 0x004CA091;
// HookOld：按焦点与 esi+0x80 标志决定 enable/disable IME
__declspec(naked) void switchIme() {
	__asm {
		cmp [esp + 0Ch], edi
		jz  label_jz
		xor eax, eax
		cmp [esi + 0x80], eax
		setz al
		push eax
		call funcEnableImeAddr
		mov enabled, 1
		jmp  enableRtnAddr

		label_jz :
		push 0
		call funcEnableImeAddr
		jmp  disableRtnAddr
	}
}

// HookOld：CCtrlMLEdit 多行输入 OnSetFocus IME 切换
DWORD enableMLRtnAddr = 0x004D32E0;
DWORD disableMLRtnAddr = 0x004D32E2;
__declspec(naked) void switchMLIme() {
	__asm {
		cmp  dword ptr[esp + 8], 0
		jz   label_jz
		push 1
		call funcEnableImeAddr
		mov enabled, 1
		jmp  enableMLRtnAddr

		label_jz :
		push 0
		call funcEnableImeAddr
		jmp  disableMLRtnAddr
	}
}

// --- HookNew 专用：重写单行 IME 逻辑，Win11 下 HookOld 的 switchIme 失效 ---
DWORD newSwitchImeRtnAddr = 0x004CA08F;
// esi+0x80==1 为密码框 → DisableIme；否则 push 1 启用 IME
__declspec(naked) void newSwitchIme() {
	__asm {
		cmp[esi + 0x80], 1 // 密码框标志
		jz label_disable
		push 1
		call funcEnableImeAddr
		mov enabled, 1
		jmp newSwitchImeRtnAddr

		label_disable :
		call DisableIme
		jmp newSwitchImeRtnAddr
	}
}

// 控件销毁时若 enabled 则 DisableIme，防止 IME 残留（HookOld/HookNew 共用）
DWORD destroyWindowRtnAddr = 0x004DFEAD;
DWORD destroyWindowFuncAddr = 0x0041FE69;
__declspec(naked) void destroyWindow() {
	__asm {
		call destroyWindowFuncAddr
		or dword ptr[esi + 14h], 0FFFFFFFFh

		cmp enabled, 0
		jz label_return

		call DisableIme
		mov enabled, 0

		label_return :
		jmp destroyWindowRtnAddr
	}
}

// HookNew：多行输入一律 push 1 启用 IME（简化原 switchMLIme 分支）
DWORD newSwitchMLImeRtnAddr = 0x004D32EE;
__declspec(naked) void newSwitchMLIme() {
	__asm {
		push 1
		call funcEnableImeAddr
		mov enabled, 1
		jmp  newSwitchMLImeRtnAddr
	}
}


class FixIme {
public:
	// HookOld — 适用于较旧 Win10
	// 修正 CCtrlEdit/CCtrlMLEdit 的 OnSetFocus IME 分支；已知问题：商城礼物“内容”栏无法调出 IME
	static void HookOld() {
		GeneralHook();
		Memory::CodeCave(setOnFocusFirstJudgement, 0x004CA05B, 6); // CCtrlEdit::OnSetFocus 前置判断
		Memory::CodeCave(switchIme, 0x004CA089, 6);               // CCtrlEdit IME 切换
		Memory::FillBytes(0x004D32C6, 0x90, 2);                   // 去掉 MLEdit 原分支
		Memory::CodeCave(switchMLIme, 0x004D32D9, 7);             // CCtrlMLEdit IME 切换
		Memory::CodeCave(destroyWindow, 0x004DFEA4, 9);           // 控件销毁时禁用 IME
		std::cout << "Old Ime Hook" << std::endl;
	}

	// HookNew — 适用于 Win10 后期 / Win11（HookOld 在 Win11 失效）
	// 用 newSwitchIme/newSwitchMLIme 重写单行/多行逻辑；密码框仍通过 esi+0x80 禁用 IME
	static void HookNew() {
		GeneralHook();
		Memory::CodeCave(newSwitchIme, 0x004CA089, 6);            // 单行：非密码框启用 IME
		Memory::CodeCave(destroyWindow, 0x004DFEA4, 9);
		Memory::CodeCave(newSwitchMLIme, 0x004D32D9, 7);          // 多行：始终启用 IME
		std::cout << "New Ime Hook" << std::endl;
	}
private:
	// HookOld/HookNew 共用：NOP 卡门检测、剪贴板中文、角色名中文校验
	static void GeneralHook() {
		Memory::FillBytes(0x008D54A6, 0x90, 9); // 按键输入非 IME 字符拦截
		Memory::FillBytes(0x00937225, 0x90, 9); // 聊天输入
		Memory::FillBytes(0x00531EE8, 0x90, 9); // 群聊消息
		Memory::FillBytes(0x004CAE7D, 0x90, 2); // 剪贴板粘贴中文
		Memory::WriteByte(0x004CAE8F, 0xEB);
		Memory::FillBytes(0x007A015D, 0x90, 2); // 允许角色名含中文
	}
};