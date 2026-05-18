# PsdToUmg

> Adobe Photoshop `.psd` → Unreal Motion Graphics `UWidgetBlueprint` 导入器。
> 项目内部模块名仍是 `PSD2UMG`,本仓库命名为 `PsdToUmg` 以便阅读。

[![UE 5.7](https://img.shields.io/badge/Unreal-5.7-313131?logo=unrealengine)](https://www.unrealengine.com/)
[![Win64](https://img.shields.io/badge/platform-Win64-0078D6)]()
[![License: MIT](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

把 `.psd` 拖进 Content Browser,PsdToUmg 自动:

- 用 vendor 进来的 [MolecularMatters/psd_sdk](https://github.com/MolecularMatters/psd_sdk) 解析图层(纯 C++,零外部依赖)
- 读取可选的 `<Foo>.psd.json` 旁路文件做样式覆盖
- 每个 raster 图层生成一张 `UTexture2D`(`TC_EditorIcon` / sRGB / 不带 mipmap)
- 按图层命名约定生成 `UWidgetBlueprint`,内部节点包括 `UCanvasPanel` / `UImage` /
  `UButton` / `UProgressBar` / `UTextBlock` / `USizeBox` / `UScaleBox` /
  `UNamedSlot` / `UUserWidget`
- 支持 `#linkedpsd(Avatar.psd)` 递归导入兄弟 PSD 作为子 WBP
- 支持通过标准菜单 Reimport

```
PlayBtn#button_normal        → 一个 UButton,三个 brush
PlayBtn#button_hovered       ↗
PlayBtn#button_pressed       ↗
Panel#9slice(8,8,8,8)        → UImage + 九宫格 margin
Avatar#linkedpsd(Avatar.psd) → UUserWidget 引用子 WBP_Avatar
```

## 快速上手

1. 在你项目的 `.uproject` 里启用插件。
2. 把 `.psd` 拖进 Content Browser。
3. 输出落到 `/Game/UI/PsdImport/<PsdName>/`。

完整图层命名 tag 字典和 sidecar JSON 协议见 **[docs/schema.md](docs/schema.md)**,
样本 PSD 测试夹具说明见 **[docs/samples.md](docs/samples.md)**。

## 当前状态

v2.0.0-rc1 — 针对 UE 5.7 的完整重写。**36 / 36 Automation Spec 测试通过**。

已知限制(v1):
- 不支持 Smart Object 像素提取(psd_sdk 限制)—— 改用 `#linkedpsd(...)` 文件级引用
- 不支持 PSB(> 2GB),`PsdReader` 检测到会通过 `MessageLog` 报错
- 16 / 32 bit PSD 能读,但在 texture builder 阶段会降为 8 bit
- CommonUI 的 `UCommonTextStyle` / `UCommonButtonStyle` 在 UE 5.7 是 abstract 类,
  v1 stub builder 返回空 soft path(v2 会让你指定具体子类)
- Reimport 只保证 widget 数量稳定;v1 是清空重建,**不**做原地 merge
- 仅 Win64 编辑器

## 从源码构建

```powershell
& "<UE_ROOT>\Engine\Build\BatchFiles\Build.bat" `
  HostProjectEditor Win64 Development `
  -Project="<this-repo>\HostProject\HostProject.uproject" -WaitMutex
```

`HostProject/Plugins/PSD2UMG` 是一个指向仓库根的 NTFS **junction**,**没有**纳入 git。
clone 后需要手动重建:

```cmd
mklink /J "HostProject\Plugins\PSD2UMG" "<仓库根绝对路径>"
```

`/J` 创建目录 junction,**不**需要管理员权限。

## 跑测试套件

```powershell
& "<UE_ROOT>\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" `
  "<this-repo>\HostProject\HostProject.uproject" `
  -ExecCmds="Automation RunTests PSD2UMG.; Quit" `
  -unattended -nopause -NullRHI -log
```

预期:36 / 36 通过,exit code 0。

## 仓库结构

```
PSD2UMG.uplugin                     插件清单 (UE 5.7, Win64)
Source/PSD2UMG/                     编辑器模块
  Private/Importer/                 PsdReader, PSD2UMGFactory
  Private/Schema/                   PsdDocument, NamingParser, SidecarLoader, SchemaResolver (仅依赖 Core)
  Private/Builder/                  TextureBuilder, StyleAssetBuilder, UmgBuilder
  Private/Settings/                 UDeveloperSettings
  Private/Asset/                    UPSD2UMGCache + UAssetDefinition
Source/psd_sdk/                     vendor 进来的 MolecularMatters/psd_sdk 快照
Source/PSD2UMGTests/                Automation Spec 测试
Tests/Sample/                       6 个可重新生成的样本 PSD + generate_samples.py
HostProject/                        跑测试时用的宿主 UE 工程(部分不纳入 git)
docs/                               README / schema / samples + 设计 + 计划
```

## 设计 + 计划文档

完整的设计推演和实现计划放在 `docs/superpowers/` 下。想知道为什么图层命名约定
是这个样子、或者重写过程是怎么拆解的,从那里看起。

## 许可证

[MIT](LICENSE)。vendor 进 `Source/psd_sdk/` 的代码遵循 BSD 2-Clause © Molecular Matters
(见 `Source/psd_sdk/LICENSE.psd_sdk`)。
