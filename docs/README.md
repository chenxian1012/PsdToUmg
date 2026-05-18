# PsdToUmg

Adobe Photoshop `.psd` → Unreal Motion Graphics `UWidgetBlueprint` 导入器,**Unreal Engine 5.7** 适用。

## 它做什么

把 `.psd` 文件拖进 Content Browser,PsdToUmg 会:

- 用 MolecularMatters/psd_sdk 解析每一个图层(已 vendor 进来,纯 C++,零外部依赖)
- 读取可选的 `<Foo>.psd.json` 旁路文件做样式覆盖
- 每个 raster 图层生成一张 `UTexture2D`(TC_EditorIcon / sRGB / 不带 mipmap)
- 按命名约定建出 `UWidgetBlueprint`,节点类型涵盖 `UCanvasPanel` / `UImage` /
  `UButton` / `UProgressBar` / `UTextBlock` / `USizeBox` / `UScaleBox` /
  `UNamedSlot` / `UUserWidget`(详见 `schema.md`)
- 支持通过标准 Reimport 菜单重导

`docs/schema.md` 是图层命名 + sidecar 协议文档,`docs/samples.md` 是测试样本说明。

## 快速上手

1. 在你项目的 `.uproject` 里启用插件。
2. 在 Content Browser 里右键 `.psd` → Import。
3. 输出落在 `/Game/UI/PsdImport/<PsdName>/` 下:
   - `WBP_<PsdName>.uasset` — WidgetBlueprint
   - `T_<LayerName>.uasset` — 单层贴图
   - `PsdCache_<PsdName>.uasset` — Reimport 用的句柄资产

通过 **Project Settings → Plugins → PSD2UMG** 改默认输出路径或关闭 CommonUI 集成。

## 仓库结构

```
PSD2UMG.uplugin                     插件清单 (UE 5.7, Win64)
Source/PSD2UMG/                     编辑器模块(Importer / Schema / Builder / Settings / Asset)
Source/psd_sdk/                     vendor 进来的 MolecularMatters/psd_sdk 快照
Source/PSD2UMGTests/                Automation Spec 测试(DeveloperTool)
Tests/Sample/                       可重新生成的测试样本(generate_samples.py)
HostProject/                        测试时用的宿主 UE 工程
docs/                               本文档
```

## 从源码构建

```bash
"D:\ue\UE_5.7\Engine\Build\BatchFiles\Build.bat" \
  HostProjectEditor Win64 Development \
  -Project="D:\Ai\Project\PSD2UMG_5.7\HostProject\HostProject.uproject" \
  -WaitMutex
```

插件会编译两个 UE 模块进宿主工程:`psd_sdk`(vendor 的解析器)+ `PSD2UMG`
(编辑器集成层)。还有第三个 `PSD2UMGTests` 跑 Automation Spec 套件。

## 跑测试套件

```bash
"D:\ue\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" \
  "D:\Ai\Project\PSD2UMG_5.7\HostProject\HostProject.uproject" \
  -ExecCmds="Automation RunTests PSD2UMG.; Quit" \
  -unattended -nopause -NullRHI -log
```

预期:36 / 36 通过,exit code 0。

## 仓库 bootstrap(clone 后的步骤)

`HostProject/Plugins/PSD2UMG` 是一个 NTFS **junction**,指向仓库根 ——
没有纳入 git。clone 仓库后手动重建一次:

```cmd
mklink /J "HostProject\Plugins\PSD2UMG" ".\"
```

(Windows 上需要开启开发者模式,或者用管理员命令行;`/J` 是 junction,不需要提权。)

## 已知限制(v1)

- **不支持 Smart Object 像素提取**(psd_sdk 限制)。改用
  `#linkedpsd(SubFile.psd)` 图层命名约定把兄弟 PSD 嵌入为子 WBP,详见 `schema.md`。
- **不支持 PSB(> 2GB)文件**。PsdReader 会通过 MessageLog 报错。
- **16 / 32 bit PSD** 可读,但 texture builder 阶段会降到 8 bit,v2 会换
  `TSF_RGBA16F` 保精度。
- **样式资产**(`CommonTextStyle` / `CommonButtonStyle`)在 CommonUI 5.7 是
  abstract 类,v1 stub builder 返回空 soft path。v2 会让你通过项目设置
  指定具体子类。
- **Reimport 幂等性**只保证 widget 数量稳定,**不**做原地 merge。v1 是
  清空 + 重建,用户手工改的 Slot 位置不会保留。

## 许可证

vendor 的解析器见 `Source/psd_sdk/LICENSE.psd_sdk`(BSD 2-Clause © Molecular Matters)。
插件本体的许可证在仓库根的 `LICENSE` 文件里。
