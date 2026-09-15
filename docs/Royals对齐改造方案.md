# Royals 对齐改造方案（草案）

> **状态：仅方案，未实施**  
> 日期：2026-09-09  
> 目标：向 MapleRoyals 靠拢——800/1024 双档 UI、游戏内分辨率切换、隐藏顶部滚动公告条

---

## 0. 现状摘要

| 项目 | 当前 `feature/native-800x600` |
|------|------------------------------|
| 分辨率 | **硬编码 800×600**，`ApplyNativeResolution()` |
| 宽屏补丁 | `UpdateResolution()` / `LongQuickSlot()` **未调用** |
| EzorsiaV2 WZ | `EzorsiaV2WzIncluded=false`，不走 `frame1280` |
| 客户端 UI 资源 | `client-assets` 仅 **Login.img + StatusBar.img**（800 档） |
| 游戏内分辨率 UI | **无**（原版 SysOpt 无分辨率行） |
| 顶部滚动条 | **未处理**（`CSlideNotice` 仍按原版显示） |

已有调研资料：

- `steps/extracted/ui_diff/SUMMARY.md` — Royals vs 北斗 img 差异
- `steps/extracted/ui_switch_design.md` — 800/1024 路径表与 Hook 设计
- `steps/11-BeiDou与kaentake对比与移植路线.md` — kaentake 移植路线
- `kaentake/src/resolution.cpp` — SysOpt ComboBox + 热切参考实现
- `kaentake-archived/src/uipath.cpp` — 1024 路径 UOL 改写
- `steps/SysOpt.backgrnd.ps2.png` — 你做的 **299×396** 中文系统设置底图（含「分辨率」行）

---

## 1. MapleRoyals 在 1024 用了哪些自定义 UI？

### 1.1 核心机制：同一 `.img` 内双套节点

Royals **不是**换整个客户端目录，而是在 `Data/UI/*.img` 里为 800 和 1024 各备一套素材，通过 **`1024` 前缀**区分：

```
800 档                    1024 档
─────────────────────────────────────────────
Login.img/Common/frame          → Common/1024frame          (800×600 → 1024×768)
CashShop.img/Base/backgrnd      → Base/1024backgrnd
StatusBar.img/base/quickSlot    → base/1024quickSlot        (均为 151×80，8 格)
StatusBar.img/base/backgrnd     → base/1024backgrnd         (800×71 → 875×71)
StatusBar.img/gauge/bar         → gauge/1024bar
StatusBar.img/EquipKey/...      → 1024EquipKey/...          (等 5 组按键，共 30 个 canvas)
UIWindow.img/muruengRaid/player → muruengRaid/1024player
```

**合计：44 个真正的 1024 节点**，分布在 **4 个 img 文件**：

| # | WZ 文件 | 1024 节点数 | 说明 |
|---|---------|------------|------|
| 1 | `Login.img` | 1 | `Common/1024frame` |
| 2 | `CashShop.img` | 1 | `Base/1024backgrnd` |
| 3 | `StatusBar.img` | 40 | base/gauge/Key 全套 |
| 4 | `UIWindow.img` | 2 | 木灵战 `1024monster` / `1024player` |

完整路径表见 `steps/extracted/ui_diff/SUMMARY.md`。

### 1.2 北斗与 Royals 的策略差异（为何要对齐）

| 方面 | MapleRoyals | 北斗（改造前） |
|------|-------------|----------------|
| 分辨率档 | 800 / **1024** | 800 / **1280×720** |
| 登录框 | `frame` + `1024frame` | `frame` 1280×720 + EzorsiaV2 `frame1280` |
| 快捷栏 | 8 格 + `1024quickSlot` | **26 格** `quickSlot26`（466px 宽） |
| 运行时换图 | 客户端改 UOL（逻辑在 Royals `ijl15.dll`，本仓库未开源） | StringPool 1307 或 `UpdateResolution` codecave |

### 1.3 与 EzorsiaV2 的关系

对齐 Royals 时建议 **停用** `MapleEzorsiaV2wzfiles.img` 的 StringPool 重定向（1307/1301/1302），改走 Royals 式 **原生 img 内 `1024*` 节点**。

`GuildMark/00001024`、颜色值 `-1024` 等与分辨率无关，**不要**误改。

---

## 2. 游戏内动态切换分辨率（参考 kaentake + 你的 SysOpt 图）

### 2.1 Royals 截图里的效果

- 系统设置底部有 **SCREEN RESOLUTION** 下拉（800×600 / 1024×768）
- 切换后窗口、StatusBar、视口随之变化
- 顶部无滚动广告条（见第 3 节）

### 2.2 kaentake 已实现的能力（可直接参考）

源码：`kaentake/src/resolution.cpp`（地址已与 BeiDou.exe 对齐，见 `steps/extracted/kaentake_ida_verify.md`）

