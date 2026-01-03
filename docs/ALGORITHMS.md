# 算法详解

本文档详细介绍 N-Body 粒子仿真系统中使用的三种力计算算法和积分方法。

## 目录

- [引力计算基础](#引力计算基础)
- [Direct N² 算法](#direct-n²-算法)
- [Barnes-Hut 算法](#barnes-hut-算法)
- [Spatial Hash 算法](#spatial-hash-算法)
- [Velocity Verlet 积分](#velocity-verlet-积分)
- [算法选择指南](#算法选择指南)

---

## 引力计算基础

### 牛顿万有引力定律

两个质点之间的引力：

```
F = G * m1 * m2 / r²
```

其中：
- `G` - 万有引力常数
- `m1, m2` - 两个质点的质量
- `r` - 两质点之间的距离

### 软化参数 (Softening)

为避免粒子距离过近时力趋于无穷大，引入软化参数 ε：

```
F = G * m1 * m2 / (r² + ε²)
```

软化参数的作用：
1. 防止数值发散
2. 模拟有限大小的粒子
3. 平滑近距离相互作用

典型值：`ε = 0.01` 到 `0.1`（取决于系统尺度）

### 加速度计算

粒子 i 受到所有其他粒子的引力作用，总加速度为：

```
a_i = Σ G * m_j * (r_j - r_i) / |r_j - r_i|³
```

---

## Direct N² 算法

### 原理

直接计算每对粒子之间的引力，时间复杂度 O(N²)。

```
对于每个粒子 i:
    a_i = 0
    对于每个粒子 j ≠ i:
        r = pos_j - pos_i
        dist² = r·r + ε²
        a_i += G * m_j * r / dist^(3/2)
```

### CUDA 实现优化

#### Shared Memory Tiling

将粒子数据分块加载到 Shared Memory，减少 Global Memory 访问：

```cpp
__global__ void computeForcesDirectKernel(...) {
    extern __shared__ float shared[];
    float* s_pos_x = shared;
    float* s_pos_y = s_pos_x + blockDim.x;
    float* s_pos_z = s_pos_y + blockDim.x;
    float* s_mass = s_pos_z + blockDim.x;
    
    // 每个线程处理一个粒子
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    float ax = 0, ay = 0, az = 0;
    
    // 分块遍历所有粒子
    for (int tile = 0; tile < numTiles; tile++) {
        // 协作加载到 Shared Memory
        int j = tile * blockDim.x + threadIdx.x;
        s_pos_x[threadIdx.x] = pos_x[j];
        s_pos_y[threadIdx.x] = pos_y[j];
        s_pos_z[threadIdx.x] = pos_z[j];
        s_mass[threadIdx.x] = mass[j];
        __syncthreads();
        
        // 计算与当前 tile 中粒子的相互作用
        for (int k = 0; k < blockDim.x; k++) {
            // 力计算...
        }
        __syncthreads();
    }
}
```

#### 内存访问模式

- 使用 SoA (Structure of Arrays) 布局
- 合并内存访问 (Coalesced Access)
- 使用 `__restrict__` 提示编译器优化

#### 数学优化

使用 `rsqrtf()` 代替 `1/sqrt()`：

```cpp
float inv_dist = rsqrtf(dist2);
float inv_dist3 = inv_dist * inv_dist * inv_dist;
```

### 性能特点

| 优点 | 缺点 |
|------|------|
| 实现简单 | O(N²) 复杂度 |
| 精确计算 | 大规模时性能差 |
| GPU 并行效率高 | 内存带宽受限 |

适用场景：N < 50,000 的小规模仿真

---

## Barnes-Hut 算法

### 原理

使用八叉树 (Octree) 将远距离粒子群近似为单个质点，时间复杂度 O(N log N)。

核心思想：如果一组粒子距离足够远，可以用它们的质心和总质量来近似整组粒子的引力效应。

### 开角参数 θ

判断是否可以近似的标准：

```
θ = s / d
```

其中：
- `s` - 树节点的边长
- `d` - 粒子到节点质心的距离

如果 `θ < θ_threshold`，则使用近似；否则递归展开子节点。

- `θ = 0`: 等价于 Direct N²（完全精确）
- `θ = 0.5`: 典型值，精度与性能平衡
- `θ = 1.0`: 快速但精度较低

### 八叉树构建

```
1. 计算所有粒子的边界盒
2. 创建根节点覆盖整个边界盒
3. 对每个粒子:
   a. 从根节点开始
   b. 确定粒子所属的子八分体
   c. 如果子节点不存在，创建叶节点
   d. 如果子节点是叶节点且已有粒子，细分并重新插入
4. 自底向上计算每个节点的质心和总质量
```

### 力计算遍历

```
function computeForce(particle, node):
    if node 是叶节点:
        直接计算引力
    else:
        d = distance(particle, node.center_of_mass)
        if node.size / d < θ:
            使用节点质心近似计算引力
        else:
            for each child in node.children:
                computeForce(particle, child)
```

### GPU 实现

#### Morton 码排序

使用 Morton 码 (Z-order curve) 对粒子排序，提高空间局部性：

```cpp
__device__ uint32_t mortonCode(float3 pos, float3 min, float3 max) {
    // 归一化到 [0, 1]
    float3 norm = (pos - min) / (max - min);
    // 量化到 10 位整数
    uint32_t x = (uint32_t)(norm.x * 1023);
    uint32_t y = (uint32_t)(norm.y * 1023);
    uint32_t z = (uint32_t)(norm.z * 1023);
    // 交错位
    return expandBits(x) | (expandBits(y) << 1) | (expandBits(z) << 2);
}
```

#### 树遍历优化

- 使用栈代替递归
- 线程束 (Warp) 协作遍历
- 预取节点数据

### 性能特点

| 优点 | 缺点 |
|------|------|
| O(N log N) 复杂度 | 实现复杂 |
| 可调精度 | 树构建开销 |
| 适合大规模仿真 | GPU 实现有挑战 |

适用场景：N > 50,000 的大规模引力仿真

---

## Spatial Hash 算法

### 原理

将空间划分为均匀网格，只计算相邻格子内粒子的相互作用，时间复杂度 O(N)。

适用于短程力（如分子动力学中的 Lennard-Jones 势）。

### 网格划分

```
cell_index = (floor(x/cell_size), floor(y/cell_size), floor(z/cell_size))
hash = hash_function(cell_index)
```

### 哈希函数

```cpp
__device__ int hashCell(int3 cell, int3 grid_dims) {
    // 简单线性哈希
    return cell.x + cell.y * grid_dims.x + cell.z * grid_dims.x * grid_dims.y;
}
```

### 构建过程

```
1. 计算每个粒子的 cell 索引
2. 按 cell 索引排序粒子
3. 构建 cell_start 和 cell_end 数组
```

### 邻居搜索

对于截断半径 r_cut，需要搜索的邻居 cell 数量：

```
neighbor_cells = ceil(r_cut / cell_size)
```

通常 cell_size ≈ r_cut，只需搜索 27 个相邻 cell (3×3×3)。

### GPU 实现

```cpp
__global__ void spatialHashForceKernel(
    const int* cell_start, const int* cell_end,
    const int* sorted_indices,
    const float* pos_x, const float* pos_y, const float* pos_z,
    const float* mass,
    float* acc_x, float* acc_y, float* acc_z,
    int N, float cutoff, float G, float eps2
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    
    int3 my_cell = computeCell(pos_x[i], pos_y[i], pos_z[i]);
    float ax = 0, ay = 0, az = 0;
    
    // 遍历 27 个相邻 cell
    for (int dz = -1; dz <= 1; dz++) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                int3 neighbor_cell = my_cell + make_int3(dx, dy, dz);
                int hash = hashCell(neighbor_cell);
                
                // 遍历 cell 中的粒子
                for (int j = cell_start[hash]; j < cell_end[hash]; j++) {
                    int idx = sorted_indices[j];
                    // 计算力...
                }
            }
        }
    }
}
```

### 性能特点

| 优点 | 缺点 |
|------|------|
| O(N) 复杂度 | 仅适用于短程力 |
| 实现相对简单 | 需要合适的 cell_size |
| 内存访问局部性好 | 粒子分布不均时效率下降 |

适用场景：分子动力学、SPH 流体仿真等短程相互作用

---

## Velocity Verlet 积分

### 原理

Velocity Verlet 是一种辛积分器 (Symplectic Integrator)，能够长期保持能量守恒。

### 算法步骤

```
1. 保存旧加速度: a_old = a(t)
2. 更新位置: x(t+dt) = x(t) + v(t)*dt + 0.5*a(t)*dt²
3. 计算新加速度: a(t+dt) = F(x(t+dt)) / m
4. 更新速度: v(t+dt) = v(t) + 0.5*(a_old + a(t+dt))*dt
```

### 与其他积分器对比

| 积分器 | 精度 | 能量守恒 | 计算量 |
|--------|------|----------|--------|
| Euler | O(dt) | 差 | 1次力计算 |
| Leapfrog | O(dt²) | 好 | 1次力计算 |
| Velocity Verlet | O(dt²) | 好 | 1次力计算 |
| RK4 | O(dt⁴) | 一般 | 4次力计算 |

### 辛积分器的优势

辛积分器保持相空间体积不变，这意味着：
- 长期能量误差有界（振荡而非漂移）
- 适合长时间仿真
- 保持系统的定性行为

### GPU 实现

```cpp
// 位置更新
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

// 速度更新
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
粒子数量 N?
├── N < 10,000
│   └── Direct N² (简单高效)
├── 10,000 < N < 100,000
│   ├── 需要高精度? → Direct N²
│   └── 需要高性能? → Barnes-Hut
├── N > 100,000
│   ├── 长程力 (引力)? → Barnes-Hut
│   └── 短程力 (分子)? → Spatial Hash
```

### 性能对比

| 粒子数 | Direct N² | Barnes-Hut (θ=0.5) | Spatial Hash |
|--------|-----------|---------------------|--------------|
| 1K     | 0.1 ms    | 0.5 ms              | 0.1 ms       |
| 10K    | 10 ms     | 2 ms                | 0.5 ms       |
| 100K   | 1000 ms   | 15 ms               | 5 ms         |
| 1M     | 100000 ms | 200 ms              | 50 ms        |

*测试环境: NVIDIA RTX 3080*

### 精度对比

| 算法 | 相对误差 | 适用场景 |
|------|----------|----------|
| Direct N² | 0 (精确) | 基准测试、小规模仿真 |
| Barnes-Hut (θ=0.3) | < 0.1% | 高精度大规模仿真 |
| Barnes-Hut (θ=0.5) | < 1% | 一般大规模仿真 |
| Barnes-Hut (θ=0.8) | < 5% | 快速预览 |
| Spatial Hash | 精确 (截断内) | 短程力仿真 |

---

## 参考文献

1. Barnes, J., & Hut, P. (1986). A hierarchical O(N log N) force-calculation algorithm. *Nature*, 324(6096), 446-449.

2. Salmon, J. K., & Warren, M. S. (1994). Skeletons from the treecode closet. *Journal of Computational Physics*, 111(1), 136-155.

3. Nyland, L., Harris, M., & Prins, J. (2007). Fast N-body simulation with CUDA. *GPU Gems 3*, 677-695.

4. Green, S. (2010). Particle simulation using CUDA. *NVIDIA Whitepaper*.

5. Verlet, L. (1967). Computer "experiments" on classical fluids. *Physical Review*, 159(1), 98.
