# PSD2UMG 重写设计 (方案 B)

- 日期: 2026-05-17
- 适用引擎: Unreal Engine 5.7
- 状态: 已通过 brainstorming 用户审阅,待写实施计划

## 1. 背景与目标

现有 `PSD2UMG_5.7` 插件基于 2018 年闭源 `PSDParser.lib` + `UFactory`,代码停留在 UE 4.x 风格,
含若干潜在 bug(`brush.ImageSize` 误用、`Text` 拷贝死循环、`GetAssetRegistryTags` API 已弃用、
不支持新版 PS / 16-32 bit PSD 等)。

本次重写目标:

1. 把 PSD 解析换成持续维护、纯 C++ 零三方依赖的 [MolecularMatters/psd_sdk](https://github.com/MolecularMatters/psd_sdk)。
2. 与 UE 5.7 UMG / Slate / AssetDefinition / CommonUI 现代 API 对齐。
3. 引入 PSD 命名约定 + 伴随 JSON,让设计师在 PS 内完成绝大多数控件类型与样式的标注。
4. 提供生产可用的 9-slice、富文本、CommonUI、链接子 PSD → 子 WidgetBlueprint、Reimport 增量、FMessageLog 错误反馈。

非目标(留待后续):

- Texture Atlas 自动打包
- WidgetAnimation 多态过渡轨道
- CommonUI Activatable Widget Stack
- 跨平台(Mac / Linux):本期仅 Win64 Editor
- **Photoshop Smart Object 嵌入字节的像素提取**(psd_sdk 当前不支持),改用"独立子 PSD 文件 + 名字引用"约定
- **PSB 大文件(>2GB)**(psd_sdk 不支持),导入时由 PsdReader 检测格式头并报错

## 2. 既定决策(brainstorming 输出)

| 决策项 | 选项 |
|---|---|
| 旧代码 | **完全推倒重写**(不保留对老 .psdumg 资产的兼容) |
| 导入后缀 | **直接用 `.psd`**(放弃老插件的 `.psdumg`),让 PSD 文件可直接拖入 Content Browser |
| PSD 解析库 | **MolecularMatters/psd_sdk vendor**(纯 C++ ~7200 LOC,零外部依赖,active 维护至 2026-05) |
| 平台 | **Win64 Editor only** |
| 功能档位 | **生产可用集**(9-slice / 链接子 PSD / 富文本 / CommonUI / FMessageLog) |
| Schema 协议 | **图层命名约定 + 伴随 `.psd.json`** |
| CommonUI | **默认开启**,可通过项目设置 / 图层 `#vanilla` tag 关闭 |
| 纹理策略 | **一层一贴图**,TC_UI / sRGB / BC7,无去重无 atlas |
| 资产目录布局 | **同目录平铺**`/Game/UI/PsdImport/<PsdName>/` |
| 子 PSD 引用 | **图层名标记 `#linkedpsd(SubFile.psd)`**,Importer 递归处理 sibling 文件 |
| 测试 | **Automation 集成测试** + 样本 PSD |
| 整体架构 | **B1 三层管线**:Importer → PsdDocument 中间表示 → UmgBuilder |

## 3. 模块与目录结构

```
PSD2UMG.uplugin                        5.7, Editor, Win64 only
Source/PSD2UMG/
  PSD2UMG.Build.cs                     依赖见下
Source/psd_sdk/                        vendored sibling UE module
  psd_sdk.Build.cs
  Includes/  Source/  Private/PsdSdkModule.cpp
  LICENSE.psd_sdk
Source/PSD2UMG/Private/
  PSD2UMG.cpp                          模块入口
  Importer/
    PSD2UMGFactory.{h,cpp}             UFactory + FReimportHandler, 递归 LinkedPsd
    PsdReader.{h,cpp}                  psd_sdk ↔ FPsdDocument 适配
  Schema/
    PsdDocument.{h,cpp}                纯 C++ 中间表示
    PsdNamingParser.{h,cpp}            解析 name#tag(args)
    PsdSidecarLoader.{h,cpp}           读 .psd.json
    PsdSchemaResolver.{h,cpp}          命名 + JSON → FWidgetSpec
  Builder/
    UmgBuilder.{h,cpp}                 FWidgetSpec → UWidgetBlueprint
    TextureBuilder.{h,cpp}             像素 → UTexture2D
    StyleAssetBuilder.{h,cpp}          TextStyle / ButtonStyle DataAsset
  Settings/
    PSD2UMGSettings.{h,cpp}            UDeveloperSettings
  Asset/
    PSD2UMGCache.{h,cpp}               UObject 资产
    AssetDefinition_PSD2UMG.{h,cpp}    UAssetDefinition
Source/PSD2UMG/Public/
  IPSD2UMG.h
```

## 3. 模块与目录结构

```
PSD2UMG.uplugin                        5.7, Editor, Win64 only
Source/PSD2UMG/
  PSD2UMG.Build.cs                     依赖见下
Source/psd_sdk/                        vendored sibling UE module
  psd_sdk.Build.cs
  Includes/  Source/  Private/PsdSdkModule.cpp
  LICENSE.psd_sdk
Source/PSD2UMG/Private/
  PSD2UMG.cpp                          模块入口
  Importer/
    PSD2UMGFactory.{h,cpp}             UFactory + FReimportHandler, 递归 LinkedPsd
    PsdReader.{h,cpp}                  psd_sdk ↔ FPsdDocument 适配
  Schema/
    PsdDocument.h                      纯 C++ 中间表示
    PsdNamingParser.{h,cpp}            解析 name#tag(args)
    PsdSidecarLoader.{h,cpp}           读 .psd.json
    PsdSchemaResolver.{h,cpp}          命名 + JSON → FWidgetSpec
    WidgetSpec.h
  Builder/
    UmgBuilder.{h,cpp}                 FWidgetSpec → UWidgetBlueprint
    TextureBuilder.{h,cpp}             像素 → UTexture2D
    StyleAssetBuilder.{h,cpp}          TextStyle / ButtonStyle DataAsset
  Settings/
    PSD2UMGSettings.{h,cpp}            UDeveloperSettings
  Asset/
    PSD2UMGCache.{h,cpp}               UObject 资产
    AssetDefinition_PSD2UMG.{h,cpp}    UAssetDefinition
Source/PSD2UMG/Public/
  IPSD2UMG.h
Tests/Sample/                          5 个样本 PSD + expected.json + generate_samples.py
Source/PSD2UMGTests/                   独立测试模块 (DeveloperTool)
docs/
  README.md / schema.md / samples.md
```

### Build.cs 依赖

```
PublicDependencyModuleNames:
  Core, CoreUObject, Engine, UMG

PrivateDependencyModuleNames:
  UnrealEd, UMGEditor, Slate, SlateCore, RenderCore, ImageWrapper,
  AssetTools, AssetRegistry, AssetDefinition, ToolMenus, DeveloperSettings,
  MessageLog, Json, JsonUtilities,
  KismetCompiler, BlueprintGraph, Projects,
  CommonUI,
  psd_sdk
```

### 模块边界(强约束)

- `Schema/` **不**依赖 `UMG / Engine`,只依赖 `Core`,可在 commandlet/Automation 命令行下独立测试。
- `Builder/` 只接 `FWidgetSpec`,**不**知道 psd_sdk 的存在,可被 mock 输入驱动。
- `Importer/` 是唯一同时碰 psd_sdk 和 UE 资产系统的层。

## 4. 数据流

```
.psd 文件
   │
   │ PsdReader (psd_sdk::Document + psd_sdk::Layer)
   ▼
FPsdDocument {
   FIntPoint CanvasSize;
   int ColorDepth;                      // 8 / 16 / 32 (psd_sdk 支持全部, v1 在 builder 阶段降 8bit)
   TArray<FPsdLayer> Layers;            // 树状, Children 嵌套
}
FPsdLayer {
   FString Name;
   FBox2D Bounds;
   float Opacity;
   EBlendMode Blend;
   ELayerKind Kind;                     // GROUP / RASTER / TEXT / LINKED_PSD
   bool bVisible;
   TArray<uint8> RGBA8;                 // GROUP 不含
   FPsdTextRun TextRun;                 // 仅 TEXT
   FString LinkedPsdRelPath;            // 仅 LINKED_PSD: `#linkedpsd(X.psd)` 中解析出的相对路径
   TArray<FPsdLayer> Children;
}
   │
   │ PsdSidecarLoader (找 <PsdPath>.psd.json)
   │ PsdNamingParser   (解析 `name#tag(args)`, 含 #linkedpsd)
   │ PsdSchemaResolver (合并)
   ▼
FWidgetSpec {
   FName WidgetName;
   EWidgetType Type;                    // Canvas/Image/Button/ProgressBar/Text/SizeBox/ScaleBox/NamedSlot/SubWidget
   FAnchors Anchors;
   FVector2D Position;
   FVector2D Size;
   FSlateBrushSpec Brush;               // 含 9-slice Margin / DrawAs
   FTextStyleSpec TextStyle;
   bool bUseCommonUI;
   FSoftObjectPath StyleAssetRef;
   FName SubWidgetAssetName;            // SubWidget: 子 WBP 资产名,由 LinkedPsd 解析而来
   TArray<FWidgetSpec> Children;
}
   │
   │ UmgBuilder
   ▼
WBP_<PsdName>(.uasset)
T_<SanitizedLayerName>(.uasset)
BPS_<SanitizedButtonName>(.uasset)     按需
WBP_<SubPsdName>(.uasset)              来自 LinkedPsd 的子 PSD,独立递归导入
PsdCache_<PsdName>(.uasset)            UPSD2UMGCache, Reimport 入口
```

### 纹理生成

psd_sdk 解码为 planar (按 channel) 像素流。`TextureBuilder` 把 R/G/B/A 4 个平面拼到 BGRA8
+ `ParallelFor`。4K PSD 在 8 核机上预计 < 80 ms。

- 默认压缩:`TC_UI`、`SRGB=true`、`MipGen=NoMipmaps`
- Source 格式:`TSF_BGRA8`(16/32 bit PSD 在 v1 降到 8bit;v2 计划 `TSF_RGBA16F`)
- 命名:`T_<SanitizedLayerName>`(目录已含 PsdName,资产名不再重复);重名按 `_Dup_N` 兜底

### 子 PSD 引用(替代 Photoshop Smart Object)

psd_sdk 不支持 Smart Object 嵌入字节的像素提取。本期改用**文件级链接**约定:

- 图层名标 `#linkedpsd(SubFile.psd)`,意为"此处嵌入一个 UserWidget,其源是同目录(或相对路径)的 `SubFile.psd`"。
- Importer 检测到 LINKED_PSD 图层:
  - 解析相对路径 → 找到 sibling .psd 文件
  - 递归对该子 PSD 跑同一条 Importer → Schema → Builder 管线
  - 产出独立的 `WBP_<SubPsdName>` 到 `/Game/UI/PsdImport/<SubPsdName>/`
  - 主 WBP 在该位置生成 `UUserWidget` 包装节点,`WidgetClass` 指向子 WBP
- Resolver 维护 `TSet<FString>` 已访问 .psd 绝对路径,**环路立即报错**
- 子 PSD 内部 Bounds **不**映射回主图层,主图层 Bounds 仅决定外层 Slot 占位

### PSB / 大文件

- PSB(>2GB)由 PsdReader 检测格式头(`8BPB` 而非 `8BPS`)并立即报错
- 设计师需要把超大 PSD 拆成多个普通 PSD,通过 `#linkedpsd` 组装

## 5. PSD Schema(命名 + JSON)

### 命名格式

`<Name>#<tag>[(args)][#<tag>(args)]...`

生产可用 tag 集:

| tag | 作用 | UMG 输出 |
|---|---|---|
| `#image`(默认) | 图片层 | UImage |
| `#9slice(L,R,T,B)` | 九宫格 | UImage + Margin Brush, DrawAs=Box |
| `#button` | 按钮 | UButton 或 UCommonButtonBase |
| `#button_normal/_hovered/_pressed/_disabled` | 按钮态 | 同名 button 合并 |
| `#progress(bg/fill/marquee)` | 进度条 | UProgressBar / UCommonProgressBar |
| `#text` | 文本 | UTextBlock / UCommonTextBlock |
| `#sizebox(W=x,H=y)` | 固定尺寸 | USizeBox |
| `#scalebox` | 自适应缩放 | UScaleBox |
| `#slot` | 命名插槽 | UNamedSlot |
| `#anchor(TL/T/TR/L/C/R/BL/B/BR/Stretch)` | 覆盖默认锚点 | CanvasSlot |
| `#linkedpsd(RelPath.psd)` | 引用相对路径的子 PSD | UUserWidget 包装节点 + 递归导入子 WBP |
| `#vanilla` | 强制不走 CommonUI | UButton / UTextBlock |
| `#skip` | 不导入 | — |

### 伴随 JSON(可选,放 `<PsdPath>.psd.json`)

```json
{
  "version": 1,
  "globals": {
    "designDpi": 1920,
    "useCommonUI": true
  },
  "layers": {
    "Title": {
      "textStyle": "/Game/UI/Styles/TS_Title.TS_Title",
      "fontFace": "/Game/Fonts/MainFont.MainFont"
    },
    "PlayBtn": {
      "commonButtonStyle": "/Game/UI/Styles/BSt_Primary.BSt_Primary"
    }
  }
}
```

优先级:**JSON 字段 > 命名 tag args > 项目默认设置(UDeveloperSettings)**。

### 默认锚点策略

未指定 `#anchor` 时,按图层中心点相对父容器的位置落到 9 宫格(0.25/0.75 阈值),保留与
原版兼容的简单自适应。

## 6. Reimport 增量策略

按 `WidgetName` 匹配现有 WBP 树:

| 情况 | 行为 |
|---|---|
| 新增图层 | 新建 Widget 节点,新建纹理 |
| 删除图层 | **不删** Widget(避免误删用户绑定),MessageLog 警告 |
| 同名图层 + Bounds 变 | 更新 Slot Position/Size/Anchors,**保留**蓝图绑定/事件/变量 |
| 同名图层 + 像素变(像素 hash 不同) | 复写 Texture Source;hash 相同则跳过 |
| `#linkedpsd` 引用的子 PSD 变 | 子 WBP 单独 Reimport,父 WBP 不动 |
| 同名图层但 Type 变 | MessageLog 错误,跳过该层并提示用 Force Rebuild |

**强制覆盖**:AssetActions 右键 "Force Rebuild from PSD",清空 WBP 重建。

## 7. 错误处理与日志

- `FMessageLog("PSD2UMG")` 独立面板,自动在导入失败时弹出
- 三档:Error(整体失败,Factory 返回 nullptr) / Warning(单层失败,继续) / Info
- 所有消息带 PSD 图层路径,例:
  `MainMenu.psd → Buttons/PlayBtn#buton(typo) → unknown tag "buton", treated as #image`

## 8. 测试策略

### 样本 PSD(`Tests/Sample/`)

| 文件 | 覆盖点 |
|---|---|
| `Simple.psd` | 单 Canvas + 3 个 Image,验证锚点 9 宫 |
| `Nested.psd` | 3 层 Group 嵌套,验证 GROUP → CanvasPanel 递归 |
| `Buttons.psd` | `#button` 三态 |
| `NineSlice.psd` | `#9slice(8,8,8,8)` 边距应用、DrawAs=Box |
| `LinkedPsd.psd` + `Avatar.psd` | `#linkedpsd(Avatar.psd)` 引用 sibling 文件,生成 `WBP_Avatar` 并嵌入主 WBP |

每个样本配 `<name>.expected.json`,描述期望的层数、层名、Bounds 哈希、生成的 Widget 类型与
Slot 属性。

### Automation Spec(`Tests/Spec/`)

| Spec | 范围 | 依赖 |
|---|---|---|
| `PsdReader.spec.cpp` | 读 5 个样本,断言 `FPsdDocument` 与 expected.json 一致 | 不开 Editor |
| `PsdSchema.spec.cpp` | 命名/JSON 解析、tag 覆盖优先级、非法 tag 退回默认 | 不开 Editor |
| `UmgBuilder.spec.cpp` | 跑 Buttons.psd 全流程,断言 WBP 节点数/类型;Reimport 第二次后节点不变 | 需 Editor commandlet |

CI:本地 `UnrealEditor-Cmd.exe Project -ExecCmds="Automation RunTests PSD2UMG."`
能跑即视为 v1 合格;GitHub Actions 推到后续。

## 9. 风险与对冲

| 风险 | 对冲 |
|---|---|
| psd_sdk 不支持 Smart Object 嵌入字节 | 改用 `#linkedpsd(RelPath.psd)` 文件级引用,Importer 递归处理 sibling 文件 |
| psd_sdk 不支持 PSB(>2GB) | PsdReader 检测 `8BPB` 头并立即报 Error,设计师拆分 PSD |
| psd_sdk 文本图层只能取字符串,字体/字号等富属性不全 | v1 仅取文本字符串,样式靠 `.psd.json` 指定 TextStyle 资产;v2 再扩 |
| 16/32 bit PSD 当前降到 8bit 可能丢精度 | v1 在 MessageLog 给 Info 提示;v2 走 `TSF_RGBA16F` |
| `#linkedpsd` 文件路径写错 / 找不到 sibling | MessageLog Error,跳过该层,主 WBP 继续生成 |
| `#linkedpsd` 形成环路(A 引 B, B 引 A) | Importer 维护已访问 .psd 绝对路径集合,检测立即报错 |
| 5.7 上 `UAssetDefinition` 注册方式与 4.x 的 AssetTypeActions 差异大 | 先实现 AssetDefinition,跑通后定 |

## 10. 验收标准(v1 完工定义)

- 5 个样本 PSD 全部能一键导入,Automation Spec 全绿
- Reimport 5 个样本第二次,WBP 节点数 / 名称 / 关键属性与第一次一致
- MessageLog 在 4 种典型错误(未知 tag / JSON 拼错 / 类型变更 / `#linkedpsd` 环)上各给出可定位的消息
- 项目设置面板可关 CommonUI,关后 Buttons.psd 改生成 `UButton`
- 单 PSD 4K 含 30 层导入耗时 < 3 秒(老插件同样本基线 > 10 秒)
- 不再使用 `Runtime/UMG/Public/...` 绝对路径头文件
- `GetAssetRegistryTags(FAssetRegistryTagsContext)` 在 5.7 编译器零 deprecation warning
- PSB 文件导入时返回 Error 并在 MessageLog 给出明确提示
