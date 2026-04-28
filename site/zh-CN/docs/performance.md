---
layout: docs-zh
title: 性能指南
lang: zh-CN
description: N-Body 粒子仿真系统的优化策略和性能调优
---

# 性能指南

N-Body 粒子仿真系统的优化策略和性能调优。

---

## 📑 目录

1. [基准测试](#基准测试)
2. [GPU 优化](#gpu-优化)
3. [算法调优](#算法调优)
4. [分析工具](#分析工具)

---

## 基准测试

### 性能目标

| 粒子数 | 目标 FPS | 推荐算法 |
|--------|----------|----------|
| 10,000 | 60+ | Direct N² |
| 100,000 | 60+ | Barnes-Hut |
| 1,000,000 | 30+ | Barnes-Hut / Spatial Hash |

### 测试性能

#### 帧率 (FPS) - RTX 3080

| 粒子数 | Direct N² | Barnes-Hut (θ=0.5) | Spatial Hash |
|--------|-----------|-------------------|--------------|
| 10,000 | 60+ | 60+ | 60+ |
| 100,000 | ~8 | 60+ | 60+ |
| 1,000,000 | N/A | ~25 | 60+ |

#### 内存使用

| 粒子数 | 粒子数据 | Barnes-Hut | Spatial Hash | 总计 |
|--------|----------|------------|--------------|------|
| 10万 | ~5 MB | ~10 MB | ~2 MB | ~17 MB |
| 100万 | ~50 MB | ~100 MB | ~20 MB | ~170 MB |

---

## GPU 优化

### 1. 线程块大小

| 架构 | 系列 | 最优块大小 |
|------|------|-----------|
| Ada Lovelace | RTX 40xx | 256 或 512 |
| Ampere | RTX 30xx | 256 |
| Turing | RTX 20xx | 256 |

### 2. 共享内存使用

对于 Direct N² 核函数，共享内存减少全局内存流量 ~50%。

### 3. 内存合并

确保合并内存访问：

```cpp
// 好：合并访问
int i = blockIdx.x * blockDim.x + threadIdx.x;
float x = pos_x[i];
```

---

## 算法调优

### Barnes-Hut Theta 参数

| θ | 精度 | 速度 |
|---|------|------|
| 0.3 | 高 | 慢 |
| 0.5 | 中 | 中 |
| 0.8 | 低 | 快 |

### Spatial Hash 单元格大小

最优单元格大小等于截断半径：

```cpp
float optimal_cell_size = cutoff_radius;
```

---

## 分析工具

### NVIDIA Nsight Systems

系统级性能分析：

```bash
nsys profile --stats=true -o report ./nbody_sim 100000
```

### NVIDIA Nsight Compute

核函数级详细分析：

```bash
ncu --set full -o report.ncu-rep ./nbody_sim 100000
```

---

## 优化清单

- [ ] Release 构建 (`-O3 -DNDEBUG`)
- [ ] 启用快速数学 (`-use_fast_math`)
- [ ] 正确架构 (`-arch=sm_86`)
- [ ] 最新 GPU 驱动
- [ ] 选择正确算法
- [ ] 优化内存访问模式
