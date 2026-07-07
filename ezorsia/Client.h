#pragma once

// Client — 客户端补丁协调中心（全部为静态成员与方法）。
// 集中持有 config.ini 读取后的运行参数，并提供各功能模块的内存补丁入口，
// 由 dllmain.cpp 在 DLL_PROCESS_ATTACH 中按序调用。
class Client
{
public:
	// 启动期补丁：服务器 IP/端口、属性上限、UAC 相关、移动速度等核心内存修改。
	static void UpdateGameStartup();
	// 切换 IGCipher 哈希（当前未被调用，保留供自定义加密方案）。
	static void EnableNewIGCipher();
	// 根据 m_nGameWidth/Height 批量修改分辨率相关硬编码与 UI 布局。
	static void UpdateResolution();
	// 登录界面控件位置/配色（当前未在 DllMain 中调用，保留地址参考）。
	static void UpdateLogin();
	// 修复高分辨率下鼠标滚轮缩放行为。
	static void FixMouseWheel();
	// 中文/IME 相关：输入法 Hook、好友面板及 SwitchChinese 下的 UI 文案与字体。
	static void Chinese();
	// 扩展快捷栏至 26 键（Long Quick Slot）及相关数组与 Code Cave。
	static void LongQuickSlot();
	// 中文模式下修正日期显示格式（StringPool 相关 Code Cave）。
	static void FixDateFormat();
	// 中文模式下修正物品类型名称显示。
	static void FixItemType();
	// 自定义跳跃上限 jumpCap 及攀爬速度 climbSpeed / climbSpeedAuto。
	static void JumpCap();
	// 修复聊天窗口文本垂直偏移。
	static void FixChatPosHook();
	// debug + noPassword 时跳过密码校验相关逻辑。
	static void NoPassword();
	// 杂项补丁：装备页职业偏移、发包间隔、talkRepeat、大数值面板布局、窗口边界等。
	static void MoreHook();
	// 解除世界地图数量限制并居中大地图 UI。
	static void WorldMap();

	// --- 配置与常量（默认值可被 config.ini 覆盖）---

	static const int m_nIGCipherHash = 0XC65053F2; // IGCipher 哈希常量
	static int m_nGameHeight;           // 游戏窗口高度（像素）
	static int m_nGameWidth;            // 游戏窗口宽度（像素）
	static int MsgAmount;               // 聊天/系统消息显示条数上限
	static bool CustomLoginFrame;       // 是否使用自定义登录框资源
	static bool WindowedMode;           // 窗口模式（相对全屏）
	static bool RemoveLogos;            // 是否移除启动 Logo
	static int setDamageCap;            // 物理攻击面板显示上限
	static int setMAtkCap;              // 魔法攻击面板显示上限
	static int setAccCap;               // 命中上限
	static int setAvdCap;               // 回避上限
	static double setAtkOutCap;         // 输出/详情面板数值显示上限（>999999 时扩展 UI）
	static bool useTubi;                // 是否启用 Tubi 相关 NOP 补丁
	static bool bigLoginFrame;          // 是否使用大型登录框布局
	static bool SwitchChinese;          // 中文本地化开关（日期/物品类型/UI 文案等）
	static bool debug;                  // 调试模式
	static bool noPassword;             // 无密码登录（需配合 debug）
	static bool climbSpeedAuto;         // 是否用 Hook 自动计算攀爬速度
	static float climbSpeed;            // 手动攀爬速度倍率（×3 写入内存）
	static int speedMovementCap;        // 移动速度上限（写入相关校验常量）
	static unsigned char imeType;       // 输入法类型：0=旧 IME Hook，其他=新 IME Hook
	static DWORD jumpCap;               // 跳跃力上限（自定义 Hook 读取）
	static std::string ServerIP_AddressFromINI; // 自 config 解析后的 IPv4 服务器地址
	static int serverIP_Port;           // 登录服务器端口
	static bool talkRepeat;             // 是否缩短重复发言间隔
	static int talkTime;                // 发言冷却时间（毫秒）
};
