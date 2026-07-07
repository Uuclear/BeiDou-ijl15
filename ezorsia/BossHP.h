#pragma once

// BossHP — 首领 HP 百分比显示
// 拦截 CField::ShowMobHPTag 获取当前 Boss 血量，在 CUserLocal::Update 中
// 于小地图旁通过 CUIToolTip 绘制百分比 Tooltip；换图/离场时清理。

class BossHP
{
public:
	// 安装全部 Hook（入口）
	static void Hook();
private:
	static char aBossHpUIToolTip[1304];   // 自管 CUIToolTip 实例缓冲区
	static double dBossHpPercentage;      // 当前 Boss HP 百分比，0 表示无 Boss 条
	//
	static void HookInternal();           // 依次安装 Update/ShowMobHPTag/Field 钩子
	static void HookUpdate();             // Hook CUserLocal::Update，每帧刷新 Tooltip
	static void HookShowMobHPTag();       // Hook CField::ShowMobHPTag，捕获 HP/MaxHP
	static void HookDisposeField();       // Hook CField 析构，离场清理
	static void HookInitField();          // Hook CField 构造，重建 Tooltip 实例
	//
	static void SetToolTip_String(int instance, int x, int y, const char* sToolTip);
	static void ClearToolTip(int instance);
	static void DisposeToolTip(int instance);
	static void CreateToolTip(int instance);
	//
	static void DrawBossHpNumberIfNeed(); // dBossHpPercentage>0 时写入 Tooltip 文本
	static void DrawBossHpNumber(int nHP, int nMaxHP); // 由 ShowMobHPTag 回调更新百分比
	static void DisposeBossHpNumber();    // 清零百分比并清除 Tooltip
	//
	static int GetMiniMapWidth();         // 读取 CUIMiniMap 宽度，用于 Tooltip X 坐标

};