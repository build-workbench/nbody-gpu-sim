# AGENTS.zh-CN.md — n-body 仓库工作流

## 目标

维护并稳定一个基于 CUDA/OpenGL 的 N 体模拟项目。当前阶段以“高质量收尾”为主，优先保证清晰度、正确性和低维护成本，而不是继续扩展功能面。

## 规范仓库入口

| 路径 | 角色 |
|------|------|
| `openspec/specs/` | 当前活跃需求与仓库规则 |
| `openspec/changes/` | 活跃提案、设计、变更规格与任务清单 |
| `openspec/changes/archive/` | 已完成变更 |
| `specs-archived/` | 仅供历史参考，绝不能作为当前事实来源 |
| `docs/` | 仓库内读者与贡献者文档 |
| `site/` | GitHub Pages 展示表面 |
| `.github/` | CI、Pages、Issue/PR 模板与 Copilot 指令 |

## 能力清单

- `simulation-core.md` — 粒子数据、力计算、Barnes-Hut
- `force-computation.md` — 空间哈希与积分
- `visualization.md` — CUDA/OpenGL 互操作与渲染
- `simulation-control.md` — 运行时控制、序列化、校验
- `quality-attributes.md` — 性能、可靠性、可维护性、验证质量
- `repository-governance.md` — OpenSpec 工作流、文档策略、自动化质量、GitHub 呈现、收尾规则

## 必须遵循的工作流

1. 修改代码、文档、工作流或项目行为之前，先阅读 `openspec/specs/` 中的相关文件。
2. 如果工作会改变行为、流程、文档策略、自动化或仓库结构，必须先在 `openspec/changes/` 中创建或更新 OpenSpec 变更。
3. 保持 proposal、design、specs、tasks 一致，不要只凭模糊意图直接实现。
4. 按“小而完整”的批次实施，并在完成后立即更新任务复选框。
5. 对变更涉及的表面运行现有最高价值检查。
6. 在完成重大结构、工作流或文档重构前，使用 `/review` 做显式审查。

## 项目特定规则

- 将 `openspec/` 视为唯一活跃规格系统。历史规格在 `specs-archived/` 中仅供参考。
- 在当前清理阶段，把仓库里现有未提交改动视为工作基线，除非用户明确要求其他处理方式。
- 遇到重复文档或重复站点内容时，优先删除、合并或重定向，而不是继续并存。
- 不要添加通用化、无项目针对性的工程文档或 AI 提示模板。
- 面向读者的文档只谈产品与贡献方式，不对外说明项目的归档/低维护意图。
- 修改核心规格或主要入门文档时，必须同步更新其必需的双语版本。

## 工具链与自动化规则

- 优先使用规范的 CMake 构建路径和 `scripts/` 中的脚本。
- 依赖版本与 GitHub Actions 版本应固定或明确约束。
- 工作流应追求高信号，避免宽触发和仪式化检查。
- 默认 LSP 基线是使用规范构建生成 `compile_commands.json` 的 `clangd`。
- MCP / 插件应保持精简，优先使用原生 OpenSpec、GitHub 和编辑器能力。
- 非高度并行任务优先选择长时 `autopilot`，不要常规依赖 `/fleet`。

## 文档规则

- `README.md` / `README.zh-CN.md` 负责项目介绍、快速开始和规范链接。
- `CONTRIBUTING.md` 负责贡献流程与质量门禁。
- `CLAUDE.md`、`AGENTS.md` 和 `.github/copilot-instructions.md` 必须描述同一套运行模型。
- 若保留 `QWEN.md`，它只能是兼容性跳转文件，不能再成为独立说明来源。
- GitHub Pages 应补充仓库并展示项目，而不是镜像全部 Markdown。

## 工作方式

- 改动要精准，但不能只修表面。
- 可复用的现有模式就复用；结构本身有问题时就重构。
- 在触及某个表面时，顺手修复与之紧耦合的漂移和缺陷。
- 每次提交的结果都应让仓库比之前更容易理解。
