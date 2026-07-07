// NMCO.cpp — NMCO.dll 转发代理
// 游戏加载本 DLL 代替原版 NMCO.dll；CreateHook 绑定真实实现到 nmconew2.dll，
// 导出函数以 naked + jmp 无栈开销跳转。

#include "stdafx.h"
#include "NMCO.h"

FARPROC dwNMCOCallFunc;    // nmconew2!NMCO_CallNMFunc
FARPROC dwNMCOCallFunc2;   // nmconew2!NMCO_CallNMFunc2
FARPROC dwNMCOMemoryFree;  // nmconew2!NMCO_MemoryFree

void NMCO::CreateHook() {
	HMODULE hModule = LoadLibraryA("nmconew2.dll");
	if (hModule == nullptr) {
		MessageBoxW(NULL, L"Failed to find nmconew2.dll file", L"Missing file", 0);
		return;
	}
	dwNMCOCallFunc = GetProcAddress(hModule, "NMCO_CallNMFunc");
	dwNMCOCallFunc2 = GetProcAddress(hModule, "NMCO_CallNMFunc2");
	dwNMCOMemoryFree = GetProcAddress(hModule, "NMCO_MemoryFree");
}

// 以下三个导出与原 NMCO.dll 符号名一致，供客户端 GetProcAddress 解析
extern "C" __declspec(dllexport) __declspec(naked) void NMCO_CallNMFunc() {
	__asm jmp dwNMCOCallFunc
}

extern "C" __declspec(dllexport) __declspec(naked) void NMCO_CallNMFunc2() {
	__asm jmp dwNMCOCallFunc2
}

extern "C" __declspec(dllexport) __declspec(naked) void NMCO_MemoryFree() {
	__asm jmp dwNMCOMemoryFree
}
