---
layout: default
title: 教程
parent: 文档
nav_order: 3
has_children: true
---

# 教程和示例

本目录包含 N-Body 粒子仿真系统的使用教程和代码示例。

## 📚 教程列表

| 教程 | 说明 |
|------|------|
| 基础用法 | 最小化仿真设置和运行 |
| 算法对比 | 不同力计算算法的性能比较 |
| 自定义分布 | 自定义粒子初始条件 |
| 能量监控 | 监控仿真过程中的能量守恒 |

## 🔗 相关资源

- [快速入门](../setup/getting-started.md) — 安装和首次运行
- [架构概览](../architecture/architecture.md) — 系统设计
- [示例代码](https://github.com/LessUp/n-body/tree/main/examples) — 完整代码示例

## 📖 代码示例

### 基础仿真

```cpp
#include "nbody/particle_system.hpp"

using namespace nbody;

int main() {
    SimulationConfig config;
    config.particle_count = 100000;
    config.force_method = ForceMethod::BARNES_HUT;
    config.dt = 0.001f;
    
    ParticleSystem system;
    system.initialize(config);
    
    for (int i = 0; i < 1000; ++i) {
        system.update(system.getTimeStep());
    }
    
    return 0;
}
```

### 能量监控

```cpp
float ke = system.computeKineticEnergy();
float pe = system.computePotentialEnergy();
float total = system.computeTotalEnergy();
std::cout << "Energy: KE=" << ke << " PE=" << pe << " Total=" << total << std::endl;
```

### 保存和加载状态

```cpp
// 保存
system.saveState("checkpoint.nbody");

// 加载
system.loadState("checkpoint.nbody");
```
