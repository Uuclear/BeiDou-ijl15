// HpMpAlert.cpp — HP/MP 告警阈值同步
// 发包格式 4 字节：[opcode LE 0x1000][hpAlert][mpAlert]，阈值 clamp 0~20。
// 收包在 CInPacket 偏移 +4 处读 opcode，+6/+7 为 hp/mp 阈值。

#include "stdafx.h"
#include "HpMpAlert.h"
namespace {
// --- 客户端地址与偏移 ---
constexpr DWORD kSaveGlobalAddr = 0x0049C8E7;       // SaveGlobal — 玩家保存系统设置时触发
constexpr DWORD kUIStatusBarPtr = 0x00BEBF9C;       // UIStatusBar 单例指针
constexpr DWORD kHpAlertOffset = 0x80;              // 状态栏内 HP 告警阈值字段
constexpr DWORD kMpAlertOffset = 0x84;              // 状态栏内 MP 告警阈值字段
constexpr DWORD kClientSocketPtr = 0x00BE7914;       // 客户端 Socket 对象指针
constexpr DWORD kProcessPacketAddr = 0x004965F1;     // ProcessPacket — 入站包分发
constexpr WORD kOpcodeSetHpMpAlert = 0x1000;         // 自定义 HP/MP 告警同步 opcode
struct COutPacket {
    int Loopback;
    union {
        unsigned char* Data;
        void* Unk;
        unsigned short* Header;
    };
    unsigned long Size;
    unsigned int Offset;
    int EncryptedByShanda;
};
struct CInPacket {
    bool Loopback;
    int State;
    void* Data;
    unsigned long Size;
    unsigned short RawSeq;
    unsigned short DataLen;
    unsigned short Unknown;
    unsigned int Offset;
    void* Unk;
};
using SendPacket_t = void(__fastcall*)(void* pThis, void* edx, COutPacket* packet);
static SendPacket_t g_SendPacket = reinterpret_cast<SendPacket_t>(0x0049637B);
// SEH 保护的单 DWORD 读取，指针无效时返回 false
static bool TryReadDword(DWORD address, DWORD& out) {
    __try {
        out = *reinterpret_cast<DWORD*>(address);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        out = 0;
        return false;
    }
}
// 告警阈值合法范围 0~20（与客户端 UI 一致）
static unsigned char ClampAlert(int value) {
    if (value < 0) return 0;
    if (value > 20) return 20;
    return static_cast<unsigned char>(value);
}
// 从 UIStatusBar 读取当前 hp/mp 阈值，构造 0x1000 包经 SendPacket 发往服务器
static void SendHpMpAlertFromStatusBar() {
    DWORD statusBar = 0;
    if (!TryReadDword(kUIStatusBarPtr, statusBar) || statusBar == 0) {
        return;
    }
    DWORD hpRaw = 0;
    DWORD mpRaw = 0;
    if (!TryReadDword(statusBar + kHpAlertOffset, hpRaw)) {
        return;
    }
    if (!TryReadDword(statusBar + kMpAlertOffset, mpRaw)) {
        return;
    }
    const unsigned char hpAlert = ClampAlert(static_cast<int>(hpRaw));
    const unsigned char mpAlert = ClampAlert(static_cast<int>(mpRaw));
    DWORD socketPtr = 0;
    if (!TryReadDword(kClientSocketPtr, socketPtr) || socketPtr == 0) {
        return;
    }
    unsigned char payload[4] = {
        static_cast<unsigned char>(kOpcodeSetHpMpAlert & 0xFF),
        static_cast<unsigned char>((kOpcodeSetHpMpAlert >> 8) & 0xFF),
        hpAlert,
        mpAlert
    };
    COutPacket packet{};
    packet.Loopback = 0;
    packet.Data = payload;
    packet.Size = sizeof(payload);
    packet.Offset = 0;
    packet.EncryptedByShanda = 0;
    g_SendPacket(reinterpret_cast<void*>(socketPtr), nullptr, &packet);
}
// 将服务器下发的阈值写回 UIStatusBar 对应偏移
static void ApplyHpMpAlertToStatusBar(unsigned char hpAlert, unsigned char mpAlert) {
    DWORD statusBar = 0;
    if (!TryReadDword(kUIStatusBarPtr, statusBar) || statusBar == 0) {
        return;
    }
    Memory::WriteInt(statusBar + kHpAlertOffset, hpAlert);
    Memory::WriteInt(statusBar + kMpAlertOffset, mpAlert);
}
// 解析入站包：data+4 为 opcode，匹配 0x1000 后取 data[6]/data[7] 为 hp/mp
static void HandleHpMpAlertPacket(CInPacket* packet) {
    if (packet == nullptr) {
        return;
    }
    __try {
        if (packet->Data == nullptr || packet->Size < 8) {
            return;
        }
        const unsigned char* data = reinterpret_cast<const unsigned char*>(packet->Data);
        const unsigned short opcode = *reinterpret_cast<const unsigned short*>(data + 4);
        if (opcode != kOpcodeSetHpMpAlert) {
            return;
        }
        // data[6]/data[7] 为 HP/MP 告警阈值；后续字节可扩展其它设置
        const unsigned char hpAlert = ClampAlert(static_cast<int>(data[6]));
        const unsigned char mpAlert = ClampAlert(static_cast<int>(data[7]));
        ApplyHpMpAlertToStatusBar(hpAlert, mpAlert);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return;
    }
}
using SaveGlobal_t = void(__fastcall*)(void* pThis, void* edx);
static SaveGlobal_t s_SaveGlobal = reinterpret_cast<SaveGlobal_t>(kSaveGlobalAddr);
// SaveGlobal 后置钩子：原保存完成后上报当前告警阈值
static void __fastcall SaveGlobal_Hook(void* pThis, void* edx) {
    s_SaveGlobal(pThis, edx);
    SendHpMpAlertFromStatusBar();
}
using ProcessPacket_t = void(__fastcall*)(void* pThis, void* edx, CInPacket* packet);
static ProcessPacket_t s_ProcessPacket = reinterpret_cast<ProcessPacket_t>(kProcessPacketAddr);
// ProcessPacket 前置钩子：先处理 0x1000，再交原函数继续分发
static void __fastcall ProcessPacket_Hook(void* pThis, void* edx, CInPacket* packet) {
    HandleHpMpAlertPacket(packet);
    s_ProcessPacket(pThis, edx, packet);
}
} // namespace

void HookSaveGlobal(bool enable) {
    Memory::SetHook(enable, reinterpret_cast<void**>(&s_SaveGlobal), SaveGlobal_Hook);
}

void HookHpMpAlertRecv(bool enable) {
    Memory::SetHook(enable, reinterpret_cast<void**>(&s_ProcessPacket), ProcessPacket_Hook);
}
