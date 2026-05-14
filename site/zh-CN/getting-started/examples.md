# 示例

探索 `examples/` 目录中的示例程序。

## 基础示例

**文件**: `examples/example_basic.cpp`

```cpp
#include "nbody/particle_system.hpp"

using namespace nbody;

int main() {
    SimulationConfig config;
    config.particle_count = 50'000;
    config.force_method = ForceMethod::BARNES_HUT;

    ParticleSystem system;
    system.initialize(config);

    for (int i = 0; i < 1000; ++i) {
        system.update(0.001f);
    }

    return 0;
}
```

## 力计算方法对比

**文件**: `examples/example_force_methods.cpp`

在相同初始条件下比较三种算法：

```cpp
#include "nbody/particle_system.hpp"
#include <iostream>
#include <chrono>

using namespace nbody;
using namespace std::chrono;

void benchmark(ForceMethod method, const std::string& name) {
    SimulationConfig config;
    config.particle_count = 100'000;
    config.force_method = method;

    ParticleSystem system;
    system.initialize(config);

    auto start = high_resolution_clock::now();

    for (int i = 0; i < 100; ++i) {
        system.update(0.001f);
    }

    auto end = high_resolution_clock::now();
    auto ms = duration_cast<milliseconds>(end - start).count();

    std::cout << name << ": " << ms << " ms" << std::endl;
}

int main() {
    benchmark(ForceMethod::DIRECT, "直接 N²");
    benchmark(ForceMethod::BARNES_HUT, "Barnes-Hut");
    benchmark(ForceMethod::SPATIAL_HASH, "空间哈希");

    return 0;
}
```

## 自定义分布

**文件**: `examples/example_custom_distribution.cpp`

创建具有自定义初始位置和速度的粒子：

```cpp
#include "nbody/particle_system.hpp"
#include "nbody/particle_data.hpp"
#include <cmath>

using namespace nbody;

int main() {
    // 直接创建粒子数据
    ParticleData data;
    data.resize(1000);

    // 在环形中初始化粒子
    for (size_t i = 0; i < 1000; ++i) {
        float angle = 2.0f * M_PI * i / 1000.0f;
        float radius = 10.0f;

        data.position_x[i] = radius * std::cos(angle);
        data.position_y[i] = radius * std::sin(angle);
        data.position_z[i] = 0.0f;

        // 圆轨道速度
        float v = std::sqrt(1.0f / radius);
        data.velocity_x[i] = -v * std::sin(angle);
        data.velocity_y[i] = v * std::cos(angle);
        data.velocity_z[i] = 0.0f;

        data.mass[i] = 1.0f;
    }

    // 从数据创建系统
    ParticleSystem system;
    system.initialize(data, ForceMethod::BARNES_HUT);

    // 运行模拟
    for (int i = 0; i < 10'000; ++i) {
        system.update(0.001f);
    }

    return 0;
}
```

## 能量守恒

**文件**: `examples/example_energy_conservation.cpp`

监控能量随时间的守恒情况：

```cpp
#include "nbody/particle_system.hpp"
#include <iostream>
#include <fstream>

using namespace nbody;

int main() {
    SimulationConfig config;
    config.particle_count = 10'000;
    config.force_method = ForceMethod::BARNES_HUT;

    ParticleSystem system;
    system.initialize(config);

    double initial_energy = system.getTotalEnergy();
    std::ofstream out("energy.csv");
    out << "step,time,kinetic,potential,total,error\n";

    for (int step = 0; step < 10'000; ++step) {
        system.update(0.001f);

        if (step % 100 == 0) {
            double ke = system.getKineticEnergy();
            double pe = system.getPotentialEnergy();
            double total = ke + pe;
            double error = std::abs((total - initial_energy) / initial_energy);

            out << step << ","
                << system.getTime() << ","
                << ke << ","
                << pe << ","
                << total << ","
                << error << "\n";
        }
    }

    std::cout << "能量数据已写入 energy.csv" << std::endl;
    return 0;
}
```

## 运行示例

```bash
# 构建示例
cmake --build build --target examples

# 运行特定示例
./build/example_basic
./build/example_force_methods
./build/example_energy_conservation
```

## 下一步

- [API 参考](/zh-CN/api-reference/particle-system) - 完整 API 文档
- [算法](/zh-CN/user-guide/algorithms) - 理解算法原理
