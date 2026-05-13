---
layout: default
title: 算法详解
parent: 文档
nav_order: 4
---

# 算法详解

N-Body 粒子仿真系统中使用的力计算算法和积分方法的详细说明。

---

## 📑 目录

1. [力计算基础](#力计算基础)
2. [Direct N² 算法](#direct-n²-算法)
3. [Barnes-Hut 算法](#barnes-hut-算法)
4. [Spatial Hash 算法](#spatial-hash-算法)
5. [Velocity Verlet 积分](#velocity-verlet-积分)
6. [算法选择指南](#算法选择指南)

---

## 力计算基础

### 牛顿万有引力定律

两个粒子之间的引力：

$$F = G \frac{m_1 m_2}{r^2}$$

其中：
- $G$ = 引力常数
- $m_1, m_2$ = 粒子质量
- $r$ = 粒子间距离

### 软化参数

为防止近距离相遇时的数值发散，引入软化参数 $\epsilon$：

$$F = G \frac{m_1 m_2}{r^2 + \epsilon^2}$$

**好处：**
- 防止力奇异
- 模拟有限大小粒子
- 平滑近距离相互作用

**典型值：** $\epsilon \in [0.01, 0.1]$（取决于系统尺度）

### 加速度计算

粒子 $i$ 受到所有其他粒子的加速度：

$$\mathbf{a}_i = \sum_{j \neq i} G m_j \frac{\mathbf{r}_j - \mathbf{r}_i}{|\mathbf{r}_j - \mathbf{r}_i|^3}$$

---

## Direct N² 算法

### 原理

直接计算所有粒子对之间的力。保证精确但 $O(N^2)$ 复杂度。

### 伪代码

```
for 每个粒子 i:
    a_i = 0
    for 每个粒子 j ≠ i:
        r = pos_j - pos_i
        dist² = |r|² + ε²
        a_i += G * m_j * r / dist^(3/2)
```

### CUDA 优化

#### 共享内存瓦片

```cuda
__global__ void computeForcesDirectKernel(
    const float* pos_x, const float* pos_y, const float* pos_z,
    const float* mass,
    float* acc_x, float* acc_y, float* acc_z,
    int N, float G, float eps2
) {
    extern __shared__ float shared[];
    float* s_pos_x = shared;
    float* s_pos_y = s_pos_x + blockDim.x;
    float* s_pos_z = s_pos_y + blockDim.x;
    float* s_mass = s_pos_z + blockDim.x;
    
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    float ax = 0, ay = 0, az = 0;
    
    float my_x = pos_x[i];
    float my_y = pos_y[i];
    float my_z = pos_z[i];
    
    // 按瓦片处理
    for (int tile = 0; tile < gridDim.x; ++tile) {
        int j = tile * blockDim.x + threadIdx.x;
        
        // 协作加载到共享内存
        s_pos_x[threadIdx.x] = pos_x[j];
        s_pos_y[threadIdx.x] = pos_y[j];
        s_pos_z[threadIdx.x] = pos_z[j];
        s_mass[threadIdx.x] = mass[j];
        __syncthreads();
        
        // 计算与当前瓦片的相互作用
        for (int k = 0; k < blockDim.x; ++k) {
            float dx = s_pos_x[k] - my_x;
            float dy = s_pos_y[k] - my_y;
            float dz = s_pos_z[k] - my_z;
            float dist2 = dx*dx + dy*dy + dz*dz + eps2;
            
            float inv_dist = rsqrtf(dist2);
            float inv_dist3 = inv_dist * inv_dist * inv_dist;
            
            float f = G * s_mass[k] * inv_dist3;
            ax += f * dx;
            ay += f * dy;
            az += f * dz;
        }
        __syncthreads();
    }
    
    acc_x[i] = ax;
    acc_y[i] = ay;
    acc_z[i] = az;
}
```

#### 应用的优化

| 技术 | 效果 |
|------|------|
| 共享内存 | 全局内存带宽减少 ~50% |
| 合并访问 | 最大化内存吞吐量 |
| `rsqrtf()` | 快速逆平方根 |
| `__restrict__` | 启用编译器优化 |

### 性能特点

| 方面 | 值 |
|------|-----|
| 复杂度 | $O(N^2)$ |
| 内存 | $O(N)$ |
| 精度 | 精确 |
| 适用 | $N < 50{,}000$ |

---

## Barnes-Hut 算法

### 原理

使用八叉树将远距离粒子簇近似为单个质点。复杂度降至 $O(N \log N)$。

### 开角判据

是否接受节点近似：

$$\theta = \frac{s}{d}$$

其中：
- $s$ = 节点边长
- $d$ = 粒子到节点质心的距离

如果 $\theta < \theta_{threshold}$，使用近似。否则，递归到子节点。

| θ | 精度 | 速度 | 适用场景 |
|---|------|------|----------|
| 0.0 | 精确 | 最慢 | 基准 |
| 0.3 | 高 | 慢 | 科学计算 |
| 0.5 | 中 | 中 | 通用 |
| 0.8 | 低 | 快 | 预览 |
| 1.0 | 很低 | 最快 | 草稿 |

### 算法步骤

1. **计算边界盒** - 所有粒子的
2. **构建八叉树** - 递归细分空间
3. **计算质心** - 所有节点的
4. **遍历树** - 对每个粒子使用开角判据

### 树遍历 (CUDA)

```cuda
__device__ float3 computeBarnesHutForce(
    int particle_idx,
    const OctreeNode* nodes,
    float theta,
    float G, float eps2
) {
    float3 force = make_float3(0, 0, 0);
    int stack[64];  // 显式栈用于遍历
    int stack_ptr = 0;
    
    stack[stack_ptr++] = 0;  // 从根开始
    
    while (stack_ptr > 0) {
        int node_idx = stack[--stack_ptr];
        const OctreeNode& node = nodes[node_idx];
        
        float3 dp = make_float3(
            node.center_of_mass.x - pos_x[particle_idx],
            node.center_of_mass.y - pos_y[particle_idx],
            node.center_of_mass.z - pos_z[particle_idx]
        );
        
        float dist2 = dp.x*dp.x + dp.y*dp.y + dp.z*dp.z + eps2;
        float dist = sqrtf(dist2);
        
        // 开角测试
        if (node.is_leaf || (node.size / dist < theta)) {
            // 接受近似
            float inv_dist3 = 1.0f / (dist2 * dist);
            float f = G * node.total_mass * inv_dist3;
            force.x += f * dp.x;
            force.y += f * dp.y;
            force.z += f * dp.z;
        } else {
            // 递归到子节点
            for (int i = 0; i < 8; ++i) {
                if (node.children[i] >= 0) {
                    stack[stack_ptr++] = node.children[i];
                }
            }
        }
    }
    
    return force;
}
```

### 性能特点

| 方面 | 值 |
|------|-----|
| 复杂度 | $O(N \log N)$ |
| 内存 | $O(N)$（树开销 ~2× 粒子） |
| 精度 | 可通过 θ 配置 |
| 适用 | $N > 50{,}000$，长程力 |

---

## Spatial Hash 算法

### 原理

将空间划分为均匀网格单元。粒子只与相邻单元中的其他粒子相互作用。短程力 $O(N)$。

### 网格构建

```
cell_idx = floor(position / cell_size)
hash = cell_idx.x + cell_idx.y * grid_width + cell_idx.z * grid_width * grid_height
```

### 邻居搜索

对于截断半径 $r_{cut}$ 和单元格大小 $s$：

$$\text{neighbor\_cells} = \left\lceil \frac{r_{cut}}{s} \right\rceil$$

当 $s \approx r_{cut}$ 时，搜索 $3^3 = 27$ 个相邻单元格。

### CUDA 实现

```cuda
__global__ void spatialHashForceKernel(
    const int* cell_start, const int* cell_end,
    const int* sorted_indices,
    const float* pos_x, const float* pos_y, const float* pos_z,
    const float* mass,
    float* acc_x, float* acc_y, float* acc_z,
    int N, int3 grid_dims, float cell_size,
    float cutoff, float G, float eps2
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    
    float ax = 0, ay = 0, az = 0;
    float my_x = pos_x[i];
    float my_y = pos_y[i];
    float my_z = pos_z[i];
    
    int3 my_cell = computeCell(my_x, my_y, my_z, cell_size);
    float cutoff2 = cutoff * cutoff;
    
    // 遍历 27 个相邻单元格
    for (int dz = -1; dz <= 1; ++dz) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                int3 neighbor_cell = my_cell + make_int3(dx, dy, dz);
                int hash = hashCell(neighbor_cell, grid_dims);
                
                // 遍历单元格中的粒子
                for (int j = cell_start[hash]; j < cell_end[hash]; ++j) {
                    int idx = sorted_indices[j];
                    if (idx == i) continue;
                    
                    float dx = pos_x[idx] - my_x;
                    float dy = pos_y[idx] - my_y;
                    float dz = pos_z[idx] - my_z;
                    float dist2 = dx*dx + dy*dy + dz*dz;
                    
                    if (dist2 < cutoff2 && dist2 > 0) {
                        dist2 += eps2;
                        float inv_dist = rsqrtf(dist2);
                        float inv_dist3 = inv_dist * inv_dist * inv_dist;
                        float f = G * mass[idx] * inv_dist3;
                        ax += f * dx;
                        ay += f * dy;
                        az += f * dz;
                    }
                }
            }
        }
    }
    
    acc_x[i] = ax;
    acc_y[i] = ay;
    acc_z[i] = az;
}
```

### 性能特点

| 方面 | 值 |
|------|-----|
| 复杂度 | $O(N)$（均匀分布） |
| 内存 | $O(N + C)$，C = 单元格数 |
| 精度 | 截断范围内精确 |
| 适用 | 短程力，分子动力学 |

---

## Velocity Verlet 积分

### 原理

辛积分器，长期保持能量守恒。二阶精度，每步单次力计算。

### 算法

1. **保存旧加速度：**
   $$\mathbf{a}_{old} = \mathbf{a}(t)$$

2. **更新位置：**
   $$\mathbf{x}(t+dt) = \mathbf{x}(t) + \mathbf{v}(t) \cdot dt + \frac{1}{2}\mathbf{a}(t) \cdot dt^2$$

3. **计算新力：**
   $$\mathbf{a}(t+dt) = \frac{\mathbf{F}(\mathbf{x}(t+dt))}{m}$$

4. **更新速度：**
   $$\mathbf{v}(t+dt) = \mathbf{v}(t) + \frac{1}{2}(\mathbf{a}_{old} + \mathbf{a}(t+dt)) \cdot dt$$

### 为什么选择 Velocity Verlet？

| 积分器 | 阶数 | 能量守恒 | 力计算次数 |
|--------|------|----------|------------|
| Euler | 1 | 差 | 1 |
| Leapfrog | 2 | 好 | 1 |
| **Velocity Verlet** | **2** | **优秀** | **1** |
| RK4 | 4 | 一般 | 4 |

### 辛性质

辛积分器保持相空间体积，确保：
- 长期能量误差有界（振荡）
- 适合长时间仿真
- 保持定性动力学
- 能量无系统漂移

### CUDA 实现

```cuda
// 位置更新核函数
__global__ void updatePositionsKernel(
    float* pos_x, float* pos_y, float* pos_z,
    const float* vel_x, const float* vel_y, const float* vel_z,
    const float* acc_x, const float* acc_y, const float* acc_z,
    int N, float dt
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < N) {
        float dt2_half = 0.5f * dt * dt;
        pos_x[i] += vel_x[i] * dt + acc_x[i] * dt2_half;
        pos_y[i] += vel_y[i] * dt + acc_y[i] * dt2_half;
        pos_z[i] += vel_z[i] * dt + acc_z[i] * dt2_half;
    }
}

// 速度更新核函数
__global__ void updateVelocitiesKernel(
    float* vel_x, float* vel_y, float* vel_z,
    const float* acc_old_x, const float* acc_old_y, const float* acc_old_z,
    const float* acc_x, const float* acc_y, const float* acc_z,
    int N, float dt
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < N) {
        float dt_half = 0.5f * dt;
        vel_x[i] += (acc_old_x[i] + acc_x[i]) * dt_half;
        vel_y[i] += (acc_old_y[i] + acc_y[i]) * dt_half;
        vel_z[i] += (acc_old_z[i] + acc_z[i]) * dt_half;
    }
}
```

---

## 算法选择指南

### 决策流程

```
粒子数 N?
│
├── N < 10,000
│   └── Direct N²（最简单，精确）
│
├── 10,000 ≤ N < 100,000
│   └── Barnes-Hut，θ=0.5（平衡）
│
├── N ≥ 100,000
│   ├── 长程力（引力）
│   │   └── Barnes-Hut，θ=0.7（更快近似）
│   │
│   └── 短程力（分子）
│       └── Spatial Hash（O(N) 最优）
```

### 性能对比

| 粒子数 | Direct N² | Barnes-Hut | Spatial Hash |
|--------|-----------|------------|--------------|
| 1千 | 0.1 ms | 0.5 ms | 0.1 ms |
| 1万 | 10 ms | 2 ms | 0.5 ms |
| 10万 | 1000 ms | 15 ms | 5 ms |
| 100万 | 100s | 200 ms | 50 ms |

*NVIDIA RTX 3080 上的计时*

### 精度对比

| 算法 | 相对误差 | 推荐 θ |
|------|----------|--------|
| Direct N² | 0（精确） | — |
| Barnes-Hut | < 0.1% 到 < 5% | 0.3 到 1.0 |
| Spatial Hash | 截断范围内精确 | — |

---

## 参考文献

1. Barnes, J., & Hut, P. (1986). A hierarchical O(N log N) force-calculation algorithm. *Nature*, 324(6096), 446-449.

2. Nyland, L., Harris, M., & Prins, J. (2007). Fast N-body simulation with CUDA. *GPU Gems 3*, 677-695.

3. Green, S. (2010). Particle simulation using CUDA. *NVIDIA Whitepaper*.

4. Verlet, L. (1967). Computer "experiments" on classical fluids. *Physical Review*, 159(1), 98.

5. Salmon, J. K., & Warren, M. S. (1994). Skeletons from the treecode closet. *Journal of Computational Physics*, 111(1), 136-155.
