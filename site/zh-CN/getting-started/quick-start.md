# 快速开始

几分钟内运行你的第一个 N-body 模拟。

## 基本模拟

```bash
# 先构建（见安装指南）
./build/nbody_sim
```

## 命令行选项

```bash
./build/nbody_sim [粒子数量] [选项]
```

| 选项 | 描述 | 默认值 |
|------|------|--------|
| `粒子数量` | 粒子数量 | 10,000 |
| `--algorithm` | 力方法: `direct`, `barnes-hut`, `spatial-hash` | barnes-hut |
| `--distribution` | 初始分布: `spherical`, `disk`, `cube` | spherical |
| `--dt` | 时间步长 | 0.001 |

## 示例

```bash
# 10万粒子，Barnes-Hut
./build/nbody_sim 100000

# 100万粒子，空间哈希 O(N)
./build/nbody_sim 1000000 --algorithm spatial-hash

# 直接 N² 用于小系统（最精确）
./build/nbody_sim 5000 --algorithm direct
```

## 键盘控制

| 按键 | 功能 |
|------|------|
| `空格` | 暂停/继续 |
| `R` | 重置模拟 |
| `1/2/3` | 切换算法 |
| `I` | 切换信息面板 |
| `Esc` | 退出 |

## 使用 C++ API

```cpp
#include "nbody/particle_system.hpp"

using namespace nbody;

int main() {
    SimulationConfig config;
    config.particle_count = 100'000;
    config.force_method = ForceMethod::BARNES_HUT;
    
    ParticleSystem system;
    system.initialize(config);
    
    for (int i = 0; i < 10'000; ++i) {
        system.update(0.001f);
    }
    
    return 0;
}
```

## 下一步

- [示例](/zh-CN/getting-started/examples) - 更多代码示例
- [配置](/zh-CN/user-guide/configuration) - 所有配置选项