| 模块 | 作用 |
|------|------|
| `CUISysOpt::OnCreate` @ `0x00994163` | 动态创建 `CCtrlComboBox`（ID 2000，位置 76,338，166×18） |
| `Patch4(0x009945BC+1, 372)` | OK/Cancel 按钮下移（原 y=342 → 372） |
| `CConfig::LoadGlobal/SaveGlobal` | 读写 `soScreenResolution` |
| `CConfig::ApplySysOpt_hook` | 点确定时调用 `set_screen_resolution()` |
| `set_screen_resolution()` | `CWzGr2D::ScreenResolution` + 视口/地图重载 |
| `set_stage_hook` | 进图时应用分辨率 |
| `CSlideNotice` 等 | 按当前宽度补丁（宽屏时拉满） |

**kaentake-archived（更贴近 Royals）** 仅 **2 档**：800×600、1024×768，并有 `uipath.cpp` 在 `get_screen_width() >= 1024` 时改写 UOL。

### 2.3 你的 `SysOpt.backgrnd.ps2.png`

| 属性 | 值 |
|------|-----|
| 尺寸 | **299×396**（比原版 365 高 **31px**，多一行「分辨率」） |
| 内容 | 中文标签 + 最后一行「分辨率」留白给 ComboBox |
| 优于 kaentake 默认 | 中文排版、与北斗汉化风格一致 |

**部署方式（二选一）：**

| 轨道 | 做法 | 优点 | 缺点 |
|------|------|------|------|
| **A. 改 UIWindow.img** | ImgPatch 已支持：`ReplaceCanvasPng(SysOpt/backgrnd)` | 不依赖 Custom.wz，与 Track A 一致 | 改的是 `Data/UI/UIWindow.img`，体积大 |
| **B. Custom.wz 覆盖** | kaentake `resman.cpp` 挂载 `Custom.wz` | 不动原版 UI.wz，易回滚 | 需启用 `HookCWvsApp__InitializeResMan(true)` |

设计文档 `ui_switch_design.md` 约定：**SysOpt 背景只用一套 396px**，不按分辨率换图；**禁止**用 Royals 456px 宽 SysOpt 覆盖。

### 2.4 与北斗 ezorsia 的合并难点（必读）

**不能**同时跑完整的 `UpdateResolution()`（1280 宽屏 + 26 格）和 kaentake `resolution.cpp`（另一套地址补丁）。

建议合并为单一入口：

```cpp
void Client::ApplyResolution(int w, int h);
// DllMain 启动时调用一次
// SysOpt ComboBox 确定时再调用
```

需要统一的决策：

| 决策点 | 建议（Royals 对齐） |
|--------|---------------------|
| 分辨率档位 | **仅 800×600、1024×768**（与 Royals WZ + 你的目标一致） |
| 快捷栏 | **8 格**，不调用 `LongQuickSlot()` |
| UI 路径 | `uipath.cpp` 或扩展 StringPool + `IWzResMan::GetObject` Hook |
| `0x009F7078` 冲突 | kaentake 写 1080 上限 vs ezorsia 写当前高度 → 合并时只保留一套 |
| 登录框换图 | 1024 档热切**不强制**；重登后生效可接受（Royals 同理） |

### 2.5 推荐实施阶段

```
阶段 1 — 资源（低风险）
  ├─ ImgPatch：合并 Royals 的 1024* 节点到 Login/CashShop/StatusBar/UIWindow
  ├─ 保留北斗汉化控件（Title 中文按钮等）
  ├─ SysOpt/backgrnd ← steps/SysOpt.backgrnd.ps2.png
  └─ 输出到 client-assets/，文档化部署步骤

阶段 2 — 分辨率 UI（中风险）
  ├─ 从 kaentake 移植 resolution.cpp（精简为 2 档）
  ├─ 部署 SysOpt 396px 底图（轨道 A 或 B）
  └─ config.ini 增加 HideSlideNotice 等开关（可选）

阶段 3 — 运行时换图（中高风险）
  ├─ 移植 uipath.cpp（GetObject Hook）
  └─ 或复用/扩展 ReplacementFuncs.h StringPool（1307 改为 frame/1024frame）

阶段 4 — 坐标补丁（高风险，按需）
  ├─ 从 UpdateResolution() 抽取 1024 需要的子集（StatusBar/聊天/商城居中）
  └─ 禁止整包 1280 codecave
```

**验收标准：**

- [ ] 系统设置出现分辨率下拉，选项 800×600 / 1024×768
- [ ] 切换后 StatusBar 底图、HP 条宽度与 Royals 截图一致
- [ ] 8 格快捷栏，无 26 格残影
- [ ] 登录框在 1024 下使用 `1024frame`（重登后）
- [ ] 汉化、IME、BossHP 等北斗功能不受影响

---

## 3. 隐藏顶部滚动广告条

