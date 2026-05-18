# 样本 PSD

`Tests/Sample/` 下放着 6 个可重新生成的样本 PSD,由
`Tests/Sample/generate_samples.py` 产出(用 `psd-tools` + Pillow)。
这些样本通过 Automation Spec 测试覆盖了 PsdToUmg 的完整管线。

| 文件 | 画布 | 图层数 | 测什么 |
|---|---|---|---|
| `Simple.psd` | 1920×1080 | 3(Background, Logo, Footer) | 基础 Image 控件生成,9 宫格锚点自动推导 |
| `Nested.psd` | 1920×1080 | 3(HUD_Score, HUD_Time, Menu_Buttons_Play) | 图层名编码的嵌套(真正的 group 嵌套留待后续) |
| `Buttons.psd` | 1920×1080 | 3(PlayBtn#button_normal / _hovered / _pressed) | 同名 button 多状态合并成一个 `UButton`,3 张 brush 全填好 |
| `NineSlice.psd` | 1920×1080 | 1(Panel#9slice(8,8,8,8)) | `#9slice` tag → `FSlateBrush.Margin` + `DrawAs=Box` |
| `LinkedPsd.psd` | 1920×1080 | 1(Avatar#linkedpsd(Avatar.psd)) | `#linkedpsd` 递归导入 → 子 `WBP_Avatar` |
| `Avatar.psd` | 256×256 | 1(Body) | 被 `LinkedPsd.psd` 引用的子 PSD |

每个 `.psd` 都配了 `<Name>.expected.json` —— `FPsdReader::Read` 输出的快照。
`PsdReader.spec.cpp` 跑的时候会拿当前 reader 的输出和快照比对。

`Buttons.psd` 还有一个 `Buttons.psd.json` sidecar,用来给 `SidecarLoader.spec.cpp`
和 Factory 测试验证 `commonButtonStyle` 能正确流到 `FWidgetSpec::StyleAssetRef`。

## 重新生成

样本图层结构如果要改:

```bash
cd Tests/Sample
python generate_samples.py
```

然后切换 `Source/PSD2UMGTests/Private/Spec/PsdReader.spec.cpp` 顶部的
`PSD2UMG_REGENERATE_SNAPSHOTS` 来更新快照:

1. 把 `#define PSD2UMG_REGENERATE_SNAPSHOTS 1`
2. 重新 build,跑 `Automation RunTests PSD2UMG.PsdReader`(此时会把新 `.expected.json` 写出来)
3. 改回 `#define PSD2UMG_REGENERATE_SNAPSHOTS 0`
4. 重新 build,再跑一遍,确认 assertion 模式也过
5. 提交
