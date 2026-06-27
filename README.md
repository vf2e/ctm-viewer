# CTM Viewer

独立的 CTM / OBJ 三维模型查看器，集成 **八字拍（YRD-B9076 / B090C）电场仿真**，从 [aimQ-Frontend](D:/2025WorkSpace/aimQ-Frontend) 与 [magaim-qt](D:/2025WorkSpace/magaim-qt) 抽离核心逻辑。

## 功能

- **默认灰白脑模**（`#787878`）+ 半透明头皮 + 八字拍线圈
- **电场仿真**：预计算场数据网格插值着色（与 aimQ `setColor` 算法一致）
- 刺激强度、色标上下限、线圈高度、头皮透明度调节
- 右侧色标条（青 → 黄 → 红）
- 打开附加 `.ctm` / `.obj` 模型
- 鼠标旋转 / 平移 / 缩放

## 资源来源

| 资源 | 来源 |
|------|------|
| `assets/data/b9076_efield.bin` | `aimQ-Frontend/.../b9076EField.js` 转换 |
| `assets/models/brain_mask.ctm` | `aimQ-Frontend/statics/mni/mask.ctm` |
| `assets/models/head.ctm` | `magaim-qt/.../head.ctm` |
| `assets/models/coil_b090c.ctm` | `magaim-qt/.../B090C.ctm`（八字拍线圈） |

重新生成场数据：

```bash
node scripts/convert_efield.js
```

## 构建

| 依赖 | 来源 |
|------|------|
| Qt 5.12+ (Widgets, OpenGL) | 自行安装，配置 `CMAKE_PREFIX_PATH` |
| VTK 8.0 | `magaim-qt/3dLib/VTK64` |
| OpenCTM | `magaim-qt/3dLib/openctm` |

## 构建

```powershell
# 设置 Qt 路径（按本机实际安装路径修改）
$env:CMAKE_PREFIX_PATH = "C:\Qt\5.15.2\msvc2019_64"

cmake -S . -B build -DMAGIMAIM_QT_ROOT="D:/2025WorkSpace/magaim-qt"
cmake --build build --config Release
```

构建完成后，可执行文件位于 `build/Release/ctm-viewer.exe`（或 `build/Debug/`）。构建过程会自动将 VTK 与 OpenCTM 的 DLL 复制到输出目录。

### CMake 选项

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `MAGIMAIM_QT_ROOT` | `D:/2025WorkSpace/magaim-qt` | magaim-qt 项目根目录 |
| `VTK_DIR` | `{MAGIMAIM_QT_ROOT}/3dLib/VTK64/lib/cmake/vtk-8.0` | VTK CMake 配置 |
| `OPENCTM_DIR` | `{MAGIMAIM_QT_ROOT}/3dLib/openctm` | OpenCTM 库目录 |

## 项目结构

```
ctm-viewer/
├── CMakeLists.txt
├── README.md
└── src/
    ├── main.cpp
    ├── MainWindow.cpp/h      # 主窗口与 UI（参考 Mag-aim 扁平风格）
    ├── VtkRenderWidget.cpp/h # QOpenGLWidget + VTK 渲染
    └── ModelLoader.cpp/h     # CTM/OBJ 加载（来自 magaim-qt）
```

## 参考来源

- `magaim-qt/loader/modelloader.cpp` — CTM / OBJ 加载
- `magaim-qt/vtk/vtkitem.cpp` — VTK OpenGL 渲染与交互
- `docs/软件设计风格规范.md` — UI 色彩与控件规范

## 操作说明

| 操作 | 方式 |
|------|------|
| 旋转 | 左键拖动 |
| 平移 | 中键拖动 |
| 缩放 | 滚轮 |
| 调节透明度 | 顶部滑条（0%–100%） |
| 打开文件 | 菜单「文件 → 打开」或工具栏按钮 |
