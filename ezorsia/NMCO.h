#pragma once

// NMCO — Nexon Module Client Object DLL 代理
// 本模块导出与官方 NMCO.dll 相同的符号，运行时将调用转发至 nmconew2.dll。

class NMCO
{
public:
	// 加载 nmconew2.dll 并解析 NMCO_CallNMFunc / NMCO_CallNMFunc2 / NMCO_MemoryFree 入口
	static void CreateHook();
};
