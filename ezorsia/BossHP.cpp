// BossHP.cpp — 首领 HP 百分比显示实现
// 客户端 v95 硬编码地址；通过 Memory::SetHook 拦截原函数并在回调中
// 更新 dBossHpPercentage，再借 CUIToolTip 在小地图区域绘制 "xx.xx%"。

#include "stdafx.h"
#include "BossHP.h"

// --- 客户端符号地址（v95）---
const DWORD dw_TSingleton_CUIMiniMap___ms_pInstance = 0x00BED788; // TSingleton<CUIMiniMap>::ms_pInstance
const DWORD dwCField__ShowMobHpTag = 0x005336CA;                 // CField::ShowMobHPTag — Boss 血条数据源
const DWORD dwCField__Init = 0x00528DBC;                         // CField::CField — 进图时重建 Tooltip
const DWORD dwCField__Dispose = 0x00529035;                        // CField::~CField — 离图时清理
const DWORD dwCUIToolTip__SetToolTip_String = 0x008E6E7D;         // 设置 Tooltip 文本与坐标
const DWORD dwCUIToolTip__ClearToolTip = 0x008E6E23;               // 清空 Tooltip 显示
const DWORD dwCUIToolTip__DisposeToolTip = 0x008E6BA3;             // CUIToolTip 析构
const DWORD dwCUIToolTip__CreateToolTip = 0x008E49B5;              // CUIToolTip 构造
const DWORD dwCUserLocal__Update = 0x0094A144;                     // CUserLocal::Update — 每帧刷新入口

char BossHP::aBossHpUIToolTip[1304];
double BossHP::dBossHpPercentage = 0;

void BossHP::Hook() {
	HookInternal();
}

// 安装四条 Hook：Update 刷新、ShowMobHPTag 采数、Field 构造/析构生命周期
void BossHP::HookInternal() {
	HookUpdate();
	HookShowMobHPTag();
	BossHP::HookDisposeField();
	HookInitField();
}

// Hook CUserLocal::Update：原逻辑执行后，若存在 Boss 血条则刷新 Tooltip 位置与文本
void BossHP::HookUpdate() {
	typedef void(__fastcall* UserLocal__Update_type)(void* pThis, void* edx);
	static auto _UserLocal__Update = reinterpret_cast<UserLocal__Update_type>(dwCUserLocal__Update);

	UserLocal__Update_type Hook = [](void* pThis, void* edx) -> void
	{
		_UserLocal__Update(pThis, edx);
		DrawBossHpNumberIfNeed();
	};

	Memory::SetHook(true, reinterpret_cast<void**>(&_UserLocal__Update), Hook);
}

// Hook CField::ShowMobHPTag：客户端绘制 Boss 血条时传入 nHP/nMaxHP，据此计算百分比
void BossHP::HookShowMobHPTag() {
	typedef void(__fastcall* Field__ShowMobHPTag_type)(void* pThis, void* edx, unsigned int dwMobID, int nColor, int nBgColor, int nHP, int nMaxHP);
	static auto _Field__ShowMobHPTag = reinterpret_cast<Field__ShowMobHPTag_type>(dwCField__ShowMobHpTag);

	Field__ShowMobHPTag_type Hook = [](void* pThis, void* edx, unsigned int dwMobID, int nColor, int nBgColor, int nHP, int nMaxHP) -> void
	{
		_Field__ShowMobHPTag(pThis, edx, dwMobID, nColor, nBgColor, nHP, nMaxHP);
		DrawBossHpNumber(nHP, nMaxHP);
	};
	Memory::SetHook(true, reinterpret_cast<void**>(&_Field__ShowMobHPTag), Hook);
}

// Hook CField 构造：换图前销毁旧 Tooltip、构造新实例，避免跨地图残留
void BossHP::HookInitField() {
	typedef void(__fastcall* Field__Init_Type)(void* pThis, void* edx);
	static auto _Field__Init = reinterpret_cast<Field__Init_Type>(dwCField__Init);

	Field__Init_Type Hook = [](void* pThis, void* edx) -> void
	{
		if (dBossHpPercentage > 0) {
			BossHP::DisposeBossHpNumber();
		}
		BossHP::DisposeToolTip((int)&aBossHpUIToolTip);
		BossHP::CreateToolTip((int)&aBossHpUIToolTip);
		_Field__Init(pThis, edx);
	};
	Memory::SetHook(true, reinterpret_cast<void**>(&_Field__Init), Hook);
}

