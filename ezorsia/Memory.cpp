// Memory.cpp
// Memory 工具类实现：Detours Hook 与带可选 VirtualProtect 的进程内存补丁。
#include "stdafx.h"
#include "Memory.h"
#include "detours.h"

bool Memory::UseVirtuProtect = true;

// Detours 事务式 Hook：Begin → UpdateThread → Attach/Detach → Commit；任一步失败则 Abort。
bool Memory::SetHook(bool attach, void** ptrTarget, void* ptrDetour)
{
    if (DetourTransactionBegin() != NO_ERROR)
    {
        return false;
    }

    HANDLE pCurThread = GetCurrentThread();

    if (DetourUpdateThread(pCurThread) == NO_ERROR)
    {
        auto pDetourFunc = attach ? DetourAttach : DetourDetach;

        if (pDetourFunc(ptrTarget, ptrDetour) == NO_ERROR)
        {
            if (DetourTransactionCommit() == NO_ERROR)
            {
                return true;
            }
        }
    }

    DetourTransactionAbort();
    return false;
}

// 批量填充内存字节；UseVirtuProtect 为 true 时先改页保护再写回。
void Memory::FillBytes(const DWORD dwOriginAddress, const unsigned char ucValue, const int nCount) {
    if (UseVirtuProtect) {
        DWORD dwOldProtect;
        VirtualProtect((LPVOID)dwOriginAddress, nCount, PAGE_EXECUTE_READWRITE, &dwOldProtect); //thanks colaMint, joo, and stelmo for informing me of using virtualprotect
        memset((void*)dwOriginAddress, ucValue, nCount);
        VirtualProtect((LPVOID)dwOriginAddress, nCount, dwOldProtect, &dwOldProtect);
    }
    else { memset((void*)dwOriginAddress, ucValue, nCount); }
}

/*
 注意: 原文本长度>=新文本长度
 示例: Memory::ReplaceString(0x00B3C158, "双击发送消息", "Double-click to send a note.");
*/
void Memory::ReplaceString(const DWORD dwOriginAddress, const char* sContent, const char* oContent)
{
    WriteString(dwOriginAddress, sContent);
    const size_t sSize = strlen(sContent);
    const size_t oSize = strlen(oContent);
    FillBytes(dwOriginAddress + sSize, 0, oSize + 1 - sSize);
}

// 写入字符串并零填充至固定槽位 oSize（含 '\0'；中文每字占 2 字节）。
// 示例: Memory::WriteString(0x00AF2B28, "对联盟", 11);
void Memory::WriteString(const DWORD dwOriginAddress, const char* sContent, const int oSize)
{
    WriteString(dwOriginAddress, sContent);
    const size_t sSize = strlen(sContent);
    FillBytes(dwOriginAddress + sSize, 0, oSize + 1 - sSize);
}

// 写入 null 结尾字符串，不额外填充。
void Memory::WriteString(const DWORD dwOriginAddress, const char* sContent) {
    const size_t nSize = strlen(sContent);
    if (UseVirtuProtect) {
        DWORD dwOldProtect;
        VirtualProtect((LPVOID)dwOriginAddress, nSize, PAGE_EXECUTE_READWRITE, &dwOldProtect);
        memcpy((void*)dwOriginAddress, sContent, nSize);
        VirtualProtect((LPVOID)dwOriginAddress, nSize, dwOldProtect, &dwOldProtect);
    }
    else { memcpy((void*)dwOriginAddress, sContent, nSize); }
}

// 以下 Write* 在目标 VA 处写入对应宽度标量，可选 VirtualProtect 包裹。
void Memory::WriteByte(const DWORD dwOriginAddress, const unsigned char ucValue) {
    if (UseVirtuProtect) {
        DWORD dwOldProtect;
        VirtualProtect((LPVOID)dwOriginAddress, sizeof(unsigned char), PAGE_EXECUTE_READWRITE, &dwOldProtect);
        *(unsigned char*)dwOriginAddress = ucValue;
        VirtualProtect((LPVOID)dwOriginAddress, sizeof(unsigned char), dwOldProtect, &dwOldProtect);
    }
    else { *(unsigned char*)dwOriginAddress = ucValue; }
}

