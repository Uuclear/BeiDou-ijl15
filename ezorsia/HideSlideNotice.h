#pragma once

// Hide top slide notice (CSlideNotice)
// 0x007E15DF = CWnd::CreateWnd in ctor (800x23 bar)
// 0x007E1686 = CreateWnd override
// 0x007E16FE = OnCreate entry
// 0x007E1690 is inside ctor; hooking it crashes

__declspec(naked) void hideSlideNoticeOnCreate() {
	__asm {
		xor eax, eax
		ret
	}
}

__declspec(naked) void hideSlideNoticeCreateWnd() {
	__asm {
		xor eax, eax
		ret 4
	}
}

class HideSlideNotice {
public:
	// Skip bar window creation, empty OnCreate, ignore notice packet
	static void Hook() {
		Memory::WriteByte(0x007E15DF, 0x83);
		Memory::WriteByte(0x007E15E0, 0xC4);
		Memory::WriteByte(0x007E15E1, 0x14);
		Memory::WriteByte(0x007E15E2, 0x90);
		Memory::WriteByte(0x007E15E3, 0x90);
		Memory::CodeCave(hideSlideNoticeCreateWnd, 0x007E1686, 5);
		Memory::CodeCave(hideSlideNoticeOnCreate, 0x007E16FE, 5);
		Memory::PatchNop(0x0046E9D3, 5);
	}
};
