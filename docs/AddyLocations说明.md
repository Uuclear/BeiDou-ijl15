# AddyLocations.h 地址说明

> 对应文件：`ezorsia/AddyLocations.h`  
> 目标客户端：北斗 v83（映像基址 `0x00400000`）  
> 用法：DLL 在运行时把 `push` / `mov` 等指令里的**硬编码立即数**改成当前分辨率或配置值。

---

## 一、通用 / 网络 / 加密

| 符号 | 地址 | 作用 |
|------|------|------|
| `dwDInput8DLLInject` | `0x00796357` | DirectInput8 注入点 |
| `dwMovementFlushInterval` | `0x0068A83F` | 移动包刷新间隔 |
| `dwStatWndOnStatChange` | `0x00A20213` | 属性窗口刷新 |
| `dwUserEmoteCoolTime` | `0x00A244AE` | 表情冷却 |
| `dwUserGivePopularityCoolTime` | `0x00A23F28` | 人气度赠送冷却 |
| `dwMessageChatDelay` | `0x00490651` | 聊天发送延迟 |
| `dwMessageChatSpam` | `0x00490607` | 聊天刷屏限制 |
| `dwRemoteAddress` | `0x00AFE084` | 登录服务器 IP 字符串（`UpdateGameStartup` 写入） |
| `dwIGCipherHash` 等 | `0x00A4A8xx` | 封包加密（参考，北斗未启用） |
| `dwUnlimitedSoulRush` | `0x0096BF09` | 灵魂突袭限制（可选补丁） |
| `dwUnlimitedFJ` | `0x0096BEB5` | 二段跳限制（可选补丁） |
| `dwUnlimitedFJYVector` | `0x0096BF86` | 二段跳 Y 向量 |

---

## 二、分辨率核心（最重要）

### 2.1 应用与视口

| 符号 | 地址 | 游戏作用 |
|------|------|----------|
| `dwApplicationHeight` | `0x009F7B1D` | `CWvsApp::InitializeGr2D` 推入的高度（600/768） |
| `dwApplicationWidth` | `0x009F7B23` | 同上，宽度（800/1024） |
| `dwViewPortHeight` | `0x009DFCF0` | 视口高度偏移计算 |
| `dwViewPortWidth` | `0x009DFE68` | 视口宽度偏移计算 |

### 2.2 鼠标 / 光标

| 符号 | 地址 | 游戏作用 |
|------|------|----------|
| `dwCursorVectorVPos` | `0x0059A15D` | `CInputSystem::SetCursorVectorPos` 光标向量 Y（通常 `-高度/2`） |
| `dwCursorVectorHPos` | `0x0059A169` | 光标向量 X（通常 `-宽度/2`） |
| `dwUpdateMouseLimitVPos` | `0x0059AC22` | `UpdateMouse` 垂直边界 |
| `dwUpdateMouseLimitHPos` | `0x0059AC09` | `UpdateMouse` 水平边界 |
| `dwCursorPosLimitVPos` | `0x0059A8B1` | `SetCursorPos` 垂直上限 |
| `dwCursorPosLimitHPos` | `0x0059A898` | `SetCursorPos` 水平上限 |

### 2.3 状态栏 `CUIStatusBar`

| 符号 | 地址 | 游戏作用 |
|------|------|----------|
| `dwStatusBarVPos` | `0x008CFD55` | 状态栏 Y 坐标 codecave 入口（1024 用 `AdjustStatusBar`） |
| `dwStatusBarBackgroundVPos` | `0x008D1F65` | 状态栏背景 Y codecave |
| `dwStatusBarInputVPos` | `0x008D217C` | 聊天输入框 Y codecave |
| （`0x008CFD4B`） | — | 状态栏创建时 push 的高度（`高度-22`，800→578） |
| （`0x008CFD50`） | — | 状态栏创建时 push 的宽度 |

### 2.4 快捷栏 `CQuickSlot`

| 符号 | 地址 | 游戏作用 |
|------|------|----------|
| `dwQuickSlotInitVPos` | `0x008D1793` | 快捷栏初始化垂直偏移 |
| `dwQuickSlotInitHPos` | `0x008D179A` | 快捷栏初始化水平 push（800→647，1024→815） |
| `dwQuickSlotVPos` | `0x008DF782` | 快捷栏绘制垂直偏移 |
| `dwQuickSlotHPos` | `0x008DF7F8` | 快捷栏绘制水平 push |
| `dwQuickSlotCWndVPos` | `0x008DE8EE` | 快捷栏窗口 lea 垂直 |
| `dwQuickSlotCWndHPos` | `0x008DE8E5` | 快捷栏窗口 lea 水平（800→-647） |
| （`0x008DE850/896/82B`） | — | 快捷栏鼠标命中检测 X/Y |

### 2.5 Tooltip / Buff 图标

