# 示例

本目录包含针对 nbody-gpu-sim 主要工作流的聚焦示例程序。

## 可用程序

| 程序 | 用途 |
|------|------|
| [`example_basic.cpp`](example_basic.cpp) | 最小端到端模拟 |
| [`example_force_methods.cpp`](example_force_methods.cpp) | 对比力计算算法 |
| [`example_custom_distribution.cpp`](example_custom_distribution.cpp) | 构建自定义粒子布局 |
| [`example_energy_conservation.cpp`](example_energy_conservation.cpp) | 检查积分器稳定性 |

## 构建

示例默认通过标准 CMake 路径构建：

```bash
./scripts/build.sh
```

或手动：

```bash
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DNBODY_BUILD_EXAMPLES=ON
cmake --build . -j"$(nproc)"
```

生成的可执行文件位于 `build/`：

```bash
./build/example_basic
./build/example_force_methods
./build/example_custom_distribution
./build/example_energy_conservation
```

## 各示例覆盖内容

### `example_basic.cpp`

- 初始化 `ParticleSystem`
- 运行基础模拟循环
- 保存与加载状态
- 查看总能量

### `example_force_methods.cpp`

- 对比 Direct N²、Barnes-Hut 和 Spatial Hash
- 运行时切换算法
- 查看性能权衡

### `example_custom_distribution.cpp`

- 创建自定义初始粒子分布
- 直接写入粒子数据
- 试验非默认配置

### `example_energy_conservation.cpp`

- 跟踪能量随时间漂移
- 查看时间步敏感度
- 评估积分器稳定性

## 相关

- [快速上手](../docs/setup/getting-started.md)
