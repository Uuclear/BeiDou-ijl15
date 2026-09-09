#pragma once

// 隐藏顶部滚动公告（CSlideNotice）
// 0x007E16FE = OnCreate 入口；0x007E1690 在构造函数内部，Hook 会闪退

__declspec(naked) void hideSlideNoticeOnCreate() {
	__asm {
		xor eax, eax
		ret
	}
}

class HideSlideNotice {
public:
	// OnCreate 返回 0 + 忽略公告包 SetMsg 调用
	static void Hook() {
		Memory::CodeCave(hideSlideNoticeOnCreate, 0x007E16FE, 5);
		Memory::PatchNop(0x0046E9D3, 5);
	}
};
