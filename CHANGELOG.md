# 更新日志

nbody-gpu-sim 是一个基于 CUDA 加速的 N 体粒子模拟系统，提供实时可视化、多种受力算法与命令行工具。
格式基于 [Keep a Changelog](https://keepachangelog.com/zh-CN/1.1.0/)，版本号遵循[语义化版本](https://semver.org/lang/zh-CN/)。

## [Unreleased]

### 新增

- 新增应用内运行时控制与诊断面板（Dear ImGui，F1 切换），显示 FPS、帧时间、粒子数、模拟时间与受力方法，支持暂停/重置与受力方法切换；新增 `NBODY_ENABLE_UI` 构建选项。
- 新增 Barnes-Hut 受力算法的基准/正确性基线，输出分阶段耗时用于回归检测。
- 文档站迁移到 VitePress，加入算法对比、性能表格、Mermaid 架构图与 llms.txt。

### 变更

- 项目与仓库更名为 nbody-gpu-sim（原 n-body），README 重写为功能/算法/CLI/架构完整章节，LICENSE 版权归属改为 Vibe Knight。
- 文档与站点统一中文化：README 中文升为主文档，删除 README.zh-CN.md 与站点 en/zh-CN 双语及语言跳转，架构图改纯文本并移除 mermaid 依赖。
- 大幅精简项目：4 个 AI 指令文件合并为单一 AGENTS.md，删除治理模板与 30+ 冗余子页面，去除头文件 doxygen 样板注释，markdown 文件从 68 个降至 10 个。
- 提升构建可复现性与运行时异常安全：rapidcheck 固定到具体 commit、imgui 固定到 release tag，窗口改用 RAII（unique_ptr + GLFWWindowDeleter）管理，glm 依赖收窄到渲染路径。
- README 收敛为快速开始，新增浏览器框架构截图与浅色模式下的交互式中文架构图，文档站字体改为自托管。

### 修复

- 修复 CUDA 构建下 `types.hpp` 中 int3 重复定义，以及其 CUDA 检测条件与 error_handling.hpp 不一致导致的编译错误。
- `integrator.cu` 补充 `include force_calculator.hpp`，消除“pointer to incomplete class type”CUDA 编译错误。
- GPU 无关测试改用范围校验（`validateParticleCountRange`），避免在无 GPU 的 CI runner 上探测 CUDA 设备失败。
- 修复 GitHub Pages 语言跳转路径错误，并更新 LessUp → AICL-Lab 的历史引用与失效链接。

### 移除

- 移除冗余文档与治理开销（issue/PR 模板、CODEOWNERS、FUNDING、dependabot、Doxyfile 等）。
- 移除 HDF5 导出/导入及其依赖与测试，导入/导出统一改用 Serializer 私有检查点格式（该功能在同一开发周期内先加入后回退）。

## [v2.2.0] - 2026-04-27

### 新增

- 项目初始版本：基于 CUDA 加速的 N 体粒子模拟系统与实时 OpenGL 可视化。
- 多种受力计算算法：Barnes-Hut、空间哈希网格与直接求和。
- 项目基础设施：README、CMake 构建、示例程序与脚本。
- GitHub Actions CI 工作流与更新日志；GitHub Pages 站点（Jekyll）。
- 双语（中/英）文档支持。
- 接入 RapidCheck 属性测试并隔离模拟状态；新增性能可观测性模块。

### 变更

- v2.0.0 大规模重构：修复 atomicMin/Max 对负数浮点的处理、修复 SpatialHashGrid 未初始化指针、GPU 包围盒归约、预分配积分器临时缓冲、按需 CUDA_CHECK_KERNEL 同步、现代化 CMakeLists。
- 支持仅 CPU 构建（`NBODY_ENABLE_CUDA` 选项），build.sh 自动探测 CUDA 可用性。
- 优化项目目录结构并规范化项目治理。
- GitHub Pages 采用 Just the Docs 主题重做。
- CI 调整：clang-format v18、markdownlint、Node.js 24 兼容。

### 修复

- 关键 bug 修复与文档改进。
- 加固运行时校验并清理无用代码。
- 移除导致 GitHub Pages 构建失败的 custom SCSS，修正文档链接路径与 Gemfile 中重复的 gem 声明。
