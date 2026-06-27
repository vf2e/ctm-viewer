# CTM Viewer 新人上手指南

本文档面向**首次接手本项目**的开发者或测试人员，目标是在 30–60 分钟内完成环境搭建、成功运行程序，并理解代码大致分工。

> 更深入的架构与算法说明见 [TECHNICAL.md](./TECHNICAL.md)  
> 单元测试说明见 [TESTING.md](./TESTING.md)

---

## 1. 项目是什么

**CTM Viewer** 是一个独立的 Windows 桌面应用，用于：

1. 加载并显示 **CTM / OBJ** 三维网格模型；
2. 展示 **八字拍（YRD-B9076 / B090C）** 经颅磁刺激电场仿真效果；
3. 通过滑条调节线圈 6 自由度位姿、刺激强度、色标范围、头皮透明度等参数。

本项目从以下仓库抽离核心能力：

| 来源 | 抽离内容 |
|------|----------|
| `magaim-qt` | CTM/OBJ 加载、VTK Win32 渲染、OpenCTM 依赖 |
| `aimQ-Frontend` | 八字拍预计算电场数据、着色算法思路 |

---

## 2. 开发环境要求

| 组件 | 版本 / 说明 |
|------|-------------|
| 操作系统 | Windows 10/11 x64 |
| 编译器 | Visual Studio 2019（MSVC v142）或兼容工具链 |
| CMake | ≥ 3.16 |
| Qt | 5.12+，推荐 **5.15.2 msvc2019_64** |
| VTK | **8.0**（随 magaim-qt 提供，勿自行混用其他版本） |
| OpenCTM | 随 magaim-qt 提供 |
| Node.js | 可选，仅用于重新生成电场二进制数据 |

### 2.1 必须准备的外部路径

默认 CMake 缓存假设 magaim-qt 位于：

```
D:/2025WorkSpace/magaim-qt
```

其中应包含：

```
magaim-qt/
├── 3dLib/VTK64/          # VTK 8.0 头文件、lib、dll
└── 3dLib/openctm/        # openctm.lib / openctm.dll
```

若路径不同，配置时通过 `-DMAGIMAIM_QT_ROOT=...` 指定。

### 2.2 Qt 路径

常见安装位置示例：

```
C:\qt5.15.2\5.15.2\msvc2019_64
```

通过环境变量或 CMake 参数传入：

```powershell
$env:CMAKE_PREFIX_PATH = "C:\qt5.15.2\5.15.2\msvc2019_64"
```

---

## 3. 第一次构建（逐步）

在项目根目录 `ctm-viewer/` 打开 PowerShell：

```powershell
# 1. 配置
cmake -S . -B build `
  -DCMAKE_PREFIX_PATH="C:\qt5.15.2\5.15.2\msvc2019_64" `
  -DMAGIMAIM_QT_ROOT="D:/2025WorkSpace/magaim-qt" `
  -DBUILD_TESTING=ON

# 2. 编译 Release
cmake --build build --config Release

