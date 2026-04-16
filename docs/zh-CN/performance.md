---
layout: default
title: 性能指南
parent: 文档
nav_order: 5
---

# 性能指南

N-Body 粒子仿真系统的优化策略和性能调优。

---

## 📑 目录

1. [基准测试](#基准测试)
2. [GPU 优化](#gpu-优化)
3. [内存优化](#内存优化)
4. [算法调优](#算法调优)
5. [分析工具](#分析工具)
6. [故障排除](#故障排除)

---

## 基准测试

### 性能目标

| 粒子数 | 目标 FPS | 推荐算法 |
|--------|----------|----------|
| 10,000 | 60+ | Direct N² |
| 100,000 | 60+ | Barnes-Hut |
| 1,000,000 | 30+ | Barnes-Hut / Spatial Hash |

### 参考硬件

| 组件 | 规格 |
|------|------|
| GPU | NVIDIA RTX 3080 (10GB) |
| CUDA | 12.2 |
| CPU | AMD Ryzen 9 5900X |
| 内存 | 32GB DDR4-3600 |

### 测试性能

#### 帧率 (FPS)

| 粒子数 | Direct N² | Barnes-Hut (θ=0.5) | Spatial Hash |
|--------|-----------|-------------------|--------------|
| 1,000 | 60+ | 60+ | 60+ |
| 10,000 | 60+ | 60+ | 60+ |
| 50,000 | ~30 | 60+ | 60+ |
| 100,000 | ~8 | 60+ | 60+ |
| 500,000 | <1 | ~45 | 60+ |
| 1,000,000 | N/A | ~25 | 60+ |

#### 内存使用

| 粒子数 | 粒子数据 | Barnes-Hut | Spatial Hash | 总计 |
|--------|----------|------------|--------------|------|
| 10万 | ~5 MB | ~10 MB | ~2 MB | ~17 MB |
| 100万 | ~50 MB | ~100 MB | ~20 MB | ~170 MB |
| 1000万 | ~500 MB | ~1 GB | ~200 MB | ~1.7 GB |

---

## GPU 优化

### 1. 线程块大小

按 GPU 架构推荐块大小：

| 架构 | 系列 | 最优块大小 |
|------|------|-----------|
| Ada Lovelace | RTX 40xx | 256 或 512 |
| Ampere | RTX 30xx | 256 |
| Turing | RTX 20xx | 256 |
| Volta | V100 | 256 或 512 |

```cpp
// 查询最优配置
cudaDeviceProp prop;
cudaGetDeviceProperties(&prop, 0);

int max_threads = prop.maxThreadsPerBlock;
int warp_size = prop.warpSize;
int optimal_block_size = (max_threads / warp_size / 2) * warp_size;  // 通常 256
```

### 2. 共享内存使用

对于 Direct N² 核函数，共享内存减少全局内存流量 ~50%：

```cpp
// 每块共享内存
size_t shared_size = block_size * 4 * sizeof(float);  // pos_x, pos_y, pos_z, mass

// 检查限制
if (shared_size > prop.sharedMemPerBlock) {
    // 减小块大小或禁用共享内存瓦片
}
```

### 3. 占用率

使用 CUDA 占用率 API 找最优配置：

```cpp
int min_grid_size, optimal_block_size;
cudaOccupancyMaxPotentialBlockSize(
    &min_grid_size, &optimal_block_size,
    computeForcesDirectKernel,
    0,  // 动态共享内存
    0   // 块大小限制
);
```

### 4. 内存合并

确保合并内存访问：

```cpp
// 好：合并访问
int i = blockIdx.x * blockDim.x + threadIdx.x;
float x = pos_x[i];  // 线程 i 访问地址 i

// 差：跨步访问（不合并）
float x = pos_x[i * stride];  // 线程访问非连续地址
```

### 5. Warp 分歧

最小化 warp 内分支分歧：

```cpp
// 差：Warp 分歧
if (threadIdx.x % 2 == 0) {
    // 路径 A（16 线程活动）
} else {
    // 路径 B（16 线程活动）
}

// 好：无分歧
float result = (threadIdx.x % 2 == 0) ? value_a : value_b;
```

---

## 内存优化

### 数据布局

使用数组结构（SoA）而非结构数组（AoS）：

```cpp
// 差：AoS（GPU 不友好）
struct Particle { float x, y, z, vx, vy, vz, mass; };
Particle particles[N];

// 好：SoA（合并访问）
struct ParticleData {
    float* pos_x; float* pos_y; float* pos_z;
    float* vel_x; float* vel_y; float* vel_z;
    float* mass;
};
```

### 内存池

避免频繁分配：

```cpp
class MemoryPool {
public:
    void* allocate(size_t size) {
        if (pools_.count(size) && !pools_[size].empty()) {
            void* ptr = pools_[size].back();
            pools_[size].pop_back();
            return ptr;
        }
        void* ptr;
        cudaMalloc(&ptr, size);
        return ptr;
    }
    
    void deallocate(void* ptr, size_t size) {
        pools_[size].push_back(ptr);
    }
    
private:
    std::map<size_t, std::vector<void*>> pools_;
};
```

### 零拷贝渲染

CUDA-OpenGL 互操作消除 CPU-GPU 传输：

```cpp
// 传统（慢）
cudaMemcpy(h_positions, d_positions, size, cudaMemcpyDeviceToHost);
glBufferData(GL_ARRAY_BUFFER, size, h_positions, GL_DYNAMIC_DRAW);

// 零拷贝（快）
float* d_vbo = interop.mapPositionBuffer();
updatePositionsKernel<<<grid, block>>>(d_particles, d_vbo);
interop.unmapPositionBuffer();
// VBO 可直接渲染
```

---

## 算法调优

### Barnes-Hut Theta 参数

平衡精度与性能：

```cpp
// 基于 FPS 自适应 theta
void adaptTheta(float current_fps, float target_fps) {
    if (current_fps < target_fps * 0.9f) {
        theta = std::min(theta + 0.05f, 1.0f);  // 更快，精度更低
    } else if (current_fps > target_fps * 1.1f) {
        theta = std::max(theta - 0.02f, 0.3f);  // 更慢，精度更高
    }
}
```

### Spatial Hash 单元格大小

最优单元格大小等于截断半径：

```cpp
float optimal_cell_size = cutoff_radius;

// 对于非均匀分布，稍大一些
float cell_size = cutoff_radius * 1.2f;
```

### 时间步长选择

估计最大稳定时间步长：

```cpp
float estimateMaxTimeStep(const ParticleData* d_particles, float softening) {
    // 找最大速度和加速度
    float v_max = computeMaxVelocity(d_particles);
    float a_max = computeMaxAcceleration(d_particles);
    
    // 稳定性条件
    float dt_pos = softening / v_max;           // 位置变化限制
    float dt_vel = sqrt(softening / a_max);     // 速度变化限制
    
    // 使用安全系数
    return std::min(dt_pos, dt_vel) * 0.5f;
}
```

---

## 分析工具

### NVIDIA Nsight Systems

系统级性能分析：

```bash
nsys profile --stats=true -o report ./nbody_sim 100000
nsys-ui report.nsys-rep  # GUI 打开
```

**关键指标：**
- GPU 利用率
- CPU-GPU 同步开销
- 核函数执行时间
- 内存传输时间

### NVIDIA Nsight Compute

核函数级详细分析：

```bash
ncu --set full -o report.ncu-rep ./nbody_sim 100000
ncu-ui report.ncu-rep  # GUI 打开
```

**关键指标：**
- 占用率 (%)
- 内存吞吐量
- 计算吞吐量
- 指令混合
- Warp 停顿原因

### 内置性能计数器

```cpp
class PerformanceCounter {
public:
    void start() {
        cudaEventRecord(start_);
    }
    
    void stop() {
        cudaEventRecord(stop_);
        cudaEventSynchronize(stop_);
        
        float ms;
        cudaEventElapsedTime(&ms, start_, stop_);
        times_.push_back(ms);
    }
    
    float getAverageFPS() const {
        float avg = std::accumulate(times_.begin(), times_.end(), 0.0f) 
                   / times_.size();
        return 1000.0f / avg;
    }
    
private:
    cudaEvent_t start_, stop_;
    std::vector<float> times_;
};
```

---

## 故障排除

### GPU 利用率低

**症状：** GPU 利用率 < 50%

**解决方案：**
1. 增大块大小
2. 减少同步点
3. 使用多个 CUDA 流
4. 检查 CPU 瓶颈

### 内存带宽瓶颈

**症状：** 计算吞吐量低，内存吞吐量高

**解决方案：**
1. 使用共享内存瓦片
2. 确保合并访问
3. 考虑算法更改
4. 降低精度（float16）

### 寄存器溢出

**症状：** 性能突然下降

**使用检查：** `ncu --metrics launch_stats`

**解决方案：**
1. 减少局部变量
2. 使用 `__launch_bounds__`：
   ```cpp
   __global__ void __launch_bounds__(256, 4) kernel(...) { }
   ```

### 树构建开销（Barnes-Hut）

**症状：** 小 N 时比 Direct N² 慢

**解决方案：**
```cpp
// 基于粒子数自动切换
if (particle_count < 10000) {
    useDirectMethod();
} else {
    useBarnesHut();
}
```

---

## 优化清单

分析前确保：

- [ ] Release 构建 (`-O3 -DNDEBUG`)
- [ ] 启用快速数学 (`-use_fast_math`)
- [ ] 正确架构 (`-arch=sm_86`)
- [ ] 最新 GPU 驱动
- [ ] 热路径无调试输出

优化优先级：

1. 选择正确算法
2. 优化内存访问模式
3. 调整线程块大小
4. 使用共享内存
5. 减少同步
6. 微优化

---

## 参考文献

- [CUDA 最佳实践指南](https://docs.nvidia.com/cuda/cuda-c-best-practices-guide/)
- [CUDA 编程指南](https://docs.nvidia.com/cuda/cuda-c-programming-guide/)
- [Nsight 文档](https://docs.nvidia.com/nsight-systems/)
