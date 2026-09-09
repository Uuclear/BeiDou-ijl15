# Royals 对齐 — 待办清单

> **工作方式**：从上到下逐项做；**每完成一项 → 测试 → `git commit` 一次**（只提交该项相关文件）。  
> **分支**：`feature/native-800x600`  
> **资源部署**：合并进 `client-assets/Data/`，客户端用 `robocopy client-assets\Data <客户端>\Data /E /XO`

**已拍板决策**

| 项 | 选择 |
|----|------|
| 分辨率档位 | 仅 **800×600** + **1024×768** |
| SysOpt 底图 | **轨道 A**：打进 `UIWindow.img`（不用 Custom.wz） |
| 顶栏公告 | 客户端 **HideSlideNotice** Hook（不依赖服务端关公告） |

---

## 已完成

- [x] **T00** DLL 固定 800×600 + 汉化编码修复（`feature/native-800x600` 早期提交）
- [x] **T01** `client-assets`：`Data/UI/Login.img`、`StatusBar.img`（800 档）
- [x] **T02** `client-assets/Data/Map/Obj/login.img` 原版 MapleStory 横条 Logo
- [x] **T03** `client-assets` 按 `Data/` 目录镜像 + README 部署说明
- [x] **T04** 窗口标题改为 MapleStory（2026-09-10）

---

## 待办（按顺序）

### 阶段 A — 小改动（资源 / 一行代码）

- [x] **T04 — 窗口标题改为 MapleStory** ✓ 2026-09-10  
  - 改：`ezorsia/ReplacementFuncs.h` 索引 `1163`：`"BeiDou"` → `"MapleStory"`  
  - 编译部署 `ijl15.dll`  
  - 验：任务栏 / 窗口标题显示 MapleStory  
  - 提交示例：`fix: window title MapleStory instead of BeiDou`

- [ ] **T05 — SysOpt 中文底图（396px）**  
  - 用 `steps/SysOpt.backgrnd.ps2.png` 打进 `UIWindow.img/SysOpt/backgrnd`  
  - 工具：`steps/tools/ImgPatch` 或 `ReplaceCanvasPng`  
  - 输出：`client-assets/Data/UI/UIWindow.img`  
  - 验：系统设置底图变长，末行留空给分辨率 ComboBox  
  - 提交示例：`assets: SysOpt backgrnd 299x396 Chinese layout`

- [ ] **T06 — 合并 Login.img 1024 节点**  
  - 从 Royals 合并 `Common/1024frame`（保留北斗汉化控件）  
  - 更新 `client-assets/Data/UI/Login.img`  
  - 验：Harepacker 可见 `1024frame` 节点  
  - 提交示例：`assets: Login.img add Common/1024frame`

- [ ] **T07 — 合并 StatusBar.img 1024 节点**  
  - 合并 40 个 `1024*` 节点；`quickSlot` 保持 8 格；删 `quickSlot26/32`  
  - 更新 `client-assets/Data/UI/StatusBar.img`  
  - 提交示例：`assets: StatusBar.img Royals 1024 nodes and 8-slot quickSlot`

- [ ] **T08 — 合并 CashShop.img 1024 节点**  
  - 合并 `Base/1024backgrnd`  
  - 输出：`client-assets/Data/UI/CashShop.img`  
  - 提交示例：`assets: CashShop.img add Base/1024backgrnd`

- [ ] **T09 — 合并 UIWindow.img 1024 节点（可选木灵战）**  
  - 合并 `muruengRaid/1024monster`、`1024player`（若暂不需要可跳过，标为 cancelled）  
  - 提交示例：`assets: UIWindow.img add muruengRaid 1024 nodes`

- [ ] **T10 — config 默认关闭 EzorsiaV2 WZ 重定向**  
  - 改：`config.ini` / 文档：`EzorsiaV2WzIncluded=false`  
  - 说明不再依赖 `MapleEzorsiaV2wzfiles.img` 的 frame1280 路线  
  - 提交示例：`config: disable EzorsiaV2WzIncluded for Royals-style UI`

- [ ] **T11 — 更新 client-assets README**  
  - 补全 T06–T09 文件清单与目录树  
  - 提交示例：`docs: client-assets README for 1024 imgs`

---

### 阶段 B — DLL 分辨率与 UI 切换

- [ ] **T12 — 统一分辨率入口 `ApplyResolution(w,h)`**  
  - 新建/合并单一入口，禁止 `UpdateResolution()` 1280 与 kaentake 双轨并存  
  - 提交示例：`refactor: unified ApplyResolution entry`

- [ ] **T13 — 移植分辨率 ComboBox（800 / 1024）**  
  - 从 kaentake 移植 `resolution.cpp` 精简版：SysOpt ComboBox、OK 按钮下移、`CConfig` 读写  
  - 依赖 T05 SysOpt 底图  
  - 验：系统设置出现分辨率下拉  
  - 提交示例：`feat: in-game resolution ComboBox 800 and 1024`

- [ ] **T14 — 运行时 1024 UI 路径切换**  
  - 移植 `uipath.cpp` 或扩展 StringPool / `GetObject` Hook（`frame` ↔ `1024frame` 等）  
  - 依赖 T06–T08 资源  
  - 验：1024 下 StatusBar / 登录框素材正确（重登后生效可接受）  
  - 提交示例：`feat: runtime UOL switch for 1024 UI assets`

- [ ] **T15 — 1024 档坐标补丁（子集）**  
  - 从 `UpdateResolution()` 只抽取 1024 需要的宽度补丁（StatusBar、聊天、商城居中）  
  - **禁止**整包 1280 codecave  
  - 提交示例：`fix: 1024-only layout patches for StatusBar and chat`

---

### 阶段 C — 顶栏公告

- [ ] **T16 — IDA 确认 CSlideNotice 调用点**  
  - 记录创建 / `SetMsg` 地址（对照 `0x007E15BE` 系）  
  - 产出：简短笔记写入 `docs/` 或 `BeiDou-ijl15-resources/docs/`  
  - 提交示例：`docs: CSlideNotice hook addresses for HideSlideNotice`

- [ ] **T17 — HideSlideNotice 配置 + Hook**  
  - `config.ini`：`HideSlideNotice=true`  
  - Hook `OnCreate` 或 `SetMsg` 空实现  
  - 验：进游戏无顶部滚动公告条  
  - 提交示例：`feat: HideSlideNotice config and client hook`

---

### 阶段 D — 验收与文档收尾

- [ ] **T18 — 全量验收清单**  
  - [ ] 800 / 1024 切换正常  
  - [ ] 8 格快捷栏，无 26 格残影  
  - [ ] 登录框 1024 用 `1024frame`  
  - [ ] 顶栏无公告  
  - [ ] 汉化 / IME / BossHP 正常  
  - [ ] 窗口标题 MapleStory  
  - 提交示例：`docs: Royals alignment acceptance checklist passed`

- [ ] **T19 — 更新 WIKI 与调研文档**  
  - 更新 `docs/WIKI.md`、`docs/Royals调研-五项结论.md`（Logo 路径、client-assets 结构）  
  - 提交示例：`docs: update WIKI and Royals notes after alignment`

---

## 当前进度

**下一步建议：T05（SysOpt 中文底图）** — ImgPatch 打进 `UIWindow.img`。

完成一项后在本文件把 `[ ]` 改为 `[x]` 并写上完成日期，再 `git add` 该项文件 + 本文件（若勾选了进度）并提交。
