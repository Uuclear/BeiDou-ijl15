#pragma once

// ijl15 — Intel JPEG Library 代理 DLL
// 客户端通过加载 ijl15.dll 注入 mod；本模块将 API 转发至重命名的 2ijl15.dll（原版库）。

class ijl15
{
public:
	// 加载 2ijl15.dll 并解析 ijlInit/ijlRead/ijlWrite 等导出函数地址
	static void CreateHook();
};
