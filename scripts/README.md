# 脚本

项目的构建与自动化脚本。

## 可用脚本

- `build.sh` - 构建项目（自动检测 CUDA；无 CUDA 时回退到无头模式）
- `test.sh` - 通过 ctest 运行测试
- `benchmark.sh` - 运行基准并输出 JSON 结果
- `format.sh` - 用 clang-format 格式化代码
- `setup-hooks.sh` - 启用仓库 git hooks（.githooks/）
