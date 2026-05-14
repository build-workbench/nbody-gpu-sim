# 测试

N-Body 项目使用 GoogleTest 进行单元测试，RapidCheck 进行基于属性的测试。

## 运行测试

```bash
# 所有测试
./scripts/test.sh

# 或手动运行
cd build
ctest --output-on-failure
```

## 测试分类

### 单元测试

位于 `tests/` 目录：

| 测试文件 | 覆盖范围 |
|----------|----------|
| `test_particle_data.cpp` | ParticleData 结构、内存布局 |
| `test_particle_system.cpp` | ParticleSystem API |
| `test_force_calculator.cpp` | 力计算正确性 |
| `test_integrator.cpp` | Velocity Verlet 积分 |
| `test_barnes_hut.cpp` | 八叉树构建和遍历 |
| `test_spatial_hash.cpp` | 网格构建和邻居搜索 |
| `test_serialization.cpp` | 状态保存/加载 |

### 基于属性的测试

使用 RapidCheck 生成随机输入：

```cpp
// 示例：能量应该守恒
RC_ASSERT(std::abs(energy_after - energy_before) < tolerance);
```

## 编写测试

### 基础测试

```cpp
#include <gtest/gtest.h>
#include "nbody/particle_system.hpp"

TEST(ParticleSystemTest, InitializeSetsCorrectCount) {
    nbody::SimulationConfig config;
    config.particle_count = 1000;

    nbody::ParticleSystem system;
    system.initialize(config);

    EXPECT_EQ(system.getParticleCount(), 1000);
}
```

### CUDA 测试

```cpp
#include <gtest/gtest.h>
#include "nbody/force_calculator.hpp"

TEST(ForceCalculatorTest, DirectForceMatchesReference) {
    // 创建测试粒子
    ParticleData data;
    data.resize(100);
    // ... 初始化 ...

    // 在 GPU 上计算
    ForceCalculator calc(ForceMethod::DIRECT);
    calc.compute(data);

    // 与 CPU 参考值比较
    for (size_t i = 0; i < 100; ++i) {
        EXPECT_NEAR(data.force_x[i], ref_force_x[i], 1e-5f);
    }
}
```

## 测试配置

### 环境变量

| 变量 | 用途 |
|------|------|
| `NBODY_TEST_PARTICLES` | 测试最大粒子数 |
| `NBODY_TEST_TOLERANCE` | 浮点容差 |

### 无头测试

测试默认在无头模式下运行（无需 OpenGL）：

```bash
cmake .. -DNBODY_ENABLE_RENDERING=OFF
```

## 性能测试

```bash
# 运行性能测试
./scripts/benchmark.sh

# 特定性能测试
./build/benchmark --filter=serialization.*
```

## CI 测试

测试在 GitHub Actions 上自动运行：

- 推送到 main/master
- 影响 C++/CUDA 文件的 Pull Request
- 每周定时运行

详见 `.github/workflows/ci.yml`。
