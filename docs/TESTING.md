# CTM Viewer 单元测试文档

本文档说明本项目的单元测试框架、用例覆盖、运行方式，以及如何新增测试。

---

## 1. 测试框架与策略

| 项目 | 选择 |
|------|------|
| 框架 | **Qt Test** (`Qt5::Test`) |
| 构建集成 | CMake `enable_testing()` + CTest |
| 运行方式 | 每个测试类独立可执行文件 |
| 覆盖重点 | 可确定性验证的**纯逻辑**与**数据层** |

### 1.1 为什么不用 GUI 自动化测试

- `VtkRenderWidget` 依赖 Win32 原生 OpenGL 窗口（`vtkWin32OpenGLRenderWindow`），在无显示环境下难以稳定自动化；
- `MainWindow` / `FloatPanel` 等 UI 以视觉与交互为主，回归成本高；
- 电场着色、色标映射、模型加载等**核心业务**已在数据层具备良好可测性。

当前策略：**数据层充分单测 + 主程序人工冒烟测试**。

### 1.2 测试目录结构

```
tests/
├── CMakeLists.txt
├── fixtures/                    # 静态测试夹具
│   └── triangle.obj
├── helpers/
│   ├── TestPaths.h              # 夹具路径解析
│   ├── VtkTestHelpers.h         # 构造 vtkPolyData
│   └── EfieldBinaryWriter.h     # 动态生成电场 bin
├── test_fieldcolormap.cpp
├── test_fieldsimulator.cpp
├── test_modelloader.cpp
└── test_apptheme.cpp
```

---

## 2. 构建与运行

### 2.1 启用测试

CMake 选项（默认 **ON**）：

```cmake
option(BUILD_TESTING "Build and register unit tests" ON)
```

关闭测试：

```powershell
cmake -S . -B build -DBUILD_TESTING=OFF
```

### 2.2 编译测试目标

```powershell
cmake --build build --config Release `
  --target test_fieldcolormap test_fieldsimulator test_modelloader test_apptheme
```

### 2.3 一键运行（推荐）

```powershell
.\scripts\run_tests.ps1
```

可选参数：

| 参数 | 说明 |
|------|------|
| `-Config Debug` | 使用 Debug 配置 |
| `-QtPath` | 指定 Qt 根目录 |
| `-MagaimQtRoot` | 指定 magaim-qt 路径 |
| `-VerboseOutput` | CTest 详细输出 |

### 2.4 使用 CTest

```powershell
ctest --test-dir build -C Release --output-on-failure
```

列出测试：

```powershell
ctest --test-dir build -C Release -N
```

### 2.5 单独运行某个测试

```powershell
.\build\Release\test_fieldcolormap.exe
.\build\Release\test_fieldsimulator.exe
.\build\Release\test_modelloader.exe
.\build\Release\test_apptheme.exe
```

> `test_modelloader` 的工作目录应能访问 `tests/fixtures/`。CTest 已通过 `WORKING_DIRECTORY` 配置；若手动运行 exe，请在 `tests/` 目录下执行，或确保 `TEST_FIXTURES_DIR` 编译宏指向正确路径。

---

## 3. 测试用例清单

### 3.1 `test_fieldcolormap` — 色标映射

被测模块：`src/FieldColorMap.cpp`

| 用例 | 验证点 |
|------|--------|
| `brainGrayRgb_matchesConstant` | 脑灰 RGB = round(0.58×255) = 148 |
| `setRange_enforcesMinimumSpan` | max 至少比 min 大 1 |
| `colorForValue_belowMin_returnsBrainGray` | 低于下限 → 脑灰 |
| `colorForValue_atMin_returnsBrainGray` | 等于下限 → 脑灰（未着色外观） |
| `colorForValue_aboveMax_clampsToHeatMax` | 超高值钳制到热力图顶端 |
| `colorForValue_midRange_differsFromGray` | 中间值产生非灰色 |
| `colorForNormalized_endpoints_areStable` | t=0 灰、t=1 高红 |
| `colorForNormalized_isMonotonicInRedNearHighEnd` | 高段红色单调不减 |
| `defaultRange_matchesProductDefaults` | 默认 40–150 V/m |

### 3.2 `test_fieldsimulator` — 电场仿真

被测模块：`src/FieldSimulator.cpp`

| 用例 | 验证点 |
|------|--------|
| `loadBinary_missingFile_fails` | 文件不存在 → 失败 + 错误信息 |
| `loadBinary_truncatedHeader_fails` | 文件过短 → 失败 |
| `loadBinary_truncatedPayload_fails` | 声明点数与数据不符 → 失败 |
| `loadBinary_validFile_succeeds` | 合法 bin → 成功 |
| `hasData_reflectsLoadState` | `hasData()` 与加载状态一致 |
| `isCoilWithinRange_handlesNullAndDistance` | 空矩阵 / 距离阈值 |
| `resetBrainColors_setsUniformGray` | 所有顶点设为脑灰 |
| `applyToBrain_colorsVerticesWithFieldData` | 加载场数据后产生非灰顶点色 |
| `applyToBrain_respectsIntensityScale` | 50% 与 100% 强度着色不同 |
| `applyToBrain_emptySamples_isNoOp` | 无样本时不写 scalars |

夹具：`initTestCase` 中用 `EfieldBinaryWriter` 动态生成 3 点样本 bin。

### 3.3 `test_modelloader` — 模型加载

被测模块：`src/ModelLoader.cpp`

| 用例 | 验证点 |
|------|--------|
| `loadModel_emptyPath_fails` | 空路径 |
| `loadModel_unsupportedExtension_fails` | 非 ctm/obj |
| `loadModel_missingFile_fails` | 文件不存在 |
| `loadModel_triangleObj_succeeds` | 加载 fixtures/triangle.obj |
| `loadModel_triangleObj_hasExpectedCounts` | 3 顶点、1 三角面 |

> CTM 加载测试未纳入 CI：依赖真实 CTM 夹具体积较大。可在本地将 `assets/models/brain_mask.ctm` 复制到 `tests/fixtures/` 后追加用例。

### 3.4 `test_apptheme` — 主题配置

被测模块：`src/AppTheme.h`（头文件内联）

| 用例 | 验证点 |
|------|--------|
| `availableThemes_hasFourEntries` | 4 套主题 |
| `viewportColors_returnsValidRgb` | 背景色在 [0,1] |
| `stylesheet_isNonEmptyForAllThemes` | 每套主题 QSS 非空且含关键选择器 |
| `settingsDialogStylesheet_containsKeySelectors` | 设置弹框样式完整 |
| `themeAccent_isValidHexColor` | 强调色为 `#RRGGBB` |