### 3.1 是什么？

客户端类 **`CSlideNotice`** — 屏幕顶部横向滚动的服务器公告（ezorsia 注释：`server message at top`）。

相关地址（已在 `Client.cpp` / kaentake 中使用）：

| 地址 | 用途 |
|------|------|
| `0x007E15BE` | `CSlideNotice` 构造 / 窗口宽 |
| `0x007E16BE` | `OnCreate` 画布宽 |
| `0x007E1E07` | `SetMsg` 消息宽 |
| `0x007E19CA` | 相关显示逻辑 |

当前 `feature/native-800x600` 分支 **未调用** `UpdateResolution()`，这些补丁**未生效**；原版 800 宽公告条仍会显示。

### 3.2 Royals 为何看不到？

本仓库 **没有** Royals 的 `ijl15.dll` 源码，可能原因：

| 可能 | 说明 |
|------|------|
| **服务端不发公告包** | 私服关闭 `SlideNotice` 广播 |
| **客户端补丁隐藏** | 3MB Royals `ijl15.dll` 内 NOP/Hook（需 IDA 验证） |
| **进图时机** | 截图时恰好无活动公告 |

不能仅凭截图断定是客户端还是服务端行为。

### 3.3 北斗 ijl15 可行方案（待你确认后实施）

| 方案 | 做法 | 风险 | 推荐 |
|------|------|------|------|
| **A. 配置开关 + Hook** | `config.ini` `HideSlideNotice=true`；Hook `CSlideNotice::OnCreate` 直接 return 或 `SetVisible(false)` | 低 | ⭐ 首选 |
| **B. SetMsg 空实现** | Hook `SetMsg` 不渲染 | 低 | 备选 |
| **C. NOP 创建点** | IDA 找 `CSlideNotice` 实例化调用处 NOP | 中（需逆向） | 精确定位后用 |
| **D. 仅服务端** | 北斗服关闭公告下发 | 无客户端改动 | 若只玩私服可考虑 |

**实施前需 IDA 确认**（约 0.5 天）：

1. `CSlideNotice` 谁创建、何时 `SetMsg`
2. Royals `ijl15.dll` 是否已有禁用逻辑（对比 BeiDou.exe + Royals.dll）

### 3.4 与分辨率改造的关系

kaentake 在 `set_screen_resolution()` 里会 **拉宽** `CSlideNotice` 到全屏宽，不是隐藏。  
若要做「Royals 式无顶栏」，应在 Hide 逻辑里**独立于**宽度补丁。

---

## 4. 资源改动清单（预估）

| 文件 | 是否必须 | 原因 |
|------|----------|------|
| `Login.img` | ✅ | 补 `1024frame`；`frame` 保持 800×600 |
| `StatusBar.img` | ✅ | 补 40 个 `1024*`；`quickSlot` 8 格；删 `quickSlot26/32` |
| `CashShop.img` | ✅（1024 档） | 补 `1024backgrnd` |
| `UIWindow.img` | ⚠️ 部分 | **必须**：`SysOpt/backgrnd` ← 你的 PNG；**可选**：`muruengRaid/1024*` |
| `MapleEzorsiaV2wzfiles.img` | ❌ 建议停用 | 与 Royals 路线冲突 |
| `Custom.wz` | 可选 | 若走轨道 B 放 SysOpt |

**对比你已上传的 `client-assets`：** 目前只有 800 档 Login + StatusBar；对齐 Royals 还需 **1024 节点 + SysOpt 396px**。

---

## 5. 建议的下一步（等你审阅后）

1. **你确认目标档位**：是否只做 **800 + 1024**（不做 1280/1366）？
2. **你确认 SysOpt 部署轨道**：A（改 UIWindow.img）还是 B（Custom.wz）？
3. **你确认顶栏**：客户端强制隐藏 vs 服务端关公告？
4. **我按阶段 1 执行**：ImgPatch 生成完整 `client-assets`，**不动 DLL**
5. **阶段 2–3**：在 `feature/native-800x600` 或新分支 `feature/royals-1024` 上移植 kaentake 模块

---

## 6. 风险与回滚

| 风险 | 缓解 |
|------|------|
| kaentake + ezorsia 地址补丁冲突 | 单一 `ApplyResolution()`，禁止双轨 |
| 汉化编码再踩坑 | 遵守 `docs/汉化与编译踩坑记录.md` |
| WZ 合坏汉化控件 | ImgPatch 只合并 `1024*` + 指定 canvas，不整文件覆盖 |
| DX8 启动失败 | 保持 `WindowedMode=true` |

回滚：恢复 `client-assets` 备份 + `SwitchChinese` + 旧 `ijl15.dll`；WZ 用 `_backup_ui_*` 目录。

---

*请审阅后回复：档位确认、SysOpt 轨道、顶栏策略，再开始阶段 1。*
