# 客户端补丁资源（镜像 `Data/` 目录）

本目录下的 **`Data/`** 与北斗客户端 `Data/` **路径一一对应**。部署时整目录覆盖即可，无需逐个挑文件。

## 目录结构

```text
client-assets/
└── Data/
    ├── UI/
    │   ├── Login.img          # 800×600 登录框
    │   └── StatusBar.img      # 8 格快捷栏底图
    └── Map/
        └── Obj/
            └── login.img      # 原版 MapleStory 登录横条 Logo
```

## 文件说明

| 路径 | 大小（约） | 修改内容 |
|------|-----------|----------|
| `Data/UI/Login.img` | 7.0 MB | `Common/frame` **800×600**；`CharSelect/effect/1/0–4` 与 Royals 对齐 |
| `Data/UI/StatusBar.img` | 117 KB | `base/quickSlot` **8 格**；删除 `quickSlot26` / `quickSlot32` |
| `Data/Map/Obj/login.img` | 4.9 MB | `Title/logo/0/0`、`0/1` 原版 **MapleStory** 横条（自 `MapleStory/Map.wz` 导出） |

## 部署（推荐：整目录覆盖）

部署前请关闭游戏 / Harepacker，并备份客户端 `Data/`。

```powershell
# 将本仓库 client-assets/Data 合并覆盖到客户端 Data（保留客户端其它未列出的文件）
robocopy ".\client-assets\Data" "<客户端根目录>\Data" /E /XO

# 或 PowerShell：
Copy-Item ".\client-assets\Data\*" "<客户端根目录>\Data\" -Recurse -Force
```

只会覆盖上表三个 `.img`；`Data` 下其它子目录（Character、Item 等）不受影响。

## 背景说明

北斗客户端曾为宽屏改过 UI；DLL 固定 **800×600** 且关闭 `LongQuickSlot` 后需要上述资源：

- 登录框比例（`Login.img`）
- 底部 8 格快捷栏（`StatusBar.img`）
- 登录横条 Logo 在 **`Map/Obj/login.img`**（不是 `UI/Login.img/Title/MSTitle`）

`CashShop.img`、`UIWindow.img` 等未收录——800×600 下用客户端原版即可。

源 PNG / 打补丁工具：`BeiDou-ijl15-resources/login-banner-scan/vanilla-export/`、`steps/tools/ImgCanvasPatch`、`steps/tools/ImgPatch`。

## 是否需要这些资源？

| 场景 | 是否需要 |
|------|----------|
| 宽屏 + `LongQuickSlot`（1280×720） | 否 |
| DLL 800×600 + 关闭长快捷键 | 是（本目录或自行 ImgPatch） |
| 客户端从未打宽屏 UI 补丁 | 仅 `login.img` Logo 可选 |