| 符号 | 地址 | 游戏作用 |
|------|------|----------|
| `dwToolTipLimitVPos` | `0x008F32DF` | `CUIToolTip::MakeLayer` 垂直边界 |
| `dwToolTipLimitHPos` | `0x008F32CC` | 同上，水平边界 |
| `dwTempStatToolTipDraw` | `0x007B2EA0` | Buff 图标 Tooltip 绘制 X |
| `dwTempStatToolTipFind` | `0x007B3087` | Buff 图标 Tooltip 查找 X |
| `dwTempStatIconVPos` | `0x007B2C97` | Buff 图标 Y |
| `dwTempStatIconHpos` | `0x007B2CB5` | Buff 图标 X |
| `dwTempStatCoolTimeVPos` | `0x007B2DA0` | 冷却遮罩 Y |
| `dwTempStatCoolTimeHPos` | `0x007B2DBE` | 冷却遮罩 X |

### 2.6 喇叭 / Boss 条 / 其它 UI

| 符号 | 地址 | 游戏作用 |
|------|------|----------|
| `dwByteAvatarMegaHPos` | `0x0045B97E` | 喇叭消息 `RelMove` X |
| `dwAvatarMegaWidth` | `0x0045A5CB` | 喇叭窗口 `CreateWnd` 宽度 |
| `dwBossBar` | `0x00533B0E` | Boss 血条水平位置 codecave |
| `dwCashFix` | `0x00469348` | 商城旧版修复入口 |
| `dwVersionNumberFix` | `0x005F464D` | 版本号显示修复 |
| `dwAlwaysViewRestoreFix` | `0x00642105` | 地图视野恢复 |

### 2.7 WndMan（kaentake / resolution.cpp）

| 地址 | 作用 |
|------|------|
| `0x009F7078` | `CWvsApp::CreateWndManager` push 高度 |
| `0x009F707D` | 同上 push 宽度 |
| `0x009F7A9B` | 强制窗口模式（`WindowedMode=true` 时 patch） |

---

## 三、登录界面

| 符号 | 地址 | 游戏作用 |
|------|------|----------|
| `dwLoginBackCanvasFix` | `0x0060E1BF` | 登录背景画布尺寸 |
| `dwLoginViewRecFix` | `0x0062B334` | 登录视口矩形 |
| `dwLoginDescriptorFix` | `0x0060D85B` | 登录描述文字区域 |
| `dwLoginCreateDlg` | `0x006203E8` | 登录对话框创建位置 |
| `dwLoginUsername` | `0x006209A6` | 账号输入框位置 |
| `dwLoginPassword` | `0x00620A0D` | 密码输入框位置 |
| `dwLoginLoginBtn` 等 | `0x006206xx` | 登录/退出/注册等按钮坐标 |

---

## 四、商城 CashShop codecave

| 符号 | 地址 | 游戏作用 |
|------|------|----------|
| `dwCashFix1`–`dwCashFix8` | `0x00469414` 起 | 商城各面板 `CreateWnd` 宽高/位置（1024 居中偏移） |
| `dwCashFixOnOff` | `0x00776B5F` | 商城开关按钮 |
| `dwCashFixPrev` | `0x004AB10F` | 商城预览窗 |

---

## 五、经验/物品飘字

| 符号 | 地址 | 游戏作用 |
|------|------|----------|
| `dwMoreGainMsgs` | `0x0089B185` | 拾取/经验消息数量上限 |
| `dwMoreGainMsgsFade` | `0x0089B563` | 消息淡出 |
| `dwMoreGainMsgsFade1` | `0x0089B4E6` | 消息淡出参数 |

---

## 六、穆伦远征 UI（`dwMuruengraid*`）

一组 `0x00554xxx` / `0x00555xxx` codecave，用于远征副本内玩家信息、计时器、能量条、怪物计数等 UI 在宽屏下的位置。北斗 1024 路线中部分通过 `UIWindow.img` 的 `1024player/monster` 节点配合。

---

## 七、无限传送（可选）

| 符号 | 地址 | 作用 |
|------|------|------|
| `dwTeleFieldLimit` | `0x00957BB7` | 地图传送限制 |
| `dwTeleUpdateTime` | `0x00957BFE` | 传送冷却 |
| `dwTeleFootholdAbove/Below` | `0x00957Exx` | 落脚点检测 |
| `dwTeleIsPortal` | `0x00957C25` | 传送门判断 |

---

## 八、`dw0x0044xxxx` / `dw0x009Fxxxx` 大块

这些是 **加壳客户端** 在 CRC bypass 时用的跳转/ NOP 占位地址（`UpdateGameStartup` 里大多已注释）。北斗客户端已脱壳，**一般不需要**启用。

---

## 九、调试

| 方式 | 说明 |
|------|------|
| `config.ini` → `[debug] debug=true` | 每次切分辨率时，`beidou-ui.log` 自动 **全量 dump** 上表核心补丁点的当前值 |
| `UiDebugDumpAll()` | 代码内手动调用（预留接口） |
| `steps/extracted/kaentake_addr_verify.md` | 与 kaentake 交叉验证过的关键 VA |

**800 正常值参考**：`StatusBar_W=0x320`，`QS_Init_H=0x287(647)`，`OrgQuickSlot` 约 `x=0,y=0`。  
**1024 正常值参考**：`StatusBar_W=0x400`，`QS_Init_H=0x32F(815)`，`OrgQuickSlot` 约 `x=152,y=68`（相对锚点）。
