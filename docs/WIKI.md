# BeiDou-ijl15 开发 Wiki

> 面向后续修改开发的完整技术文档  
> 仓库：`BeiDou-ijl15`（分支 `BeiDou`）  
> 内部工程名：**ezorsia**（上游为 RageZone 社区宽屏补丁）  
> 目标客户端：MapleStory **v83**（地址硬编码，特定客户端构建）  
> 许可证：AGPL v3  
> 文档日期：2026-09-09

---

## 目录

1. [项目定位](#1-项目定位)
2. [架构总览](#2-架构总览)
3. [目录结构](#3-目录结构)
4. [构建与部署](#4-构建与部署)
5. [启动与初始化流程](#5-启动与初始化流程)
6. [三种补丁机制](#6-三种补丁机制)
7. [核心模块详解](#7-核心模块详解)
8. [config.ini 配置参考](#8-configini-配置参考)
9. [地址表 AddyLocations.h](#9-地址表-addylocationsh)
10. [Codecave 函数索引](#10-codecave-函数索引)
11. [Detours Hook 索引](#11-detours-hook-索引)
12. [字符串池汉化](#12-字符串池汉化)
13. [BeiDou 专属功能](#13-beidou-专属功能)
14. [MapleClientCollectionTypes](#14-mapleclientcollectiontypes)
15. [开发指南：如何添加新补丁](#15-开发指南如何添加新补丁)
16. [调试与排错](#16-调试与排错)
17. [已知限制与风险](#17-已知限制与风险)
18. [相关资源](#18-相关资源)

**踩坑专题**：[汉化与编译踩坑记录](./汉化与编译踩坑记录.md)（乱码、编码、SwitchChinese、DLL 验证）

---

## 1. 项目定位

### 1.1 一句话说明

BeiDou-ijl15 是一个以 **`ijl15.dll` 代理 DLL** 方式在进程最早期注入 MapleStory v83 客户端的 **32 位 C++ 内存补丁工程**，综合使用：

- **Microsoft Detours** 函数钩子
- **naked 汇编 Code Cave**（代码洞）
- **绝对地址内存写入**

为 [北斗 BeiDou](https://github.com/SleepNap/BeiDou) 私服提供宽屏化、中文化、私服协议适配与属性上限突破等功能。

### 1.2 与上游 ezorsia 的关系

| 项目 | 说明 |
|------|------|
| 上游 | RageZone 社区 444Ro666 的 ezorsia 宽屏补丁 |
| 本分支 | 继承上游 **v1**，**不与 main/v2 同步** |
| 分支原因 | v2 在部分机器登录界面崩溃，v1 稳定 |
| 注入路线 | 上游 v2 曾用 `dinput8` / `nmconew`；本分支改回 **纯 ijl15 代理** |
| 相对 main | 领先约 69 个提交，新增 IME/MAC 修复/BossHP 等北斗定制功能 |

### 1.3 代码规模

| 指标 | 数值 |
|------|------|
| 自有 `.cpp` | 11 个 |
| 自有 `.h` | 35 个（含 MapleClientCollectionTypes） |
| 总行数 | 约 8.5k 行（含第三方头） |
| 最大文件 | `ReplacementFuncs.h`（~2174 行，汉化表占近 1900 行） |
| 编译单元 | 10 个 `.cpp` |

---

## 2. 架构总览

```
MapleStory.exe 启动
       │
       ▼
加载 ijl15.dll（代理 DLL，本工程产物）
       │
       ├── DllMain(DLL_PROCESS_ATTACH)
       │     ├── 读取 config.ini → Client 静态字段
       │     ├── DNS 解析 ServerIP_Address → IPv4
       │     ├── 安装 Detours Hooks（ReplacementFuncs.h 等）
       │     ├── Client::UpdateGameStartup()   服务器 IP、属性上限
       │     ├── Client::UpdateResolution()      分辨率 + 200+ 地址补丁
       │     ├── Client::FixMouseWheel() 等北斗功能
       │     ├── BossHP::Hook()
       │     └── ijl15::CreateHook()             转发到 2ijl15.dll
       │
       └── 游戏正常运行，所有 JPEG 调用转发给原库
```

### 补丁分层

```
┌─────────────────────────────────────────┐
│  config.ini（用户可配置开关）              │
├─────────────────────────────────────────┤
│  dllmain.cpp（编排入口）                  │
├──────────────┬──────────────┬───────────┤
│ Replacement  │  Client.cpp  │ 独立模块   │
│ Funcs.h      │              │ BossHP    │
│ (Detours)    │ (内存写入)    │ HpMpAlert │
├──────────────┴──────────────┴───────────┤
│  codecaves.h（naked asm 代码洞）          │
├─────────────────────────────────────────┤
│  Memory.cpp（VirtualProtect + 写入）     │
├─────────────────────────────────────────┤
│  MapleStory.exe 内存（硬编码地址）         │
└─────────────────────────────────────────┘
```

---

## 3. 目录结构

```
BeiDou-ijl15/
├── ezorsia.sln                 # VS 解决方案
├── README.md                   # 使用说明
├── CONTRIBUTING.md             # 贡献规范
├── LICENSE                     # AGPL v3
├── docs/
│   └── WIKI.md                 # 本文档
└── ezorsia/                    # 主工程目录
    ├── dllmain.cpp             # DLL 入口、配置加载、Hook 编排
    ├── ijl15.cpp / .h          # JPEG 库代理（6 个导出函数）
    ├── Client.cpp / .h         # 核心补丁逻辑
    ├── Memory.cpp / .h         # 内存读写、CodeCave、SetHook 封装
    ├── AddyLocations.h         # 所有补丁目标地址常量
    ├── codecaves.h             # naked asm 代码洞（~1484 行）
    ├── ReplacementFuncs.h      # Detours Hook + 汉化字符串表（~2174 行）
    ├── AutoTypes.h             # 客户端函数 typedef、Hook 桩
    ├── config.ini              # 默认配置（构建时复制到 out/）
    ├── FixIme.h                # 中文 IME 修复（header-only）
    ├── FixBuddy.h              # 好友接受包修复（header-only）
    ├── BossHP.cpp / .h         # Boss 血量百分比显示
    ├── HpMpAlert.cpp / .h      # HP/MP 警报阈值同步
    ├── SelectCharMacFix.cpp/.h # 选角 MAC 地址修复
    ├── NMCO.cpp / .h           # nmconew 代理（未启用）
    ├── INIReader.h             # 头文件 INI 解析器
    ├── detours.h               # Microsoft Detours 4.0.1
    ├── MapleClientCollectionTypes/  # 客户端内部类型复刻
    │   ├── ZXString.h          # 引用计数字符串
    │   ├── ZArray.h / ZList.h / ZMap.h
    │   ├── ZRef.h / ZRefCounted*.h
    │   ├── ZAllocEx.cpp/.h     # 客户端堆分配器
    │   └── ...
    └── ezorsia.vcxproj         # MSVC 项目文件
```

---

## 4. 构建与部署

### 4.1 环境要求

| 项目 | 值 |
|------|-----|
| IDE | Visual Studio **2019** |
| 平台工具集 | **v142** |
| Windows SDK | 10.0 |
| 目标配置 | **Release / Win32 (x86)** ← 必须 |
| 输出类型 | DynamicLibrary |
| 输出文件名 | `ijl15.dll` |
| 输出目录 | `out\Release\ijl15.dll` |
| 字符集 | Unicode |
| 运行库 | `/MD` (MultiThreadedDLL) |
| 链接库 | `detours.lib`、`imm32.lib`、`ws2_32.lib` |

### 4.2 构建步骤

1. 用 VS 2019 打开 `ezorsia.sln`
2. 选择配置 **Release | x86**
3. 生成解决方案
4. 产物位于 `out\Release\ijl15.dll`，`config.ini` 自动复制到同目录

### 4.3 部署步骤

```
客户端目录/
├── MapleStory.exe
├── 2ijl15.dll          ← 原 ijl15.dll 重命名
├── ijl15.dll           ← 本工程编译产物
└── config.ini          ← 从项目根目录复制
```

### 4.4 为什么劫持 ijl15.dll

Intel JPEG Library 1.5 的 `ijl15.dll` 是 `MapleStory.exe` 的**静态导入项**。代理 DLL 放在客户端目录后，Windows 加载器在**进程启动最早期**自动加载——无需启动器。

代理导出 6 个函数，全部 `__declspec(naked)` + 单条 `jmp` 转发给 `2ijl15.dll`：

| 导出函数 | 作用 |
|----------|------|
| `ijlGetLibVersion` | 版本查询 |
| `ijlInit` | 初始化 |
| `ijlFree` | 释放 |
| `ijlRead` | 读取 JPEG |
| `ijlWrite` | 写入 JPEG |
| `ijlErrorStr` | 错误字符串 |

---

## 5. 启动与初始化流程

`dllmain.cpp` 中 `DLL_PROCESS_ATTACH` 的完整执行顺序：

```
1. INIReader 解析 config.ini → Client::* 静态字段
2. ResolveToIpv4String() 解析 ServerIP_Address（支持域名）
3. ── Detours Hooks ──
   Hook_CreateMutexA(true)           多开
   HookCreateWindowExA(true)           最小化按钮 + 窗口居中
   HookGetModuleFileName(true)         路径修复
   HookPcCreateObject_IWzResMan(true)  Wz 资源管理（透传）
   HookPcCreateObject_IWzNameSpace(true)
   HookPcCreateObject_IWzFileSystem(true)
   HookCWvsApp__Dir_BackSlashToSlash(true)
   HookCWvsApp__Dir_upDir(true)
   Hookbstr_ctor(true)
   HookIWzFileSystem__Init(true)
   HookIWzNameSpace__Mount(true)
   HookCWvsApp__InitializeResMan(false)  实验性，已禁用
   Hook_StringPool__GetString(true)      汉化 + UI 路径
   Hook_lpfn_NextLevel(true)             自定义 EXP 表
   HookSaveGlobal(true)                  HpMp 警报发送
   HookHpMpAlertRecv(true)               HpMp 警报接收
   HookSelectCharMacFix(true)            MAC 修复
4. Client::UpdateGameStartup()    服务器 IP、属性上限、中文字符串
5. Client::UpdateResolution()     分辨率补丁（最大函数，~540 行）
6. Client::FixMouseWheel()
7. Client::Chinese()              IME + 好友 + 中文字体
8. Client::LongQuickSlot()        8→26 快捷键栏
9. Client::FixDateFormat()
10. Client::FixItemType()
11. Client::JumpCap()
12. Client::FixChatPosHook()
13. Client::NoPassword()
14. Client::MoreHook()
15. BossHP::Hook()
16. Client::WorldMap()
17. Client::RefreshRate()
18. Client::DeleteChar()
19. ijl15::CreateHook()           设置 JPEG 代理
    // NMCO::CreateHook() 已注释
```

`DLL_PROCESS_DETACH` 直接调用 `ExitProcess(0)`，不支持干净卸载。

---

## 6. 三种补丁机制

### 6.1 直接内存写入

通过 `Memory::WriteInt/WriteByte/WriteDouble/WriteString/FillBytes/PatchNop` 修改客户端内存中的立即数或字符串。

```cpp
// 示例：修改分辨率宽度
Memory::WriteInt(dwApplicationWidth + 1, m_nGameWidth);

// 示例：NOP 掉一段代码
Memory::PatchNop(0x008E4252, 2);
```

- `UseVirtuProtect=true` 时自动 `VirtualProtect` 后再写入
- 适用于修改常量、字符串、单字节标志

### 6.2 Code Cave（代码洞）

通过 `Memory::CodeCave(func, addr, nNOPs)` 在目标地址写入 `0xE9`（near jmp）跳转到自定义 naked 函数：

```cpp
void Memory::CodeCave(void* ptrCodeCave, DWORD dwOriginAddress, int nNOPCount) {
    if (nNOPCount) FillBytes(dwOriginAddress, 0x90, nNOPCount);  // NOP 腾出空间
    WriteByte(dwOriginAddress, 0xe9);                               // jmp 指令
    WriteInt(dwOriginAddress + 1, (int)(((int)ptrCodeCave - (int)dwOriginAddress) - 5);
}
```

- 需要至少 **5 字节**空间（1 字节 jmp + 4 字节相对偏移）
- naked 函数中用 `__asm` 操作寄存器后 `jmp` 回原函数
- 适用于 UI 坐标注入、条件分支修改、复杂逻辑

### 6.3 Detours 函数钩子

通过 `Memory::SetHook(attach, &original, detour)` 替换整个函数：

```cpp
bool Memory::SetHook(bool attach, void** ptrTarget, void* ptrDetour) {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    attach ? DetourAttach(ptrTarget, ptrDetour) : DetourDetach(ptrTarget, ptrDetour);
    DetourTransactionCommit();
}
```

- 适用于拦截 Win32 API 或客户端函数调用
- Hook 内可调用 `original` 函数
- 需要正确的调用约定（客户端多用 `__fastcall`）

### 6.2 选择指南

| 场景 | 推荐机制 |
|------|----------|
| 改一个数值常量 | 直接内存写入 |
| 改 UI 坐标/条件跳转 | Code Cave |
| 拦截函数、修改返回值 | Detours |
| 替换整个客户端函数 | Detours + 复刻客户端类型 |
| 注入自定义汇编逻辑 | Code Cave |

---

## 7. 核心模块详解

### 7.1 Memory（`Memory.cpp` / `Memory.h`）

所有补丁的底层工具类：

| 方法 | 作用 |
|------|------|
| `SetHook` | Detours 封装 |
| `CodeCave` | 写入 E9 跳转 |
| `WriteInt/Byte/Short/Double/String` | 类型化内存写入 |
| `WriteByteArray` | 写入字节数组 |
| `FillBytes` | 填充指定字节 |
| `ReplaceString` | 替换字符串（要求新串 ≤ 旧串长度） |
| `PatchNop` | 填充 NOP (0x90) |
| `UseVirtuProtect` | 全局开关，控制是否 VirtualProtect |

### 7.2 Client（`Client.cpp` / `Client.h`）

核心补丁编排类，所有 `static` 方法在 `DllMain` 中按序调用。

#### `UpdateGameStartup()`

| 补丁项 | 地址/机制 | 说明 |
|--------|-----------|------|
| UAC 提权移除 | `0x00C08459` 等 | 效果有限（DLL 内无法改 manifest） |
| 服务器 IP | `0x00AFE084` ×3 | 写入 config 中的 IP |
| 服务器端口 | `0x007519C1` | `serverIP_Port` |
| 物攻/魔攻/命中/回避上限 | 多个地址 | 见 config.ini |
| 输出显示上限 | `0x00AFE8A0` (double) | `setAtkOutCap` |
| 移速上限 | 3 个地址 | `speedMovementCap` |
| useTubi | `0x00485C32` NOP | 移除 AP/SP 停顿 |
| 中文换行修复 | `0x008E4252` NOP | 道具介绍 |
| 技能 tooltip | `0x008F383E` codecave | GBK 换行 |
| 报错信息编码 | `0x0068DE1F` 等 | 中文字节 |

#### `UpdateResolution()`（~540 行）

分辨率补丁的核心函数，修改 **200+ 地址**：

```
输入: Client::m_nGameWidth, Client::m_nGameHeight
计算: nStatusBarY = height - 578

补丁范围:
├── Gr2D 初始化（宽/高 push 值）
├── 光标向量、鼠标限制、视口
├── Tooltip 边界（防止超出窗口）
├── 状态栏（3 个 codecave）
├── 快捷栏、Buff 图标、扩音器
├── 登录/选角界面
├── 商城（8 个 codecave 居中）
├── 交易中心 ITC（8 个 codecave 居中）
├── 武陵道场 UI（12 个 codecave）
├── 组队/公会弹窗
├── Boss 血条宽度
├── 暗图圆圈居中
├── 截图缓冲区大小
├── 窗口模式 / 跳过 Logo（条件分支）
├── 消息数量（MsgAmount codecave）
└── 登录框布局（CustomLoginFrame / bigLoginFrame）
```

#### 其他 Client 方法

| 方法 | 功能 | 关键地址 |
|------|------|----------|
| `FixMouseWheel` | 修复滚轮乱飞 | `0x009E8090` codecave |
| `Chinese` | IME + 好友 + 中文字体 | `FixIme.h`, `FixBuddy.h` |
| `LongQuickSlot` | 8→26 快捷键栏 | 20+ 地址 + 5 codecave |
| `FixDateFormat` | 中文日期顺序 | 4 codecave（需 SwitchChinese） |
| `FixItemType` | 道具分类字符串修复 | 2 codecave（防 WZ 汉化崩溃） |
| `JumpCap` | 跳跃上限 + 爬绳速度 | 3 codecave + `0x00C1CF80` |
| `FixChatPosHook` | 聊天文字垂直位置 | `0x008DD06F` codecave |
| `NoPassword` | 免密模式 | `0x00620F2F`（需 debug+noPassword） |
| `MoreHook` | 属性面板/聊天/喇叭/窗口位置 | 多个地址 |
| `WorldMap` | 大地图上限 + 居中 | `0x009EA030`, `0x009EB594` |
| `RefreshRate` | 刷新率限制 60Hz | Detour `0x009FB0E9` |
| `DeleteChar` | 绕过 PIN 删角色 | Detour `0x005F7C4A` |

未从 DllMain 调用的方法：
- `EnableNewIGCipher()` — 加密相关，未使用
- `UpdateLogin()` — 登录 UI 定位，已弃用

### 7.3 ReplacementFuncs.h

最大的单文件，包含：

1. **Detours Hook 实现**（Win32 API + 客户端函数）
2. **`newKeyValuePairs[]`** 汉化字符串表（~900 条，key 11–5639）
3. **`myArrayForCustomEXP[]`** 自定义升级经验表（~200 级）

### 7.4 ijl15 代理（`ijl15.cpp`）

```cpp
void ijl15::CreateHook() {
    HMODULE hModule = LoadLibraryA("2ijl15.dll");
    // 解析 6 个导出 → 设置全局 FARPROC
}
// 每个导出: __declspec(naked) void ijlXxx() { __asm jmp dword ptr[ijlXxx_Proc] }
```

### 7.5 NMCO 代理（`NMCO.cpp`，未启用）

与 ijl15 相同模式的 `nmconew2.dll` 代理，DllMain 中已注释 `NMCO::CreateHook()`。

---

## 8. config.ini 配置参考

### `[general]`

| 键 | 默认值 | 说明 |
|----|--------|------|
| `imeType` | `1` | `0`=旧 IME 方案, `1`=新方案（推荐，Win11 兼容） |
| `width` | `1280` | 游戏宽度 |
| `height` | `720` | 游戏高度 |
| `ServerIP_Address` | `127.0.0.1` | 服务器 IP 或域名 |
| `serverIP_Port` | `8484` | 登录端口 |
| `SwitchChinese` | `true` | 字符串池汉化 + 日期/道具修复 |
| `MsgAmount` | `10` | 右下角拾取/经验消息数量 |
| `EzorsiaV2WzIncluded` | `true` | 使用 MapleEzorsiaV2wzfiles.img 登录/商城背景 |
| `CustomLoginFrame` | `true` | 分辨率特定登录框 |
| `bigLoginFrame` | `true` | 大边框登录布局 |
| `WindowedMode` | `true` | 强制窗口模式 |
| `RemoveLogos` | `true` | 跳过启动动画 |
| `UseVirtuProtect` | `true` | 内存保护（无 CRC bypass 的客户端必须开启） |

### `[optional]`

| 键 | 默认值 | 说明 |
|----|--------|------|
| `setDamageCap` | `199999` | 物理攻击面板上限 |
| `setMAtkCap` | `1999` | 魔攻/魔防面板上限 |
| `setAccCap` | `999` | 命中上限 |
| `setAvdCap` | `999` | 回避上限 |
| `setAtkOutCap` | `199999` | 伤害显示上限（double） |
| `speedMovementCap` | `140` | 移速上限 |
| `jumpCap` | `123` | 跳跃上限 |
| `climbSpeedAuto` | `false` | 爬绳速度随移速变化 |
| `climbSpeed` | `1.0` | 固定爬绳速度系数（×3 基础） |
| `useTubi` | `true` | 移除 AP/SP 停顿 + 宠物攻击冻结 |
| `ownLoginFrame` | `false` | 使用自定义登录框（跳过 Ezorsia 覆盖） |
| `ownCashShopFrame` | `false` | 使用自定义商城背景 |
| `talkRepeat` | `false` | 允许重复发言 |
| `talkTime` | `2000` | 连续发言间隔（毫秒） |

### `[debug]`

| 键 | 默认值 | 说明 |
|----|--------|------|
| `debug` | `false` | 启用调试补丁 |
| `noPassword` | `false` | 免密模式（需 `debug=true`，且服务端支持） |

---

## 9. 地址表 AddyLocations.h

所有补丁目标地址的集中定义。每个条目通常包含：

```cpp
const DWORD dwXXX = 0x00XXXXXX;        // 补丁写入地址
const DWORD dwXXXRetn = 0x00XXXXXX;    // codecave 返回地址
const int XXXNOPs = N;                 // 需要 NOP 的字节数
```

### 关键地址速查

| 地址 | 功能 |
|------|------|
| `0x0079E993` | `_StringPool__GetString` |
| `0x0078C8A6` | `_lpfn_NextLevel`（EXP 表） |
| `0x0049637B` | `SendPacket` |
| `0x004965F1` | `ProcessPacket` |
| `0x0049C8E7` | `SaveGlobal` |
| `0x005F7C4A` | `SendDeleteCharPacket` |
| `0x009FB0E9` | `PcCreateObject_IWzPackage` |
| `0x005336CA` | `CField::ShowMobHPTag` |
| `0x0094A144` | `CUserLocal::Update` |
| `0x00AFE084` | 服务器 IP 字符串（3 份） |
| `0x009F7B1D` / `0x009F7B23` | Application Height / Width |
| `0x007519C1` | 登录端口 |

### 地址分组

| 分组 | 地址范围 | 用途 |
|------|----------|------|
| 分辨率核心 | `0x009F7Bxx`, `0x0059Axxx` | 宽/高、光标、视口 |
| 状态栏 | `0x008CFDxx`–`0x008D21xx` | Y 坐标 codecave |
| 商城 | `0x004694xx` | 8 个居中 codecave |
| 交易中心 | `0x0059E9xx`–`0x0059EAxx` | 8 个居中 codecave |
| 登录 UI | `0x0060Dxxx`–`0x0062Bxxx` | 背景/描述/版本号 |
| 武陵道场 | `0x00554xxx` | 12 个 UI 偏移 |
| 加密参考 | `0x00A4A8xx` | IGCipher（未使用） |
| 打包客户端 | `0x0044xxxx`–`0x00A4xxxx` | CRC bypass 桩（注释状态） |

---

## 10. Codecave 函数索引

### 活跃（被 Client 调用）

| 函数 | 补丁地址 | 用途 |
|------|----------|------|
| `AdjustStatusBar` | `dwStatusBarVPos` | 状态栏 Y |
| `AdjustStatusBarBG` | `dwStatusBarBackgroundVPos` | 状态栏背景 |
| `AdjustStatusBarInput` | `dwStatusBarInputVPos` | 聊天输入区 |
| `CashShopFix1`–`8` | `dwCashFix1`–`8` | 商城面板居中 |
| `CashShopFixOnOff` | `dwCashFixOnOff` | 商城预览按钮 |
| `CashShopFixPrev` | `dwCashFixPrev` | 商城预览窗口 |
| `ITCFix1`–`8` | `0x0059E9E1`–`0x0059EAAE` | 交易中心居中 |
| `VersionNumberFix` | `dwVersionNumberFix` | 登录版本号位置 |
| `ccLoginBackCanvasFix` | `dwLoginBackCanvasFix` | 世界选择背景 |
| `ccLoginViewRecFix` | `dwLoginViewRecFix` | 世界 ViewRec |
| `ccLoginDescriptorFix` | `dwLoginDescriptorFix` | 世界标签动画 |
| `ccMoreGainMsgs` / `Fade` / `Fade1` | `dwMoreGainMsgs*` | 更多拾取消息 |
| `ccMuruengraid*` (12) | `0x00554xxx` | 武陵道场 UI |
| `fixMouseWheelHook` | `0x009E8090` | 滚轮修复 |
| `CompareValidateFuncKeyMappedInfo_cave` | `0x8DD8B8` | 长快捷键验证 |
| `sub_9FA0CB_cave` | `0x9FA0DB` | 快捷键分配大小 |
| `sDefaultQuickslotKeyMap_cave` | `0x72B7BC` | 默认键位（26 槽） |
| `DefaultQuickslotKeyMap_cave` | `0x72B8E6` | 键位初始化 |
| `Restore_Array_Expanded` | `0x008CFDFD` | 技能冷却数组重置 |
| `fixDateFormat`–`4` | `0x008EBF57` 等 | 中文日期顺序 |
| `getItemType1` / `2` | `0x005CFA99` / `0x005CFAC2` | 道具分类字符串 |
| `customJumpCapHook1`–`3` | `0x00780797` 等 | 跳跃上限 |
| `chatTextPos` | `0x008DD06F` | 聊天文字位置 |
| `calcSpeedHook` | `0x0094D93C` | 自适应爬绳速度 |
| `faceHairCave` | `0x005C94F3` | 脸型/发型创建 |
| `canSendPkgTimeCave` | `0x00485C28` | 发包冷却 200ms |
| `apDetailBtn` | `0x008C4E1B` | 属性详情按钮 |
| `darkMap1cc`–`3cc` | `0x0055BEE6` 等 | 暗图圆圈居中 |
| `wordMapUIcc` | `0x009EB594` | 大地图居中 |
| `skillToolTipNew` | `0x008F383E` | GBK 技能 tooltip |

### 来自其他头文件

| 函数 | 文件 | 补丁地址 | 用途 |
|------|------|----------|------|
| `setOnFocusFirstJudgement` | FixIme.h | — | IME 焦点判断 |
| `switchIme` / `switchMLIme` | FixIme.h | — | IME 切换 |
| `newSwitchIme` / `newSwitchMLIme` | FixIme.h | — | 新 IME 方案 |
| `fixBuddyAccept` | FixBuddy.h | `0x0053129E` | 好友接受中文名 |

### 已定义但未使用

- `PositionLoginDlg/Username/Password` — `UpdateLogin()` 未调用
- `PositionBossBarY/Y1/Y2` — 已注释
- `cc0x0044xxxx` / `cc0x009Fxxxx` / `cc0x00A4xxxx` — 打包客户端 CRC bypass 参考
- `testingCodeCave` 1–4 — 测试用
- `AlwaysViewRestoreFix` — 作者标注"currently does nothing"

---

## 11. Detours Hook 索引

### Win32 API

| Hook 函数 | 目标 | 作用 |
|-----------|------|------|
| `Hook_CreateMutexA` | `CreateMutexA` | 多开（`WvsClientMtx` 返回假句柄） |
| `HookCreateWindowExA` | `CreateWindowExA` | 最小化按钮 + 窗口居中 |
| `HookGetModuleFileName` | `GetModuleFileNameW` | 路径获取失败时重试 |

### 客户端函数

| Hook 函数 | 地址 | 作用 |
|-----------|------|------|
| `Hook_StringPool__GetString` | `0x0079E993` | 汉化 + UI 资源路径 |
| `Hook_lpfn_NextLevel` | `0x0078C8A6` | 自定义 EXP 表 |
| `HookPcCreateObject_IWzResMan` | — | Wz 资源管理（透传） |
| `HookPcCreateObject_IWzNameSpace` | — | Wz 命名空间（透传） |
| `HookPcCreateObject_IWzFileSystem` | — | Wz 文件系统（透传） |
| `HookCWvsApp__Dir_BackSlashToSlash` | — | 路径反斜杠转换 |
| `HookCWvsApp__Dir_upDir` | — | 上级目录 |
| `Hookbstr_ctor` | — | BSTR 构造 |
| `HookIWzFileSystem__Init` | — | 文件系统初始化 |
| `HookIWzNameSpace__Mount` | — | 命名空间挂载 |
| `HookCWvsApp__InitializeResMan` | — | 资源管理器（**已禁用**） |

### 独立模块 Hook

| 模块 | Hook 目标 | 地址 | 作用 |
|------|-----------|------|------|
| BossHP | `CField::ShowMobHPTag` | `0x005336CA` | 捕获 Boss HP |
| BossHP | `CUserLocal::Update` | `0x0094A144` | 绘制百分比 |
| BossHP | `CField::Init/Dispose` | `0x00528DBC`/`0x00529035` | Tooltip 生命周期 |
| HpMpAlert | `SaveGlobal` | `0x0049C8E7` | 保存时发送阈值 |
| HpMpAlert | `ProcessPacket` | `0x004965F1` | 接收阈值同步 |
| SelectCharMacFix | `SendPacket` | `0x0049637B` | 重写选角 MAC |
| Client | `SendDeleteCharPacket` | `0x005F7C4A` | 绕过 PIN 删角色 |
| Client | `PcCreateObject_IWzPackage` | `0x009FB0E9` | 刷新率限制 |

---

## 12. 字符串池汉化

### 机制

`Hook_StringPool__GetString` 拦截客户端 `_StringPool__GetString`，按字符串索引 `nIdx` 返回自定义文本。

### 特殊索引

| 索引 | 返回值 | 条件 |
|------|--------|------|
| `1163` | `"BeiDou"` | 始终 |
| `1307` | `UI/MapleEzorsiaV2wzfiles.img/Common/frame{W}` | `EzorsiaV2WzIncluded && !ownLoginFrame` |
| `1301` | `UI/MapleEzorsiaV2wzfiles.img/Base/backgrnd` | `EzorsiaV2WzIncluded && !ownCashShopFrame` |
| `1302` | `.../backgrnd1` | 同上 |
| `5361` | `.../backgrnd2` | 同上 |

### 汉化表 `newKeyValuePairs[]`

当 `SwitchChinese=true` 时，遍历约 **900 条** `{key, value}` 替换字符串池内容。

| 分类 | 索引范围（示例） | 内容 |
|------|------------------|------|
| 职业名称 | 11–63 | 战士、魔法师等 |
| 登录/错误消息 | 65–93 | 连接失败、密码错误等 |
| 聊天命令 | 122–146 | `/好友`、`/交易` 等 |
| 组队/公会/家族 | 310–365 | 组队邀请、公会管理等 |
| 商城/道具/装备 | 417–647 | 道具类型、装备栏等 |
| 提示/属性/日期 | 655–679 | tooltip、属性名等 |
| 好友/结婚/举报 | 708–3007 | 社交系统 |
| MapleTV/婚礼/学院 | 3960–4638 | 活动系统 |
| 玩家商店/宠物/技能 | 3460–3607 | 经济/战斗 |
| GM 消息/反作弊 | 3140–3181 | 管理消息 |
| 用户协议 | 3029–3044 | EULA 文本 |

> **注意（重要）**：`ReplacementFuncs.h` 必须以 **GBK 单字节**保存，编辑器中汉化条目显示为 `Ã°ÏÕµº` 一类字符是**正常现象**。若被存成 UTF-8 后重编译，游戏里动态文字会全部乱码。  
> 详见 **[汉化与编译踩坑记录](./汉化与编译踩坑记录.md)**。  
> 工程 `Release|Win32` 已配置 `/source-charset:.936 /execution-charset:.936`；`Client.cpp` 带 UTF-8 BOM，不受影响。

### 自定义 EXP 表

`myArrayForCustomEXP[]` 定义约 200 级的升级经验，通过 `Hook_lpfn_NextLevel` 注入。**必须与服务端经验表一致**。

---

## 13. BeiDou 专属功能

### 13.1 中文 IME（`FixIme.h`）

| 方案 | `imeType` | 特点 |
|------|-----------|------|
| 旧方案 | `0` | 焦点切换 IME；商城赠礼字段有问题 |
| 新方案 | `1` | 密码框禁用 IME；多行始终启用；Win11 兼容 |

通用修复（两种方案共用）：
- NOP 聊天/按键/群组消息的 IME 阻塞
- 启用剪贴板中文
- 移除角色名 ASCII-only 检查

### 13.2 好友中文名（`FixBuddy.h`）

Codecave @ `0x0053129E`：好友接受包中名字长度为 9 字节（3 个汉字）时，跳转到备用处理路径 `0x005312D5`。

### 13.3 Boss HP 百分比（`BossHP.cpp`）

```
CField::ShowMobHPTag → 记录 nHP/nMaxHP → 计算百分比
CUserLocal::Update   → 在迷你地图 X+37 处绘制 "XX.XX%" tooltip
CField::Init/Dispose → tooltip 生命周期管理
```

### 13.4 HP/MP 警报同步（`HpMpAlert.cpp`）

自定义协议 **opcode `0x1000`**：

```
客户端保存设置 (SaveGlobal)
  → 读取 CUIStatusBar HP/MP 警报阈值 (0x00BEBF9C + 0x80/0x84)
  → SendPacket 发送 [0x1000, hpAlert, mpAlert]

服务端回传 (ProcessPacket)
  → 解析 opcode 0x1000
  → 写回 CUIStatusBar 阈值
```

阈值范围 0–20，**需要服务端支持**。

### 13.5 选角 MAC 修复（`SelectCharMacFix.cpp`）

Hook `SendPacket` @ `0x0049637B`：

```
检测 opcode 0x0013（选角包）
  → 从 hostname 后缀解析 12 位 hex
  → 格式化为 XX-XX-XX-XX-XX-XX
  → 缩短 MAC 字段，修正包长度
```

修复中文环境下 MAC 地址格式导致的选角问题（卡门）。

### 13.6 长快捷键栏（`LongQuickSlot`）

将默认 8 格快捷键扩展到 **26 格**：

- 修改绘制/验证/键位映射中的 `cmp 8` → `cmp 26`（0x1A）
- 注入扩展数组 `Array_Expanded`、`Array_aDefaultQKM_0` 等到客户端内存
- 5 个 codecave 处理验证和初始化

### 13.7 道具分类修复（`FixItemType`）

不修改 WZ 的情况下解决汉化后崩溃：
- Eqp/Etc 汉化后游戏崩溃
- Use 汉化后吃药没声音

通过 codecave 重定向道具分类的字符串 ID。

### 13.8 免密删角色（`DeleteChar`）

Detour `SendDeleteCharPacket` @ `0x005F7C4A`，发送空密码的删角色包，绕过 PIN 对话框。

### 13.9 其他北斗调整

- 聊天框文字垂直位置（`FixChatPosHook`）
- 有效期日期顺序（`FixDateFormat`）
- 装备/道具 tooltip 字体大小
- 魔攻/魔防/命中/回避/跳跃上限突破
- 交易中心居中（ITC codecave ×8）

---

## 14. MapleClientCollectionTypes

复刻 Nexon/Wizet 客户端内部 C++ 类型，使补丁代码能与客户端堆和调用约定互操作。

| 类型 | 用途 | 使用场景 |
|------|------|----------|
| `ZXString<T>` | 引用计数字符串 | 字符串池 Hook、DeleteChar 空密码 |
| `ZArray<T>` | 动态数组 | 客户端容器操作 |
| `ZList<T>` | 链表 | 客户端容器操作 |
| `ZMap<K,V>` | 映射表 | 客户端容器操作 |
| `ZRef<T>` | 智能指针 | 引用计数对象 |
| `ZRefCounted` | 引用计数基类 | 对象生命周期 |
| `ZAllocEx` | 客户端堆分配器 | **必须用此分配 DLL 侧字符串传给客户端** |
| `ZtlSecure<T>` | 加密数值 | 属性读取 |
| `TSecType<T>` | 安全类型 | 属性操作 |

> **关键规则**：通过 Hook 传给客户端的字符串/对象，必须使用 `ZAllocEx` 分配，否则客户端释放时会崩溃。

---

## 15. 开发指南：如何添加新补丁

### 15.1 贡献规范（CONTRIBUTING.md）

1. 补丁主要用于**分辨率缩放**体验
2. 非分辨率功能必须在 `config.ini` 中提供开关
3. 公开发布时，非分辨率功能默认**关闭**
4. 未知地址可作为注释放入 `AddyLocations.h`

### 15.2 标准开发流程

```
1. IDA/x64dbg 分析 v83 客户端，找到目标地址
2. 在 AddyLocations.h 添加地址常量（含 Retn、NOPs）
3. 选择补丁机制：
   ├── 简单常量 → Memory::WriteInt/Byte 放入 Client::UpdateGameStartup/UpdateResolution
   ├── 自定义逻辑 → codecaves.h 写 naked 函数 + Memory::CodeCave
   └── 函数拦截 → AutoTypes.h 定义 typedef → ReplacementFuncs.h 写 Hook → dllmain 注册
4. 如需配置开关 → config.ini 添加键 → dllmain 读取 → Client 中条件分支
5. Release x86 编译 → 部署测试（确保 2ijl15.dll 存在）
```

### 15.3 添加字符串汉化

在 `ReplacementFuncs.h` 的 `newKeyValuePairs[]` 中添加：

```cpp
{索引, "中文文本"},
```

或通过 `Hook_StringPool__GetString` 的 `switch(nIdx)` 添加特殊 case。

### 15.4 添加新分辨率支持

1. 在 `Hook_StringPool__GetString` 的 case `1307` 中添加新 `case` 分支
2. 在 `UpdateResolution()` 中检查是否有硬编码的 800/600 常量遗漏
3. 准备对应分辨率的 `MapleEzorsiaV2wzfiles.img/Common/frame{W}` UI 资源
4. 参考 `BeiDou-ijl15-resources/layouts/` 中的布局 JSON 进行 UI 坐标验证

### 15.5 编写 Codecave 模板

```cpp
// codecaves.h
DWORD myFuncRetn = 0x00XXXXXX;
__declspec(naked) void myFunc() {
    __asm {
        // 保存/修改寄存器
        push eax
        mov eax, Client::m_nGameWidth
        // ...
        pop eax
        jmp myFuncRetn    // 跳回原函数
    }
}

// Client.cpp
Memory::CodeCave(myFunc, dwMyFuncAddr, MyFuncNOPs);
```

### 15.6 编写 Detours Hook 模板

```cpp
// AutoTypes.h
typedef void(__fastcall* MyFunc_t)(void* pThis, void* edx, int param);
static auto _MyFunc = reinterpret_cast<MyFunc_t>(0x00XXXXXX);

// ReplacementFuncs.h
bool Hook_MyFunc(bool bEnable) {
    MyFunc_t Hook = [](void* pThis, void* edx, int param) -> void {
        // 自定义逻辑
        _MyFunc(pThis, edx, param);
    };
    return Memory::SetHook(bEnable, reinterpret_cast<void**>(&_MyFunc), Hook);
}

// dllmain.cpp
Hook_MyFunc(true);
```

---

## 16. 调试与排错

### 16.1 开启调试控制台

`dllmain.cpp` 中取消注释：

```cpp
CreateConsole();  // AllocConsole + freopen CONOUT$
```

已有 `std::cout` 输出分辨率信息和 Hook 状态。

### 16.2 常见问题

| 现象 | 可能原因 | 排查 |
|------|----------|------|
| 启动报 "Failed to find 2ijl15.dll" | 未重命名原 DLL | 确认 `2ijl15.dll` 存在 |
| 登录界面崩溃 | 地址不匹配 / 分辨率不支持 | 检查客户端版本是否为 v83 |
| 汉化乱码（动态文字花、静态 UI 正常） | `ReplacementFuncs.h` 被存成 UTF-8，DLL 字符串双重编码 | **不要**关 `SwitchChinese`；按 [踩坑记录](./汉化与编译踩坑记录.md) 转回 GBK 并重编译 |
| 界面变英文 | 误将 `SwitchChinese=false` 当作乱码修复 | 保持 `SwitchChinese=true`，修源文件编码 |
| DX8 启动失败 `0x80004005` | Win10/11 全屏模式 | `WindowedMode=true` + `ApplyNativeResolution()` |
| 补丁无效 | CRC 保护 | 开启 `UseVirtuProtect=true` |
| 卡门（选角后卡住） | MAC 格式 / IME | 检查 `SelectCharMacFix` 和 `imeType` |
| 滚轮乱飞 | 未加载补丁 | 确认 `FixMouseWheel` codecave 生效 |
| Tooltip 超出窗口 | 分辨率补丁遗漏 | 检查 `dwToolTipLimitH/VPos` |
| BossHP 不显示 | 地址偏移 | 验证 `0x005336CA` 等地址 |
| HpMp 同步无效 | 服务端未实现 | 需要 opcode `0x1000` 服务端支持 |

### 16.3 地址验证

所有地址针对特定 v83 客户端构建。如果客户端 EXE 被修改或更新：

1. 用 IDA Pro 打开客户端，验证关键函数地址
2. 对比 `AddyLocations.h` 中的地址
3. 使用项目中的 IDA MCP 工具辅助分析

---

## 17. 已知限制与风险

### 客户端兼容性

- 地址**硬编码**于特定 v83 构建，任何 EXE 更新都会破坏补丁
- 仅测试 **Release x86**，x64 不可用
- 分辨率除 1280×720 外不保证可用

### 功能限制

| 限制 | 说明 |
|------|------|
| UAC 提权 | DLL 内修改 manifest 无效，需要启动器 |
| `InitializeResMan` | 实验性 .img 加载，已禁用 |
| `AlwaysViewRestoreFix` | 作者标注无效 |
| 摄像头 VR 补丁 | 注释掉，可能导致崩溃 |
| NMCO 代理 | 未启用，仅 ijl15 代理活跃 |
| 旧 IME 方案 | 商城赠礼字段 IME 异常 |
| FixBuddy | 仅处理 9 字节（3 汉字）好友名 |
| 免密模式 | 需服务端配合 |
| DLL 卸载 | `ExitProcess(0)` 硬退出 |

### 代码质量

- 单文件过大（`ReplacementFuncs.h` 2174 行、`codecaves.h` 1484 行）
- 大量注释掉的历史代码和参考地址
- 汉化表线性查找（900 条），性能可优化为 hash map
- EXP 表硬编码，需手动与服务端同步

---

## 18. 相关资源

| 资源 | 路径/链接 | 说明 |
|------|-----------|------|
| 本仓库 | `BeiDou-ijl15/` | 补丁源码 |
| **汉化与编译踩坑记录** | `docs/汉化与编译踩坑记录.md` | 乱码/编码/SwitchChinese 专题 |
| 深度研究报告 | `../docs/BeiDou-ijl15-研究报告.md` | 技术分析报告 |
| UI 资源与布局 | `../BeiDou-ijl15-resources/` | 解码 WZ、布局 JSON、合成预览 |
| IDA 布局笔记 | `../BeiDou-ijl15-resources/docs/ida-layout-notes.md` | UI 坐标逆向笔记 |
| 北斗服务端 | https://github.com/SleepNap/BeiDou | 推荐配套服务端 |
| 上游 ezorsia | RageZone 444Ro666 | 原始宽屏补丁 |
| Harepacker | `../Harepacker-resurrected/` | WZ 文件编辑工具 |
| CustomExpTable | github.com/PurpleMadness/CustomExpTable | EXP 表参考 |
| Microsoft Detours | 4.0.1 | 函数 Hook 库 |

### UI 开发工作流

```
1. Harepacker 导出/编辑 UI.wz
2. BeiDou-ijl15-resources/tools/ 中的 Python 工具生成布局
3. compose-all.ps1 合成预览图对比
4. 在 codecaves.h / UpdateResolution 中调整坐标
5. 编译测试
```

---

## 附录 A：文件依赖关系

```
dllmain.cpp
├── INIReader.h
├── ReplacementFuncs.h
│   ├── AutoTypes.h
│   ├── Memory.h
│   └── MapleClientCollectionTypes/ZXString.h
├── Client.h → Client.cpp
│   ├── AddyLocations.h
│   ├── codecaves.h
│   ├── FixIme.h
│   └── FixBuddy.h
├── BossHP.h → BossHP.cpp
├── HpMpAlert.h → HpMpAlert.cpp
├── SelectCharMacFix.h → SelectCharMacFix.cpp
├── ijl15.h → ijl15.cpp
└── NMCO.h → NMCO.cpp (未启用)
```

## 附录 B：编译单元列表

| .cpp 文件 | 编译 | 说明 |
|-----------|------|------|
| `stdafx.cpp` | PCH | 预编译头 |
| `dllmain.cpp` | ✓ | 入口 |
| `Client.cpp` | ✓ | 核心补丁 |
| `Memory.cpp` | ✓ | 内存工具 |
| `ijl15.cpp` | ✓ | JPEG 代理 |
| `BossHP.cpp` | ✓ | Boss HP |
| `HpMpAlert.cpp` | ✓ | HP/MP 同步 |
| `SelectCharMacFix.cpp` | ✓ | MAC 修复 |
| `ZAllocEx.cpp` | ✓ | 分配器 |
| `NMCO.cpp` | ✓ | NMCO 代理（未链接调用） |

---

*本文档基于 2026-09-09 的 `BeiDou` 分支源码生成。后续开发时请同步更新此文档。*