void Memory::WriteShort(const DWORD dwOriginAddress, const unsigned short usValue) {
    if (UseVirtuProtect) {
        DWORD dwOldProtect;
        VirtualProtect((LPVOID)dwOriginAddress, sizeof(unsigned short), PAGE_EXECUTE_READWRITE, &dwOldProtect);
        *(unsigned short*)dwOriginAddress = usValue;
        VirtualProtect((LPVOID)dwOriginAddress, sizeof(unsigned short), dwOldProtect, &dwOldProtect);
    }
    else { *(unsigned short*)dwOriginAddress = usValue; }
}

void Memory::WriteInt(const DWORD dwOriginAddress, const unsigned int dwValue) {
    if (UseVirtuProtect) {
        DWORD dwOldProtect;
        VirtualProtect((LPVOID)dwOriginAddress, sizeof(unsigned int), PAGE_EXECUTE_READWRITE, &dwOldProtect);
        *(unsigned int*)dwOriginAddress = dwValue;
        VirtualProtect((LPVOID)dwOriginAddress, sizeof(unsigned int), dwOldProtect, &dwOldProtect);
    }
    else { *(unsigned int*)dwOriginAddress = dwValue; }
}

void Memory::WriteDouble(const DWORD dwOriginAddress, const double dwValue) {
    if (UseVirtuProtect) {
        DWORD dwOldProtect;
        VirtualProtect((LPVOID)dwOriginAddress, sizeof(double), PAGE_EXECUTE_READWRITE, &dwOldProtect);
        *(double*)dwOriginAddress = dwValue;
        VirtualProtect((LPVOID)dwOriginAddress, sizeof(double), dwOldProtect, &dwOldProtect);
    }
    else { *(double*)dwOriginAddress = dwValue; }
}

// 按字节逐个写入数组（每字节单独 VirtualProtect，适合稀疏保护页场景）。
void Memory::WriteByteArray(const DWORD dwOriginAddress, unsigned char* ucValue, const int ucValueSize) {
    if (UseVirtuProtect) {
        for (int i = 0; i < ucValueSize; i++) {
            const DWORD newAddr = dwOriginAddress + i;
            DWORD dwOldProtect;
            VirtualProtect((LPVOID)newAddr, sizeof(unsigned char), PAGE_EXECUTE_READWRITE, &dwOldProtect);
            *(unsigned char*)newAddr = ucValue[i];
            VirtualProtect((LPVOID)newAddr, sizeof(unsigned char), dwOldProtect, &dwOldProtect);
        }
    }
    else {
        for (int i = 0; i < ucValueSize; i++) { const DWORD newAddr = dwOriginAddress + i; *(unsigned char*)newAddr = ucValue[i]; }
    }
}

// 在原地址写入 5 字节相对 JMP（0xE9 + 偏移）跳转到 Code Cave；可选先 NOP 腾出空间。
// 偏移 = ptrCodeCave - dwOriginAddress - 5；异常时 __except 静默吞掉。
void Memory::CodeCave(void* ptrCodeCave, const DWORD dwOriginAddress, const int nNOPCount) { //tested and working
	__try {
		if (nNOPCount) FillBytes(dwOriginAddress, 0x90, nNOPCount); // create space for the jmp
		WriteByte(dwOriginAddress, 0xe9); // jmp instruction
		WriteInt(dwOriginAddress + 1, (int)(((int)ptrCodeCave - (int)dwOriginAddress) - 5)); // [jmp(1 byte)][address(4 bytes)] //this means you need to clear a space of at least 5 bytes (nNOPCount bytes)
	} __except (EXCEPTION_EXECUTE_HANDLER) {}
}
