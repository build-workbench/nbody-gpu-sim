# AGENTS.zh-CN.md — n-body 仓库工作流

## 目标

维护一个基于 CUDA/OpenGL 的 N 体模拟 demo。优先保证清晰度、正确性和低维护成本。

## 规范仓库入口

| 路径 | 角色 |
|------|------|
| `docs/` | 仓库内读者与贡献者文档 |
| `site/` | GitHub Pages 展示表面 |
| `.github/` | CI、Pages、Issue/PR 模板与 Copilot 指令 |

## 必须遵循的工作流

1. 修改代码前先阅读 `include/nbody/` 中的相关头文件。
2. 按“小而完整”的批次实施。
3. 对变更涉及的表面运行现有最高价值检查。

## 项目特定规则

- 遇到重复文档或重复站点内容时，优先删除、合并或重定向，而不是继续并存。
- 不要添加通用化、无项目针对性的工程文档或 AI 提示模板。
- 修改主要入门文档时，必须同步更新其必需的双语版本。

## 工具链与自动化规则

- 优先使用规范的 CMake 构建路径和 `scripts/` 中的脚本。
- 依赖版本与 GitHub Actions 版本应固定或明确约束。
- 默认 LSP 基线是使用规范构建生成 `compile_commands.json` 的 `clangd`。

## 文档规则

- `README.md` / `README.zh-CN.md` 负责项目介绍、快速开始和规范链接。
- `CONTRIBUTING.md` 负责贡献流程与质量门禁。
- `CLAUDE.md`、`AGENTS.md` 和 `.github/copilot-instructions.md` 必须描述同一套运行模型。
- GitHub Pages 应补充仓库并展示项目，而不是镜像全部 Markdown。

## 工作方式

- 改动要精准，但不能只修表面。
- 可复用的现有模式就复用；结构本身有问题时就重构。
- 在触及某个表面时，顺手修复与之紧耦合的漂移和缺陷。
- 每次提交的结果都应让仓库比之前更容易理解。
