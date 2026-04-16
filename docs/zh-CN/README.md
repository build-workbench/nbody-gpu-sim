# 中文文档

N-Body 粒子模拟系统的中文文档。

## 📚 文档结构

| 目录 | 用途 |
|------|------|
| [`setup/`](./setup/) | 环境搭建、构建说明、故障排除 |
| [`architecture/`](./architecture/) | 系统设计、算法、API 参考、性能 |

## 🚀 快速导航

### 入门指南
- [快速入门指南](./setup/getting-started.md) — 安装、构建和首次运行

### 架构与设计
- [架构概览](./architecture/architecture.md) — 系统设计、组件和数据流
- [算法详解](./architecture/algorithms.md) — 力计算算法详解
- [API 参考](./architecture/api.md) — 完整的 API 文档
- [性能指南](./architecture/performance.md) — 优化策略和性能分析

### 规范文档（唯一事实来源）
- [产品规范](../../specs/product/n-body-simulation.md) — 需求和验收标准
- [架构 RFC](../../specs/rfc/0001-core-architecture.md) — 技术设计决策

## 🌐 切换语言

| 语言 | 文档链接 |
|------|----------|
| 🇺🇸 English | [English Documentation](../) |
| 🇨🇳 简体中文 | [当前页面](./) |

## 📊 系统能力

| 特性 | 能力 |
|------|------|
| 最大粒子数 | 1000 万+ |
| GPU 加速 | CUDA 11.0+ |
| 实时渲染 | OpenGL 3.3+ |
| 算法 | 3 种（直接 N²、Barnes-Hut、Spatial Hash） |
| 积分器 | Velocity Verlet（辛积分器） |

## 🔗 外部资源

- [主 README](../../README.md) | [中文版 README](../../README.zh-CN.md)
- [更新日志](../../CHANGELOG.md)
- [示例代码](../../examples/)
- [贡献指南](../../CONTRIBUTING.md)
