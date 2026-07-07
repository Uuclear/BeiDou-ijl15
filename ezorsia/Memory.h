#pragma once

// Memory — 客户端进程内存读写与 Hook 工具类（全部为静态方法）。
// 封装 Microsoft Detours 挂钩及 VirtualProtect 保护下的字节/字符串补丁操作，
// 供 Client 与各 Hook 模块直接调用目标地址（DWORD 形式的 VA）。
class Memory
{
public:
	// 使用 Detours 附加/卸载 inline hook。attach=true 时 DetourAttach，否则 DetourDetach。
	static bool SetHook(bool attach, void** ptrTarget, void* ptrDetour);

	// 从 dwOriginAddress 起连续写入 nCount 个字节 ucValue（常用 0x00 或 0x90）。
	static void FillBytes(DWORD dwOriginAddress, unsigned char ucValue, int nCount);

	// 替换内存字符串：写入 sContent 并用 0 填充至原串 oContent 长度（新串长度须 <= 原串）。
	static void ReplaceString(DWORD dwOriginAddress, const char* sContent, const char* oContent);

	// 写入字符串并填充至固定槽位长度 oSize（含结尾 '\0' 区域；中文按字节计长）。
	static void WriteString(DWORD dwOriginAddress, const char* sContent, const int oSize);

	// 写入以 '\0' 结尾的字符串，长度为 strlen(sContent)。
	static void WriteString(DWORD dwOriginAddress, const char* sContent);

	static void WriteByte(DWORD dwOriginAddress, unsigned char ucValue);
	static void WriteShort(DWORD dwOriginAddress, unsigned short usValue);
	static void WriteInt(DWORD dwOriginAddress, unsigned int dwValue);
	static void WriteDouble(DWORD dwOriginAddress, double dwValue);

	// 在 dwOriginAddress 处写入相对跳转（E9）至 ptrCodeCave；nNOPCount 为跳转前 NOP 填充字节数。
	static void CodeCave(void* ptrCodeCave, DWORD dwOriginAddress, int nNOPCount);

	// 逐字节写入 ucValue 数组，长度为 ucValueSize。
	static void WriteByteArray(DWORD dwOriginAddress, unsigned char* ucValue, const int ucValueSize);

	// 是否在写入前调用 VirtualProtect 临时改为 PAGE_EXECUTE_READWRITE（config.ini: UseVirtuProtect）。
	static bool UseVirtuProtect;

	// 在指定地址连续写入 NOP 指令（0x90），等效于 FillBytes(..., 0x90, nCount)。
    static void PatchNop(DWORD dwOriginAddress, int nCount) {
        if (nCount <= 0) return;

        // 根据需要设置内存保护
        DWORD oldProtect;
        if (UseVirtuProtect) {
            VirtualProtect((LPVOID)dwOriginAddress, nCount, PAGE_EXECUTE_READWRITE, &oldProtect);
        }

        // 填充NOP指令(0x90)
        memset((void*)dwOriginAddress, 0x90, nCount);

        // 恢复内存保护
        if (UseVirtuProtect) {
            DWORD temp;
            VirtualProtect((LPVOID)dwOriginAddress, nCount, oldProtect, &temp);
        }
    }
};

