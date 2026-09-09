# 客户端 UI 资源（800×600 原版）

本目录存放还原原版分辨率时**已打补丁**的 `.img` 文件，部署到客户端 `Data/UI/` 即可。

## 文件清单

| 文件 | 大小（约） | 修改内容 |
|------|-----------|----------|
| `Data/UI/Login.img` | 7.0 MB | `Common/frame` 改为 **800×600**；`CharSelect/effect/1/0–4` 与 Royals 对齐 |
| `Data/UI/StatusBar.img` | 117 KB | `base/quickSlot` 改为 **8 格**底图（151×80）；删除 `quickSlot26` / `quickSlot32` |

## 部署

```text
复制本目录下 Data/UI/*.img → <客户端根目录>/Data/UI/
```

部署前建议备份客户端原文件。

## 为何只改这两个？

北斗客户端此前为宽屏（1280×720）改过 UI：`Login.img` 的登录框是 1280 宽，`StatusBar.img` 带 26 格快捷栏素材。  
DLL 还原 **800×600** 且关闭 `LongQuickSlot` 后，若不换资源会出现：

- 登录框比例/裁切异常
- 底部快捷栏底图与 8 格布局不匹配

`CashShop.img`、`UIWindow.img` 等**未修改**——800×600 下由原版客户端资源即可正常显示。

## 能否不修改？

| 场景 | 是否需要改 IMG |
|------|----------------|
| 继续用宽屏 + `LongQuickSlot`（1280×720） | **不需要**，用客户端原有宽屏资源 |
| DLL 固定 800×600 + 关闭长快捷键 | **需要**（本目录资源，或自行用 ImgPatch 生成） |
| 客户端从未打过宽屏 UI 补丁 | **不需要**，用原始 `Login.img` / `StatusBar.img` 即可 |

生成工具：`steps/tools/ImgPatch`（仓库外，见主项目 `Maple/steps/`）。
