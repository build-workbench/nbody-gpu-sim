# n-body

基于 GPU 的高性能 N 体模拟项目，提供实时 CUDA/OpenGL 可视化。

[GitHub Pages](https://lessup.github.io/n-body/) · [快速开始](docs/setup/getting-started.md) · [示例](examples/) · [OpenSpec](openspec/specs/)

## 项目价值

这个项目把三种力计算策略统一到同一套仿真/运行时 API 中：

- **Direct N²**：用于精确的成对参考结果
- **Barnes-Hut**：用于可扩展的长程近似
- **Spatial Hash**：用于高效的短程作用力计算
- **Velocity Verlet**：用于稳定的辛积分
- **CUDA/OpenGL 互操作**：用于零拷贝可视化

项目的目标不只是“把粒子渲染得更快”，而是让算法对比、仿真结构和工程质量都保持可理解、可测试、可维护。

## 技术亮点

| 领域 | 提供内容 |
|------|----------|
| 计算 | CUDA 力计算与积分内核 |
| 算法 | Direct N²、Barnes-Hut、Spatial Hash |
| 渲染 | OpenGL 渲染器与 CUDA/OpenGL 互操作 |
| 架构 | `ParticleSystem` 外观 + `ForceCalculator` 策略 |
| 质量 | GoogleTest + RapidCheck、OpenSpec 驱动流程 |

## 算法选型

| 算法 | 复杂度 | 适用场景 |
|------|--------|----------|
| Direct N² | O(N²) | 小规模系统、基准校验 |
| Barnes-Hut | O(N log N) | 大规模引力系统 |
| Spatial Hash | O(N) | 短程作用力工作负载 |

## 快速开始

### 环境要求

- 支持 CUDA 的 NVIDIA GPU
- CUDA Toolkit 11+
- CMake 3.18+
- OpenGL、GLFW、GLEW、GLM

### 构建

```bash
./scripts/build.sh
```

手动构建路径：

```bash
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j"$(nproc)"
```

### 运行

```bash
./build/nbody_sim
./build/nbody_sim 100000
```

### 测试

```bash
./scripts/test.sh
```

## 项目结构

| 路径 | 作用 |
|------|------|
| `include/nbody/` | 公共头文件 |
| `src/` | 核心逻辑、CUDA、渲染、工具代码 |
| `tests/` | 单元测试与属性测试 |
| `examples/` | 示例程序与使用模式 |
| `docs/` | 仓库内规范文档入口 |
| `site/` | GitHub Pages 展示站点 |
| `openspec/specs/` | 当前活跃规格 |
| `openspec/changes/` | 活跃提案与实施任务 |

## 规范文档入口

- [快速开始](docs/setup/getting-started.md)
- [架构说明](docs/architecture/architecture.md)
- [算法说明](docs/architecture/algorithms.md)
- [API 参考](docs/architecture/api.md)
- [性能说明](docs/architecture/performance.md)
- [贡献指南](CONTRIBUTING.md)

## OpenSpec 工作流

本仓库以 OpenSpec 为治理核心。

1. 先阅读 [`openspec/specs/`](openspec/specs/) 中的相关规格。
2. 任何会改变行为或工作流的修改，都应先在 [`openspec/changes/`](openspec/changes/) 中创建或更新变更。
3. 实现必须来自变更任务清单。
4. 重大结构或治理性重构在完成前应使用 `/review` 做显式审查。

当前能力规格：

- [simulation-core](openspec/specs/simulation-core.md)
- [force-computation](openspec/specs/force-computation.md)
- [visualization](openspec/specs/visualization.md)
- [simulation-control](openspec/specs/simulation-control.md)
- [quality-attributes](openspec/specs/quality-attributes.md)
- [repository-governance](openspec/specs/repository-governance.md)

## 示例

- [`example_basic.cpp`](examples/example_basic.cpp)
- [`example_force_methods.cpp`](examples/example_force_methods.cpp)
- [`example_custom_distribution.cpp`](examples/example_custom_distribution.cpp)
- [`example_energy_conservation.cpp`](examples/example_energy_conservation.cpp)

## 开发提示

- 规范构建路径：CMake + `scripts/build.sh`
- 规范 LSP 基线：`clangd` + `compile_commands.json`
- 规范 AI 协作入口：[AGENTS.md](AGENTS.md)、[CLAUDE.md](CLAUDE.md)、[.github/copilot-instructions.md](.github/copilot-instructions.md)

## 许可证

[MIT](LICENSE)
