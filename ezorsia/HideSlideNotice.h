#pragma once

// 隐藏顶部滚动公告（CSlideNotice）
// 地址对应 v83 / MapleStory.exe（与 ezorsia UpdateResolution 一致）

// OnCreate：返回 0，不创建顶栏窗口
__declspec(naked) void hideSlideNoticeOnCreate() {
	__asm {
		xor eax, eax
		ret
	}
}

class HideSlideNotice {
public:
	// 挂钩 OnCreate，并 NOP 公告包处理里的 SetMsg 调用
	static void Hook() {
		Memory::CodeCave(hideSlideNoticeOnCreate, 0x007E1690, 5);
		Memory::PatchNop(0x0046E9D3, 5);
	}
};
