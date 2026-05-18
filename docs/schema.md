# PSD 图层命名 + Sidecar 协议

PsdToUmg 从两个地方读控件生成指令:

1. **图层名 tag**(主要机制,设计师在 Photoshop 里直接控制)
2. **Sidecar `<Foo>.psd.json`**(可选,主要用来引用样式资产)

## 图层名格式

```
<Name>#<tag1>[(args)]#<tag2>[(args)]...
```

`<Name>` 会成为生成的 WidgetBlueprint 里 widget 的 `FName`。`#` 后面的 tag
追加类型、修饰或行为。多个 tag 可以叠加。

### Tag 字典(v1)

| Tag | 输出控件 | 参数 |
|---|---|---|
| `#image`(默认) | `UImage` | — |
| `#text` | `UTextBlock`(开 CommonUI 时为 `UCommonTextBlock`) | — |
| `#button` | `UButton`(或 `UCommonButtonBase`) | — |
| `#button_normal` / `#button_hovered` / `#button_pressed` / `#button_disabled` | 同名图层合并成一个 `UButton`,各状态填对应 brush | — |
| `#progress(bg\|fill\|marquee)` | `UProgressBar` 的对应部位 | — |
| `#9slice(L,R,T,B)` | `UImage` 上九宫格 margin,4 个像素值 | `8,8,8,8` |
| `#sizebox(W=,H=)` | `USizeBox`,带宽高 override | `W=200,H=80` |
| `#scalebox` | `UScaleBox` | — |
| `#slot` | `UNamedSlot`(运行时再填充) | — |
| `#anchor(...)` | 覆盖默认锚点预设 | `TL / T / TR / L / C / R / BL / B / BR / Stretch` 之一 |
| `#linkedpsd(RelPath)` | 引用同目录的兄弟 `.psd`,递归生成 `WBP_<base>`,在此处放一个 `UUserWidget` | `Avatar.psd` |
| `#vanilla` | 强制走原生 UMG(不走 CommonUI) | — |
| `#skip` | 该图层不导入 | — |

不认识的 tag 会进 MessageLog 警告,按 no-op 处理。

### 默认锚点策略

如果没有写 `#anchor(...)`,PsdToUmg 会按图层中心点在父容器(PSD 画布)中的相对位置推导:

| 在父容器里 X / Y 比例 | 结果 |
|---|---|
| < 25% | 起始锚(TL / T / L 等) |
| 25–75% | 居中锚 |
| > 75% | 末端锚 |

九宫格近似。自动推导不对的话用 `#anchor(...)` 显式覆盖。

## Sidecar `<Foo>.psd.json`

如果有一个跟 `<Foo>.psd` 同名同目录的 JSON 文件 `<Foo>.psd.json`,PsdToUmg 会
加载它,里面的字段优先级高于图层名 tag。

```json
{
  "version": 1,
  "globals": {
    "designDpi": 1920,
    "useCommonUI": true
  },
  "layers": {
    "PlayBtn": {
      "commonButtonStyle": "/Game/UI/Styles/BSt_Primary.BSt_Primary"
    },
    "Title": {
      "textStyle": "/Game/UI/Styles/TS_Title.TS_Title",
      "fontFace": "/Game/Fonts/MainFont.MainFont"
    }
  }
}
```

### 优先级

`JSON > 命名 tag args > UDeveloperSettings 默认值`

只有 `commonButtonStyle` / `textStyle` / `fontFace` 引用的资产**需要**在工程里
真实存在。资产缺失时导入会给一条 MessageLog 警告,然后用默认样式继续。

## 示例

| PSD 图层名 | 生成的控件 |
|---|---|
| `Background` | 名为 `Background` 的 `UImage`,锚点自动推导 |
| `PlayBtn#button_normal` + `PlayBtn#button_hovered` + `PlayBtn#button_pressed` | 一个名为 `PlayBtn` 的 `UButton`,3 张 brush 全填好 |
| `Panel#9slice(8,8,8,8)` | `UImage`,margin `{L:8,R:8,T:8,B:8}`,`DrawAs=Box` |
| `Avatar#linkedpsd(Avatar.psd)` | 引用 `WBP_Avatar` 的 `UUserWidget`(同时递归导入 Avatar.psd) |
| `Title#text#vanilla` | `UTextBlock`(不走 CommonUI) |
| `Header#slot` | `UNamedSlot`,运行时填 |