# 3. 运行
.\build\Release\ctm-viewer.exe
```

构建成功后：

- 可执行文件：`build/Release/ctm-viewer.exe`
- `assets/` 目录会自动复制到 exe 同级（POST_BUILD）
- VTK / OpenCTM 的 DLL 也会复制到输出目录

### 3.1 常见问题

| 现象 | 可能原因 | 处理 |
|------|----------|------|
| `VTK headers not found` | `MAGIMAIM_QT_ROOT` 错误 | 检查路径，重新 cmake |
| `openctm.lib not found` | OpenCTM 目录缺失 | 确认 magaim-qt 完整 |
| 启动后场景空白 | `assets/` 未复制或模型缺失 | 检查 exe 旁 `assets/models/` |
| 缺少 Qt DLL | 未部署 Qt 运行时 | 使用 `scripts/package.ps1` 打包 |

---

## 4. 目录结构速览

```
ctm-viewer/
├── CMakeLists.txt           # 主工程 + 测试开关 BUILD_TESTING
├── README.md                # 项目概览
├── assets/                  # 运行时资源（模型、电场数据、图标）
│   ├── data/b9076_efield.bin
│   ├── models/*.ctm
│   └── icon.ico
├── docs/                    # 文档
│   ├── ONBOARDING.md        # 本文档
│   ├── TECHNICAL.md         # 技术详解
│   ├── TESTING.md           # 测试文档
│   └── 软件设计风格规范.md
├── resources/               # Qt 资源与 Windows exe 图标
├── scripts/
│   ├── package.ps1          # 一键打包
│   ├── run_tests.ps1        # 运行单元测试
│   └── convert_efield.js    # 从 aimQ JS 生成电场 bin
├── src/                     # 应用源码
└── tests/                   # Qt Test 单元测试
```

---

## 5. 源码模块分工（先看这些文件）

| 文件 | 职责 | 新手建议 |
|------|------|----------|
| `main.cpp` | 应用入口、设置图标与样式 | 5 分钟浏览 |
| `MainWindow.*` | 主界面布局、控件、主题、快捷键 | **优先阅读** |
| `VtkRenderWidget.*` | VTK 渲染、场景、线圈、漫游、FPS | **核心渲染** |
| `FieldSimulator.*` | 电场数据加载与 IDW 插值着色 | **核心算法** |
| `FieldColorMap.*` | 灰脑 → 热力图色标映射 | 算法简单，适合写测试 |
| `ModelLoader.*` | CTM / OBJ → vtkPolyData | 与 magaim-qt 同源 |
| `FloatPanel.*` | 左右可折叠浮层面板 | UI 组件 |
| `ColorBarWidget.*` | 右侧竖向色标条绘制 | UI 组件 |
| `SettingsDialog.*` | 主题设置弹框 | UI 组件 |
| `AppTheme.h` | 四套主题 QSS + 视口背景色 | 纯头文件样式 |

### 5.1 数据流（简化）

```
启动
  → MainWindow 构造
  → VtkRenderWidget::loadSimulationScene(assets/)
       → FieldSimulator::loadBinary(b9076_efield.bin)
       → ModelLoader 加载 head / brain / coil
  → 用户调节线圈滑条
       → VtkRenderWidget::setCoilPose
       → FieldSimulator::applyToBrain (IDW)
       → FieldColorMap::colorForValue
       → VTK 顶点颜色更新
```

---

## 6. 界面操作说明

应用**无顶部菜单栏**，功能集中在左右浮层：

### 左侧面板

| 控件 | 功能 |
|------|------|
| 打开 | 加载附加 CTM/OBJ 模型 |
| 重置视角 | 恢复启动时保存的相机位姿（`Home`） |
| 设置 | 打开主题选择弹框 |
| 漫游 | 线圈在头模附近随机平滑运动 |
| 电场仿真 | 开关顶点场强着色 |
| FPS | 开关左下角帧率叠加 |
| 线圈模型 | 显示/隐藏八字拍网格（默认隐藏，显示坐标轴） |
| 刺激强度 | 0–100%，缩放场强 |
| 线圈 X/Y/Z/Rx/Ry/Rz | 6 自由度位姿（mm / 度） |
| 头皮透明度 | 半透明头皮 |

### 右侧面板

| 控件 | 功能 |
|------|------|
| 色标条 | 灰 → 蓝 → 青 → 黄 → 红 连续映射预览 |
| 下限 / 上限 SpinBox | 直接输入 LUT 范围（V/m，1–999） |

### 快捷键

| 快捷键 | 功能 |
|--------|------|
| `Ctrl+O` | 打开模型 |
| `Home` | 重置视角 |

### 3D 交互

| 操作 | 方式 |
|------|------|
| 旋转 | 左键拖动 |
| 平移 | 中键拖动 |
| 缩放 | 滚轮 |

---

## 7. 资源文件说明

### 7.1 电场二进制 `b9076_efield.bin`

格式（小端）：

```
uint32 sampleCount
重复 sampleCount 次：
  float x, y, z, magnitude   # 各 4 字节
```

重新从 aimQ 前端 JS 生成：

```bash
node scripts/convert_efield.js
```

> 脚本内源路径需指向本机 `b9076EField.js` 实际位置。

### 7.2 模型文件

| 文件 | 说明 |
|------|------|
| `brain_mask.ctm` | 脑组织 mask，场带着色目标 |
| `head.ctm` | 半透明头皮 |
| `coil_b090c.ctm` | 八字拍线圈几何 |

---

## 8. 打包与交付

```powershell
.\scripts\package.bat
```

输出：`dist/ctm-viewer-1.0.0-win64/` 及同名 zip，内含 Qt 运行时与全部依赖。

---

## 9. 运行单元测试

```powershell
.\scripts\run_tests.ps1
```

或：

```powershell
ctest --test-dir build -C Release --output-on-failure
```

详见 [TESTING.md](./TESTING.md)。

---

## 10. 推荐学习路径（第 1 周）

| 天数 | 任务 |
|------|------|
| Day 1 | 按本文档完成构建与运行，熟悉 UI 操作 |
| Day 2 | 阅读 `MainWindow` + `VtkRenderWidget::loadSimulationScene` |
| Day 3 | 阅读 `FieldSimulator` + `FieldColorMap`，跑通单元测试 |
| Day 4 | 阅读 `ModelLoader`，尝试替换/加载自定义 CTM |
| Day 5 | 阅读 `docs/TECHNICAL.md`，理解 VTK Win32 嵌入约束 |
| Day 6–7 | 小改动练手（如调整默认线圈位姿、色标范围、主题色） |

---

## 11. 交接清单

接手时请确认对方已提供或可自行访问：

- [ ] magaim-qt 完整目录（VTK64 + openctm）
- [ ] Qt 5.15 MSVC 安装路径
- [ ] `assets/` 下全部模型与电场数据
- [ ] 能成功 `cmake --build` 并启动 exe
- [ ] 能成功 `.\scripts\run_tests.ps1`
- [ ] 了解 aimQ / magaim-qt 上游仓库位置（用于追溯算法来源）

---

## 12. 获取帮助

| 问题类型 | 查阅 |
|----------|------|
| 构建失败 | 本文档 §3.1 + README |
| 算法与架构 | [TECHNICAL.md](./TECHNICAL.md) |
| 测试失败 | [TESTING.md](./TESTING.md) |
| UI 风格规范 | [软件设计风格规范.md](./软件设计风格规范.md) |
