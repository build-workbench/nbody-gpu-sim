# Contributing to N-Body Particle Simulation

感谢你对本项目的关注！欢迎通过 Issue 和 Pull Request 参与贡献。

[English](#english) | [中文](#中文)

---

## 中文

### 开发流程

本项目严格遵循**规范驱动开发（Spec-Driven Development）**范式。所有代码实现必须以 `/specs` 目录下的规范文档为唯一事实来源。

1. **阅读规范**：在编写任何代码之前，首先阅读 `/specs` 目录下相关的产品文档、RFC 和 API 定义
2. **更新规范**：如果是新功能或需要改变现有接口/数据库结构，**必须先修改或创建相应的 Spec 文档**
3. **等待确认**：等待 Spec 修改被确认后，再进行代码实现
4. **编写代码**：严格按照 Spec 中的定义编写代码
5. **编写测试**：根据 Spec 中的验收标准编写测试用例
6. 创建特性分支：`git checkout -b feature/your-feature`
7. 确保测试通过
8. 提交更改：`git commit -m "feat: add your feature"`
9. 推送分支：`git push origin feature/your-feature`
10. 创建 Pull Request

### 规范文档结构

```
specs/
├── product/            # 产品功能定义与验收标准
│   └── n-body-simulation.md
└── rfc/                # 技术设计与架构方案
    └── 0001-core-architecture.md
```

详细说明请参阅 [AGENTS.md](AGENTS.md)。

### 构建与测试

```bash
# 克隆仓库
git clone https://github.com/LessUp/n-body.git
cd n-body

# 创建构建目录
mkdir build && cd build

# 配置项目 (Release 模式)
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译
cmake --build . -j$(nproc)

# 运行测试
ctest --output-on-failure
# 或直接运行测试可执行文件
./nbody_tests

# 运行模拟
./nbody_sim 10000
```

### 系统要求

| 组件 | 最低要求 | 推荐配置 |
|------|----------|----------|
| GPU | NVIDIA CC 7.0+ | NVIDIA CC 8.0+ |
| CUDA | 11.0 | 12.0+ |
| CMake | 3.18 | 3.25+ |
| C++ 编译器 | GCC 9 / MSVC 2019 | GCC 11+ / Clang 14+ |

### 代码规范

- C++/CUDA 代码遵循项目现有风格
- 使用 `.editorconfig` 中定义的缩进和格式规则
- 使用 `clang-format` 格式化代码（配置见 `.clang-format`）
- 新增功能请附带单元测试
- 确保所有现有测试通过

### 代码格式化

```bash
# 检查格式
clang-format --dry-run --Werror src/**/*.cpp src/**/*.cu include/**/*.hpp

# 自动格式化
clang-format -i src/**/*.cpp src/**/*.cu include/**/*.hpp
```

### 提交信息格式

推荐使用 [Conventional Commits](https://www.conventionalcommits.org/)：

| 前缀 | 说明 |
|------|------|
| `feat:` | 新功能 |
| `fix:` | 修复 Bug |
| `docs:` | 文档更新 |
| `perf:` | 性能优化 |
| `refactor:` | 代码重构 |
| `test:` | 测试相关 |
| `chore:` | 构建或辅助工具变更 |

**示例**:
```
feat: add multi-GPU support for Barnes-Hut algorithm

- Implement domain decomposition across GPUs
- Add P2P memory transfer for boundary particles
- Update performance benchmarks

Closes #123
```

### Pull Request 指南

1. **标题**: 简洁描述更改内容
2. **描述**: 详细说明更改原因和实现方式
3. **测试**: 说明如何测试这些更改
4. **文档**: 如有必要，更新相关文档
5. **检查**: 确保通过所有 CI 检查

### 问题报告

提交 Issue 时请包含：

- 操作系统和 CUDA 版本
- GPU 型号和驱动版本
- 重现步骤
- 期望行为和实际行为
- 相关日志或截图

---

## English

### Development Workflow

This project strictly follows the **Spec-Driven Development (SDD)** paradigm. All code implementations must use the `/specs` directory as the Single Source of Truth.

1. **Review Specs**: Before writing any code, first read the relevant product spec, RFC, and API definition in `/specs/`
2. **Update Specs**: If this is a new feature or requires changes to existing interfaces/database structures, **you must first modify or create the relevant spec documents**
3. **Wait for Confirmation**: Proceed to code implementation only after spec changes are confirmed
4. **Write Code**: Strictly follow the definitions in the specs
5. **Write Tests**: Write test cases based on the acceptance criteria in the specs
6. Fork this repository
7. Create a feature branch: `git checkout -b feature/your-feature`
8. Make changes and ensure tests pass
9. Commit your changes: `git commit -m "feat: add your feature"`
10. Push to your branch: `git push origin feature/your-feature`
11. Create a Pull Request

### Spec Directory Structure

```
specs/
├── product/            # Product feature definitions and acceptance criteria
│   └── n-body-simulation.md
└── rfc/                # Technical design and architecture decisions
    └── 0001-core-architecture.md
```

For detailed instructions, see [AGENTS.md](AGENTS.md).

### Build and Test

```bash
# Clone the repository
git clone https://github.com/LessUp/n-body.git
cd n-body

# Create build directory
mkdir build && cd build

# Configure project (Release mode)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build . -j$(nproc)

# Run tests
ctest --output-on-failure
# Or run the test executable directly
./nbody_tests

# Run simulation
./nbody_sim 10000
```

### System Requirements

| Component | Minimum | Recommended |
|-----------|---------|-------------|
| GPU | NVIDIA CC 7.0+ | NVIDIA CC 8.0+ |
| CUDA | 11.0 | 12.0+ |
| CMake | 3.18 | 3.25+ |
| C++ Compiler | GCC 9 / MSVC 2019 | GCC 11+ / Clang 14+ |

### Code Style

- Follow existing project style for C++/CUDA code
- Use indentation and formatting rules defined in `.editorconfig`
- Use `clang-format` to format code (see `.clang-format`)
- Add unit tests for new features
- Ensure all existing tests pass

### Code Formatting

```bash
# Check formatting
clang-format --dry-run --Werror src/**/*.cpp src/**/*.cu include/**/*.hpp

# Auto-format
clang-format -i src/**/*.cpp src/**/*.cu include/**/*.hpp
```

### Commit Message Format

We recommend using [Conventional Commits](https://www.conventionalcommits.org/):

| Prefix | Description |
|--------|-------------|
| `feat:` | New feature |
| `fix:` | Bug fix |
| `docs:` | Documentation update |
| `perf:` | Performance improvement |
| `refactor:` | Code refactoring |
| `test:` | Testing related |
| `chore:` | Build or auxiliary tool changes |

**Example**:
```
feat: add multi-GPU support for Barnes-Hut algorithm

- Implement domain decomposition across GPUs
- Add P2P memory transfer for boundary particles
- Update performance benchmarks

Closes #123
```

### Pull Request Guidelines

1. **Title**: Concise description of changes
2. **Description**: Detailed explanation of why and how
3. **Testing**: Explain how to test the changes
4. **Documentation**: Update relevant docs if necessary
5. **Checks**: Ensure all CI checks pass

### Bug Reports

When submitting an Issue, please include:

- Operating system and CUDA version
- GPU model and driver version
- Steps to reproduce
- Expected and actual behavior
- Relevant logs or screenshots

---

## License

By contributing to this project, you agree that your contributions will be licensed under the [MIT License](LICENSE).
