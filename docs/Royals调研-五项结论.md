# Royals 对齐调研 — 五项结论（2026-09-09）

> 仅调研与资源导出，**未改 DLL / 未部署客户端**（窗口标题除外见下文说明）。

---

## 1. Royals 1024 资源已导出

**审阅目录：** `BeiDou-ijl15-resources/royals-1024-export/`

| 子目录 | 用途 |
|--------|------|
| `assets-1024-only/` | **44 张** 1024 相关 PNG |
| `xml/` | Login / CashShop / StatusBar / UIWindow 完整 XML |
| `INDEX.md` | 节点路径与尺寸表 |

---

## 2. 登录横幅 Logo 在哪里？能否隐藏？

### 结论

- **`Login.img/Title/MSTitle`**：两边都是原版 MapleStory Beta 竖版 Logo，**不是你截图里的 BeiDouMS / MapleRoyals 横条**。
- 你截图里的 **私服横条 Logo** 更可能在：
  - `Login.img/Title/effect/*` 动画层，或
  - 选世界/过场相关节点（`WorldSelect`、`MapLogin.img` 的 `Title/logo` UOL）
- 解码 XML 中的 `Title/backgrnd/0`（579×37）在**当前**客户端 `Login.img` 二进制里**未出现在 `Title` 下**（XML 可能过期）。

### 能否隐藏

| 方式 | 可行性 |
|------|--------|
| 找到 canvas 后换透明图 / 改 origin | ✅ 推荐 |
| `RemoveLogos=true` | 仅跳过**启动片头**（Nexon/Wizet），不挡登录框内横幅 |
| DLL Hook 绘制 | ✅ 可行，需 IDA |

详见：`BeiDou-ijl15-resources/login-title-banners/README.md`

---

## 3. 窗口标题：北斗 → MapleStory

**位置：** `ezorsia/ReplacementFuncs.h`，StringPool 索引 **1163**（始终生效，与 `SwitchChinese` 无关）

```cpp
if (nIdx == 1163) {
    *ret = "BeiDou";   // 改为 "MapleStory"
}
```

表中 `{1163, "±±¶·"}` 为 GBK 占位，运行时被上面硬编码覆盖。

> **未在本次自动修改**；你确认后改一行即可。

---

## 4. SysOpt / 地图部署：方法 A（已记录）

- **采用：** 将 `steps/SysOpt.backgrnd.ps2.png`（299×396）打进 **`Data/UI/UIWindow.img`** 的 `SysOpt/backgrnd`
- **不采用：** `Custom.wz` + `InitializeResMan`（IMG 与 WZ 两套读取标准，避免混用）
- 工具：`steps/tools/ImgPatch` 已支持 `ReplaceCanvasPng(SysOpt/backgrnd, png)`

---

## 5. Royals 里有没有现成的 NOP（顶栏公告 / SlideNotice）？

### 结论：**无法在公开二进制里找到可复用的 NOP 补丁**

| 文件 | 分析结果 |
|------|----------|
| `MapleRoyals/ijl15.dll`（~3MB） | **Themida 加壳**，IDA 仅 2 个函数、无明文字符串；搜不到 `0x007E15BE` 等客户端地址 |
| `MapleRoyals/mr.dll` | 无 `SlideNotice` / `7E15` 相关字符串 |
| `MapleRoyals.exe` | 壳程序，非完整 v83 客户端逻辑 |

对比：

| 项目 | `CSlideNotice`（顶部滚动公告） |
|------|-------------------------------|
| ezorsia `UpdateResolution()` | 改**宽度**（`0x007E15BE` 等），**不隐藏** |
| kaentake `resolution.cpp` | 同上，拉宽到全屏 |
| Royals | **无可见 NOP**；截图无顶栏更可能是 **服务端不发公告** 或补丁在壳内 |

### 北斗可行做法（后续实施）

1. `config.ini`：`HideSlideNotice=true`
2. Hook `CSlideNotice::OnCreate` 或 `SetMsg` 空实现（地址与 ezorsia 已知的 `0x007E15BE` 系一致）
3. 不依赖 Royals 现成 NOP

---

## 建议的下一步（等你确认）

1. 你打开 `royals-1024-export/assets-1024-only/` 审阅 1024 素材  
2. 告知登录 Logo 出现在**哪一屏**（片头 / 标题 / 选世界 / 选角），便于精确定位 canvas  
3. 确认后：改窗口标题、导出 Login effect 帧对比、实施 SysOpt 打进 `UIWindow.img`
