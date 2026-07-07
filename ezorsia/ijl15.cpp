// ijl15.cpp — ijl15.dll 注入入口与 API 转发
// MapleStory 启动时会 LoadLibrary("ijl15.dll")；本 DLL 替代原版并
// 在 CreateHook 中绑定 2ijl15.dll，导出函数以 naked jmp 转发（须 x86 编译）。

#include "stdafx.h"
#include "ijl15.h"

FARPROC ijlErrorStr_Proc;
FARPROC ijlFree_Proc;
FARPROC ijlGetLibVersion_Proc;
FARPROC ijlInit_Proc;
FARPROC ijlRead_Proc;
FARPROC ijlWrite_Proc;

// 加载重命名后的原版 ijl15 库并缓存各 API 入口地址
void ijl15::CreateHook() {
	HMODULE hModule = LoadLibraryA("2ijl15.dll");
	if (hModule == nullptr) {
		MessageBoxW(NULL, L"Failed to find 2ijl15.dll file", L"Missing file", 0);
		return;
	}
	ijlErrorStr_Proc = GetProcAddress(hModule, "ijlErrorStr");
	ijlFree_Proc = GetProcAddress(hModule, "ijlFree");
	ijlGetLibVersion_Proc = GetProcAddress(hModule, "ijlGetLibVersion");
	ijlInit_Proc = GetProcAddress(hModule, "ijlInit");
	ijlRead_Proc = GetProcAddress(hModule, "ijlRead");
	ijlWrite_Proc = GetProcAddress(hModule, "ijlWrite");
}

// 导出符号名与官方 ijl15.dll 一致，内部 jmp 至 2ijl15.dll 对应实现
extern "C" __declspec(dllexport) __declspec(naked) void ijlGetLibVersion()
{
	__asm	jmp dword ptr[ijlGetLibVersion_Proc] // 须 x86 编译
}

extern "C" __declspec(dllexport) __declspec(naked) void ijlInit()
{
	__asm	jmp dword ptr[ijlInit_Proc]
}

extern "C" __declspec(dllexport) __declspec(naked) void ijlFree()
{
	__asm	jmp dword ptr[ijlFree_Proc]
}

extern "C" __declspec(dllexport) __declspec(naked) void ijlRead()
{
	__asm	jmp dword ptr[ijlRead_Proc]
}

extern "C" __declspec(dllexport) __declspec(naked) void ijlWrite()
{
	__asm	jmp dword ptr[ijlWrite_Proc]
}

extern "C" __declspec(dllexport) __declspec(naked) void ijlErrorStr()
{
	__asm	jmp dword ptr[ijlErrorStr_Proc]
}