---

## 4. 测试辅助工具

### 4.1 `TestPaths.h`

编译时注入：

```cmake
TEST_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
```

提供 `TestPaths::fixture("triangle.obj")` 绝对路径。

### 4.2 `EfieldBinaryWriter.h`

按与生产环境相同的格式写入临时 bin：

```cpp
std::vector<EfieldSample> samples = {{0,0,0,120}, ...};
writeEfieldBinary(path, samples);
```

### 4.3 `VtkTestHelpers.h`

- `makeTrianglePolyData()` — 3 顶点三角面
- `makeGridPolyData(n, spacing)` — n³ 点网格，用于场插值测试

---

## 5. 如何新增测试

### 5.1 新增测试文件

1. 在 `tests/` 下创建 `test_mymodule.cpp`；
2. 继承 `QObject`，使用 `QTEST_APPLESS_MAIN`；
3. 在 `tests/CMakeLists.txt` 末尾添加：

```cmake
ctm_viewer_add_test(test_mymodule)
```

4. 若需链接额外源文件，扩展 `CTM_TEST_CORE_SOURCES`。

### 5.2 命名约定

| 类型 | 约定 | 示例 |
|------|------|------|
| 测试类 | `ModuleNameTest` | `FieldColorMapTest` |
| 用例 | `method_condition_expectation` | `loadBinary_missingFile_fails` |
| 夹具文件 | 小写 + 下划线 | `triangle.obj` |

### 5.3 编写建议

- 每个用例只验证一个行为；
- 优先测**边界**（空输入、截断文件、极值 LUT）；
- 避免依赖网络、绝对路径、随机数（除非固定种子）；
- VTK 相关测试构造最小 `vtkPolyData`，勿加载完整脑模；
- 使用 `QTemporaryDir` 生成临时电场文件，测完自动清理。

### 5.4 示例骨架

```cpp
#include "MyModule.h"
#include <QtTest>

class MyModuleTest : public QObject
{
    Q_OBJECT
private slots:
    void myCase();
};

void MyModuleTest::myCase()
{
    QVERIFY(true);
}

QTEST_APPLESS_MAIN(MyModuleTest)
#include "test_mymodule.moc"
```

---

## 6. CI 集成建议

若在 Azure Pipelines / GitHub Actions 上使用 Windows runner：

```yaml
- name: Configure
  run: cmake -S . -B build -DCMAKE_PREFIX_PATH=%QT_PATH% -DMAGIMAIM_QT_ROOT=%MAGIMAIM_QT_ROOT% -DBUILD_TESTING=ON

- name: Build tests
  run: cmake --build build --config Release --target test_fieldcolormap test_fieldsimulator test_modelloader test_apptheme

- name: Run tests
  run: ctest --test-dir build -C Release --output-on-failure
```

注意：测试链接 VTK 与 OpenCTM，runner 需能访问 magaim-qt 依赖，或将所需 DLL 放入 `PATH`。

---

## 7. 人工冒烟测试清单

单元测试无法覆盖的部分，发版前建议手动确认：

- [ ] 启动后默认场景（头、脑、线圈轴）正常显示
- [ ] 拖动线圈滑条，脑模颜色实时更新
- [ ] 修改 LUT 上下限，色标与着色同步变化
- [ ] 漫游模式线圈平滑运动，关闭后复位
- [ ] 打开附加 OBJ/CTM 成功
- [ ] 设置弹框切换四套主题
- [ ] 打包后 zip 在干净机器可运行

---

## 8. 故障排查

| 现象 | 原因 | 处理 |
|------|------|------|
| `Fixture missing: triangle.obj` | 工作目录错误 | 用 CTest 或 `run_tests.ps1` 运行 |
| 链接错误缺少 VTK | 未先配置主工程 | 先 `cmake -S . -B build` 再编测试 |
| `loadBinary` 测试失败 | 临时目录权限 | 检查杀毒软件 / 磁盘权限 |
| 全部通过但主程序异常 | 渲染/UI 未覆盖 | 执行 §7 冒烟清单 |
