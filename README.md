# CTM Viewer

独立的 CTM / OBJ 三维模型查看器，集成 **八字拍（YRD-B9076 / B090C）电场仿真**，从 aimQ-Frontend 与 magaim-qt 抽离核心逻辑。

## 功能

- **默认灰白脑模** + 半透明头皮 + 八字拍线圈（默认隐藏，坐标轴示意）
- **电场仿真**：预计算场数据 IDW 插值着色（与 aimQ 算法一致）
- 线圈 **6 自由度**滑条、刺激强度、LUT 上下限（SpinBox 直接输入）
- 右侧连续色标条（灰 → 蓝 → 青 → 黄 → 红）
- 可折叠左右浮层面板、四套主题（设置弹框切换）
- 漫游模式、FPS 叠加、打开附加 `.ctm` / `.obj`
- 应用图标 `icon.ico`（窗口 + exe）

## 文档

| 文档 | 说明 |
|------|------|
| [docs/ONBOARDING.md](docs/ONBOARDING.md) | **新人上手指南** — 环境、构建、目录、操作 |
| [docs/TECHNICAL.md](docs/TECHNICAL.md) | **技术说明** — 架构、VTK 嵌入、电场算法、扩展 |
| [docs/TESTING.md](docs/TESTING.md) | **单元测试** — 用例清单、运行方式、如何新增 |
| [docs/软件设计风格规范.md](docs/软件设计风格规范.md) | UI 色彩与控件规范 |

## 资源来源

| 资源 | 来源 |
|------|------|
| `assets/data/b9076_efield.bin` | `aimQ-Frontend/.../b9076EField.js` 转换 |
| `assets/models/brain_mask.ctm` | aimQ `mask.ctm` |
| `assets/models/head.ctm` | magaim-qt `head.ctm` |
| `assets/models/coil_b090c.ctm` | magaim-qt `B090C.ctm` |

重新生成场数据：

```bash
node scripts/convert_efield.js
```

## 构建

| 依赖 | 来源 |
|------|------|
| Qt 5.12+ (Widgets) | 自行安装，配置 `CMAKE_PREFIX_PATH` |
| VTK 8.0 | `magaim-qt/3dLib/VTK64` |
| OpenCTM | `magaim-qt/3dLib/openctm` |

```powershell
cmake -S . -B build `
  -DCMAKE_PREFIX_PATH="C:\qt5.15.2\5.15.2\msvc2019_64" `
  -DMAGIMAIM_QT_ROOT="D:/2025WorkSpace/magaim-qt" `
  -DBUILD_TESTING=ON

cmake --build build --config Release
```

输出：`build/Release/ctm-viewer.exe`（自动复制 assets、VTK/OpenCTM DLL）

### CMake 选项

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `MAGIMAIM_QT_ROOT` | `D:/2025WorkSpace/magaim-qt` | magaim-qt 根目录 |
| `BUILD_TESTING` | `ON` | 构建 Qt Test 单元测试 |

## 单元测试

```powershell
.\scripts\run_tests.ps1
```

或 `ctest --test-dir build -C Release --output-on-failure`

覆盖模块：`FieldColorMap`、`FieldSimulator`、`ModelLoader`、`AppTheme`。详见 [docs/TESTING.md](docs/TESTING.md)。

## 一键打包

```powershell
.\scripts\package.bat
```

输出 `dist/ctm-viewer-1.0.0-win64/` 及 zip。参数见脚本 `-SkipBuild`、`-NoZip`、`-QtPath`、`-MagaimQtRoot`。

## 项目结构

```
ctm-viewer/
├── CMakeLists.txt
├── README.md
├── assets/                  # 模型、电场数据、图标
├── docs/                    # 上手 / 技术 / 测试文档
├── resources/               # app.qrc、app.rc
├── scripts/                 # package、run_tests、convert_efield
├── src/
│   ├── main.cpp
│   ├── MainWindow.*         # 主窗口与控件
│   ├── VtkRenderWidget.*    # VTK Win32 渲染
│   ├── FieldSimulator.*     # 电场 IDW 着色
│   ├── FieldColorMap.*      # 色标映射
│   ├── ModelLoader.*        # CTM/OBJ 加载
│   ├── FloatPanel.*         # 可折叠侧栏
│   ├── ColorBarWidget.*     # 色标条
│   ├── SettingsDialog.*     # 主题设置
│   └── AppTheme.h           # 四套主题 QSS
└── tests/                   # Qt Test 单元测试
```

## 操作说明

| 操作 | 方式 |
|------|------|
| 旋转 / 平移 / 缩放 | 左键 / 中键 / 滚轮 |
| 打开模型 | 左栏「打开」或 `Ctrl+O` |
| 重置视角 | 左栏按钮或 `Home` |
| 切换主题 | 左栏「设置」 |
| LUT 上下限 | 右栏 SpinBox（V/m） |

## 参考来源

- `magaim-qt/loader/modelloader.cpp` — CTM / OBJ 加载
- `magaim-qt/vtk/vtkitem.cpp` — VTK 渲染与交互
- aimQ-Frontend — 电场数据与着色思路
