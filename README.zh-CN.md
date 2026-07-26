# n-body

[![Build](https://github.com/AICL-Lab/n-body/actions/workflows/ci.yml/badge.svg)](https://github.com/AICL-Lab/n-body/actions/workflows/ci.yml)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

**百万粒子 GPU 物理引擎** — CUDA 加速的 N 体模拟，实时 OpenGL 可视化，三种力计算算法。

[GitHub Pages](https://aicl-lab.github.io/n-body/) · [示例](examples/)

## 算法

| 算法 | 复杂度 | 适用场景 |
|------|--------|----------|
| Direct N² | O(N²) | 小规模系统、基准校验 |
| Barnes-Hut | O(N log N) | 大规模引力系统 |
| Spatial Hash | O(N) | 短程作用力 |

## 性能

| 粒子数 | Direct N² | Barnes-Hut | Spatial Hash |
|--------|-----------|------------|--------------|
| 1万 | 60 FPS | 120 FPS | 120 FPS |
| 10万 | 10 FPS | 60 FPS | 90 FPS |
| 100万 | 1 FPS | 25 FPS | 60 FPS |

*基准测试环境：NVIDIA RTX 3080*

## 快速开始

环境要求：NVIDIA GPU、CUDA Toolkit 11+、CMake 3.18+、OpenGL/GLFW/GLEW/GLM。

```bash
./scripts/build.sh
./build/nbody_sim 100000
```

测试和基准：

```bash
./scripts/test.sh
./scripts/benchmark.sh
```

## 项目结构

| 路径 | 作用 |
|------|------|
| `include/nbody/` | 公共头文件 |
| `src/` | 核心逻辑、CUDA、渲染、工具 |
| `tests/` | 单元测试与属性测试 |
| `examples/` | 示例程序 |
| `site/` | GitHub Pages 展示站点 |

## 开发

- 构建：CMake + `scripts/build.sh`
- LSP：`clangd` + `compile_commands.json`
- AI 协作指引：[AGENTS.md](AGENTS.md)

## 许可证

[MIT](LICENSE)
