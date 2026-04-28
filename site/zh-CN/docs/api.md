---
layout: docs-zh
title: API 参考
lang: zh-CN
description: N-Body 粒子仿真库的完整 API 参考
---

# API 参考

N-Body 粒子仿真库的完整 API 参考。

---

## 📑 目录

- [核心类](#核心类)
- [数据结构](#数据结构)
- [枚举类型](#枚举类型)

---

## 核心类

### ParticleSystem

管理整个仿真的主协调器类。

```cpp
namespace nbody {

class ParticleSystem {
public:
    // 生命周期
    ParticleSystem();
    ~ParticleSystem();

    // 初始化
    void initialize(const SimulationConfig& config);

    // 仿真控制
    void update(float dt);
    void pause();
    void resume();
    void reset();

    // 参数配置
    void setForceMethod(ForceMethod method);
    void setGravitationalConstant(float G);
    void setTimeStep(float dt);

    // 状态管理
    void saveState(const std::string& filename) const;
    void loadState(const std::string& filename);

    // 能量计算
    float computeTotalEnergy() const;
};

} // namespace nbody
```

#### 使用示例

```cpp
#include "nbody/particle_system.hpp"

using namespace nbody;

int main() {
    SimulationConfig config;
    config.particle_count = 100000;
    config.force_method = ForceMethod::BARNES_HUT;

    ParticleSystem system;
    system.initialize(config);

    for (int i = 0; i < 1000; ++i) {
        system.update(system.getTimeStep());
    }

    return 0;
}
```

---

### ForceCalculator

力计算算法的抽象基类。

```cpp
class ForceCalculator {
public:
    virtual void computeForces(ParticleData* d_particles) = 0;
    virtual ForceMethod getMethod() const = 0;

    void setGravitationalConstant(float G);
    void setSofteningParameter(float eps);
};
```

---

### Integrator

Velocity Verlet 辛积分器。

```cpp
class Integrator {
public:
    void integrate(ParticleData* d_particles,
                   ForceCalculator* force_calc,
                   float dt);

    float computeTotalEnergy(const ParticleData* d_particles);
};
```

---

## 数据结构

### ParticleData

数组结构（SoA）布局，适合 GPU。

```cpp
struct ParticleData {
    float* pos_x, *pos_y, *pos_z;    // 位置
    float* vel_x, *vel_y, *vel_z;    // 速度
    float* acc_x, *acc_y, *acc_z;    // 加速度
    float* mass;                      // 质量
    size_t count;
};
```

**内存使用：** 每个粒子 52 字节

---

### SimulationConfig

仿真设置配置。

```cpp
struct SimulationConfig {
    size_t particle_count = 10000;
    InitDistribution init_distribution = InitDistribution::SPHERICAL;
    ForceMethod force_method = ForceMethod::DIRECT_N2;
    float dt = 0.001f;
    float G = 1.0f;
    float softening = 0.01f;
};
```

---

## 枚举类型

### ForceMethod

```cpp
enum class ForceMethod {
    DIRECT_N2,      // O(N²) 精确计算
    BARNES_HUT,     // O(N log N) 树近似
    SPATIAL_HASH    // O(N) 基于网格（短程）
};
```

### InitDistribution

```cpp
enum class InitDistribution {
    UNIFORM,        // 均匀盒
    SPHERICAL,      // 均匀球
    DISK            // 带旋转的扁平盘
};
```

---

## 📚 相关文档

- [快速入门](./getting-started/) - 安装和使用指南
- [架构概览](./architecture/) - 系统设计概览
- [算法详解](./algorithms/) - 算法说明