// Hook CField 析构：离开地图时清除 Boss 百分比与 Tooltip
void BossHP::HookDisposeField() {
	typedef void(__fastcall* Field__Dispose_Type)(void* pThis, void* edx);
	static auto _Field__Dispose = reinterpret_cast<Field__Dispose_Type>(dwCField__Dispose);

	Field__Dispose_Type Hook = [](void* pThis, void* edx) -> void
	{
		DisposeBossHpNumber();
		_Field__Dispose(pThis, edx);
	};
	Memory::SetHook(true, reinterpret_cast<void**>(&_Field__Dispose), Hook);
}

// 每帧调用：有 Boss 血条时在 (小地图宽度, 37) 处显示 "xx.xx%"
void BossHP::DrawBossHpNumberIfNeed() {
	if (dBossHpPercentage > 0) {
		char sToolTip[20];
		sprintf_s(sToolTip, "%.2f%%", dBossHpPercentage);
		BossHP::SetToolTip_String((int)&aBossHpUIToolTip, GetMiniMapWidth(), 37, sToolTip);
	}
}

// ShowMobHPTag 回调：nHP<=0 时视为 Boss 已死/血条消失，清零并 ClearToolTip
void BossHP::DrawBossHpNumber(int nHP, int nMaxHP) {
	if (nHP > 0) {
		dBossHpPercentage = static_cast<double>(nHP) / nMaxHP * 100.0;
	}
	else {
		dBossHpPercentage = 0;
		BossHP::ClearToolTip((int)&aBossHpUIToolTip);
	}
}

void BossHP::DisposeBossHpNumber() {
	dBossHpPercentage = 0;
	BossHP::ClearToolTip((int)&aBossHpUIToolTip);
}

// --- CUIToolTip 薄封装：instance 传 &aBossHpUIToolTip 当作 this 指针 ---

typedef void(__fastcall* UIToolTip__SetToolTip_String_Type)(int pThis, void* edx, int x, int y, const char* sToolTip);
static auto _UIToolTip__SetToolTip_String = reinterpret_cast<UIToolTip__SetToolTip_String_Type>(dwCUIToolTip__SetToolTip_String);

void BossHP::SetToolTip_String(int instance, int x, int y, const char* sToolTip) {
	_UIToolTip__SetToolTip_String(instance, 0, x, y, sToolTip);
}

typedef void(__fastcall* UIToolTip__ClearToolTip_Type)(int pThis, void* edx);
static auto _UIToolTip__ClearToolTip = reinterpret_cast<UIToolTip__ClearToolTip_Type>(dwCUIToolTip__ClearToolTip);

void BossHP::ClearToolTip(int instance) {
	_UIToolTip__ClearToolTip(instance, 0);
}

typedef void(__fastcall* UIToolTip__DisposeToolTip_Type)(int pThis, void* edx);
static auto _UIToolTip__DisposeToolTip = reinterpret_cast<UIToolTip__DisposeToolTip_Type>(dwCUIToolTip__DisposeToolTip);

void BossHP::DisposeToolTip(int instance)
{
	_UIToolTip__DisposeToolTip(instance, 0);
}

typedef void(__fastcall* UIToolTip__CreateToolTip_Type)(int pThis, void* edx);
static auto _UIToolTip__CreateToolTip = reinterpret_cast<UIToolTip__CreateToolTip_Type>(dwCUIToolTip__CreateToolTip);

void BossHP::CreateToolTip(int instance)
{
	_UIToolTip__CreateToolTip(instance, 0);
}

// 解除内存保护后读取 int（用于读全局单例指针）
int ReadInt(const DWORD dwAddress) {
	int nResult = -1;
	DWORD dwOldProtect;
	VirtualProtect((void*)dwAddress, sizeof(int), PAGE_EXECUTE_READ, &dwOldProtect);
	nResult = *reinterpret_cast<unsigned int*>(dwAddress);
	VirtualProtect((void*)dwAddress, sizeof(int), dwOldProtect, &dwOldProtect);
	return nResult;
}

// CUIMiniMap 实例 +0x24 为控件宽度，Tooltip 水平贴齐小地图右缘
int BossHP::GetMiniMapWidth() {
	return ReadInt(ReadInt(dw_TSingleton_CUIMiniMap___ms_pInstance) + 0x24);
}