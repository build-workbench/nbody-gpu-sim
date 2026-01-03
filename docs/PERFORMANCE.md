# 性能优化指南

本文档介绍 N-Body 粒子仿真系统的性能优化策略和调优建议。

## 目录

- [性能基准](#性能基准)
- [GPU 优化策略](#gpu-优化策略)
- [内存优化](#内存优化)
- [算法参数调优](#算法参数调优)
- [性能分析工具](#性能分析工具)
- [常见性能问题](#常见性能问题)

---

## 性能基准

### 测试环境

| 组件 | 规格 |
|------|------|
| GPU | NVIDIA RTX 3080 (10GB) |
| CUDA | 11.8 |
| CPU | AMD Ryzen 9 5900X |
| RAM | 32GB DDR4-3600 |
| OS | Ubuntu 22.04 |

### 帧率基准 (60 FPS 目标)

| 粒子数 | Direct N² | Barnes-Hut | Spatial Hash |
|--------|-----------|------------|--------------|
| 1,000  | 60+ FPS   | 60+ FPS    | 60+ FPS      |
| 10,000 | 60+ FPS   | 60+ FPS    | 60+ FPS      |
| 50,000 | ~30 FPS   | 60+ FPS    | 60+ FPS      |
| 100,000| ~8 FPS    | 60+ FPS    | 60+ FPS      |
| 500,000| <1 FPS    | ~45 FPS    | 60+ FPS      |
| 1,000,000| N/A     | ~25 FPS    | 60+ FPS      |

### 内存使用

每个粒子的内存占用：

| 数据 | 大小 |
|------|------|
| 位置 (x, y, z) | 12 bytes |
| 速度 (vx, vy, vz) | 12 bytes |
| 加速度 (ax, ay, az) | 12 bytes |
| 旧加速度 | 12 bytes |
| 质量 | 4 bytes |
| **总计** | **52 bytes** |

| 粒子数 | 粒子数据 | Barnes-Hut 树 | Spatial Hash | 总计 |
|--------|----------|---------------|--------------|------|
| 100K   | ~5 MB    | ~10 MB        | ~2 MB        | ~17 MB |
| 1M     | ~50 MB   | ~100 MB       | ~20 MB       | ~170 MB |
| 10M    | ~500 MB  | ~1 GB         | ~200 MB      | ~1.7 GB |

---

## GPU 优化策略

### 1. 线程块大小

最佳线程块大小取决于 GPU 架构和核函数特性：

```cpp
// 推荐值
const int BLOCK_SIZE = 256;  // 通用默认值

// 根据 GPU 调整
cudaDeviceProp prop;
cudaGetDeviceProperties(&prop, 0);
int optimal_block_size = prop.maxThreadsPerBlock / 2;  // 通常 512 或 256
```

调优建议：
- Ampere 架构 (RTX 30xx): 256 或 512
- Turing 架构 (RTX 20xx): 256
- Volta 架构 (V100): 256 或 512

### 2. Shared Memory 使用

Direct N² 核函数的 Shared Memory 配置：

```cpp
// 每个线程块需要的 Shared Memory
size_t shared_size = BLOCK_SIZE * 4 * sizeof(float);  // pos_x, pos_y, pos_z, mass

// 检查是否超出限制
if (shared_size > prop.sharedMemPerBlock) {
    // 减小 BLOCK_SIZE
}
```

### 3. 占用率优化

使用 CUDA Occupancy API 计算最佳配置：

```cpp
int min_grid_size, block_size;
cudaOccupancyMaxPotentialBlockSize(
    &min_grid_size, &block_size,
    computeForcesDirectKernel,
    shared_size, 0
);
```

### 4. 内存合并访问

确保连续线程访问连续内存：

```cpp
// 好: 合并访问
int i = blockIdx.x * blockDim.x + threadIdx.x;
float x = pos_x[i];  // 连续线程访问连续地址

// 差: 跨步访问
float x = pos_x[i * stride];  // 内存访问不连续
```

### 5. 避免 Warp 分歧

```cpp
// 差: 条件分支导致 warp 分歧
if (i % 2 == 0) {
    // 一半线程执行
} else {
    // 另一半线程执行
}

// 好: 所有线程执行相同路径
float result = (i % 2 == 0) ? value_a : value_b;
```

---

## 内存优化

### 1. SoA vs AoS

Structure of Arrays (SoA) 布局更适合 GPU：

```cpp
// AoS (Array of Structures) - 不推荐
struct Particle {
    float x, y, z;
    float vx, vy, vz;
    float mass;
};
Particle particles[N];

// SoA (Structure of Arrays) - 推荐
struct ParticleData {
    float* pos_x;
    float* pos_y;
    float* pos_z;
    float* vel_x;
    float* vel_y;
    float* vel_z;
    float* mass;
};
```

SoA 优势：
- 更好的内存合并
- 更高的缓存命中率
- 更容易向量化

### 2. 内存池

避免频繁分配/释放：

```cpp
class MemoryPool {
public:
    void* allocate(size_t size) {
        if (pool.count(size) && !pool[size].empty()) {
            void* ptr = pool[size].back();
            pool[size].pop_back();
            return ptr;
        }
        void* ptr;
        cudaMalloc(&ptr, size);
        return ptr;
    }
    
    void deallocate(void* ptr, size_t size) {
        pool[size].push_back(ptr);
    }
    
private:
    std::map<size_t, std::vector<void*>> pool;
};
```

### 3. 异步内存传输

使用 CUDA Streams 重叠计算和传输：

```cpp
cudaStream_t stream1, stream2;
cudaStreamCreate(&stream1);
cudaStreamCreate(&stream2);

// 异步传输
cudaMemcpyAsync(d_data, h_data, size, cudaMemcpyHostToDevice, stream1);

// 在另一个流上计算
kernel<<<grid, block, 0, stream2>>>(other_data);

// 同步
cudaStreamSynchronize(stream1);
cudaStreamSynchronize(stream2);
```

### 4. 零拷贝渲染

CUDA-OpenGL 互操作避免 GPU→CPU→GPU 传输：

```cpp
// 传统方式 (慢)
cudaMemcpy(h_positions, d_positions, size, cudaMemcpyDeviceToHost);
glBufferData(GL_ARRAY_BUFFER, size, h_positions, GL_DYNAMIC_DRAW);

// 零拷贝方式 (快)
float* d_vbo_ptr = interop.mapPositionBuffer();
updatePositionsKernel<<<grid, block>>>(d_particles, d_vbo_ptr);
interop.unmapPositionBuffer();
// 直接渲染，无需传输
```

---

## 算法参数调优

### Barnes-Hut θ 参数

| θ 值 | 精度 | 性能 | 适用场景 |
|------|------|------|----------|
| 0.3  | 高   | 慢   | 科学计算 |
| 0.5  | 中   | 中   | 一般仿真 |
| 0.7  | 低   | 快   | 实时可视化 |
| 1.0  | 很低 | 很快 | 快速预览 |

动态调整策略：

```cpp
// 根据帧率动态调整 θ
void adaptTheta(float current_fps, float target_fps) {
    if (current_fps < target_fps * 0.9f) {
        theta = std::min(theta + 0.05f, 1.0f);  // 降低精度提高性能
    } else if (current_fps > target_fps * 1.1f) {
        theta = std::max(theta - 0.02f, 0.3f);  // 提高精度
    }
}
```

### Spatial Hash Cell Size

最佳 cell size 约等于截断半径：

```cpp
float optimal_cell_size = cutoff_radius;

// 如果粒子分布不均匀，可以稍大
float cell_size = cutoff_radius * 1.2f;
```

Cell size 过小：
- 更多 cell 需要搜索
- 内存开销增加

Cell size 过大：
- 每个 cell 粒子过多
- 计算量增加

### 时间步长 dt

稳定性条件：

```cpp
// 估算最大安全时间步长
float v_max = computeMaxVelocity(particles);
float a_max = computeMaxAcceleration(particles);
float dt_max = std::min(
    softening / v_max,           // 位置变化限制
    sqrt(softening / a_max)      // 加速度变化限制
);

// 使用安全系数
float dt = dt_max * 0.5f;
```

---

## 性能分析工具

### NVIDIA Nsight Systems

系统级性能分析：

```bash
nsys profile --stats=true ./nbody_sim 100000
```

关注指标：
- GPU 利用率
- 内存带宽
- Kernel 执行时间
- CPU-GPU 同步开销

### NVIDIA Nsight Compute

Kernel 级详细分析：

```bash
ncu --set full ./nbody_sim 100000
```

关注指标：
- 占用率 (Occupancy)
- 内存吞吐量
- 计算吞吐量
- Warp 效率

### 内置性能计数器

```cpp
class PerformanceCounter {
public:
    void startFrame() {
        cudaEventRecord(start);
    }
    
    void endFrame() {
        cudaEventRecord(stop);
        cudaEventSynchronize(stop);
        
        float ms;
        cudaEventElapsedTime(&ms, start, stop);
        frame_times.push_back(ms);
    }
    
    float getAverageFPS() {
        float avg_ms = std::accumulate(frame_times.begin(), frame_times.end(), 0.0f) 
                       / frame_times.size();
        return 1000.0f / avg_ms;
    }
    
private:
    cudaEvent_t start, stop;
    std::vector<float> frame_times;
};
```

---

## 常见性能问题

### 1. 低 GPU 利用率

症状：GPU 利用率 < 50%

原因：
- 线程块太小
- 过多同步点
- CPU 瓶颈

解决：
```cpp
// 增大线程块
int block_size = 512;

// 减少同步
// 避免不必要的 cudaDeviceSynchronize()

// 使用异步操作
cudaMemcpyAsync(...);
```

### 2. 内存带宽瓶颈

症状：计算吞吐量低，内存吞吐量接近峰值

原因：
- 数据重复读取
- 内存访问不连续

解决：
```cpp
// 使用 Shared Memory 缓存
extern __shared__ float shared[];

// 确保合并访问
// 使用 SoA 布局
```

### 3. Warp 分歧

症状：Warp 执行效率 < 100%

原因：
- 条件分支
- 不均匀工作负载

解决：
```cpp
// 使用谓词代替分支
float result = condition ? a : b;

// 工作负载均衡
// 使用动态并行或工作窃取
```

### 4. 寄存器溢出

症状：Kernel 性能突然下降

原因：
- 核函数使用过多局部变量
- 寄存器不足导致溢出到 Local Memory

解决：
```cpp
// 减少局部变量
// 使用 __launch_bounds__ 限制寄存器使用
__global__ void __launch_bounds__(256, 4) kernel(...) {
    // ...
}
```

### 5. 树构建开销

症状：Barnes-Hut 在小规模时比 Direct N² 慢

原因：
- 树构建是 O(N log N)
- 小规模时构建开销占比大

解决：
```cpp
// 小规模时切换到 Direct N²
if (particle_count < 10000) {
    useDirectMethod();
} else {
    useBarnesHut();
}
```

---

## 性能检查清单

在优化前，确认以下事项：

- [ ] 使用 Release 构建 (`-O3 -DNDEBUG`)
- [ ] 启用 CUDA 优化 (`-use_fast_math`)
- [ ] 正确设置 GPU 架构 (`-arch=sm_86`)
- [ ] 禁用调试输出
- [ ] 使用最新驱动

优化优先级：

1. 选择正确的算法
2. 优化内存访问模式
3. 调整线程块大小
4. 使用 Shared Memory
5. 减少同步开销
6. 微优化（数学函数等）

---

## 扩展阅读

- [CUDA C++ Best Practices Guide](https://docs.nvidia.com/cuda/cuda-c-best-practices-guide/)
- [CUDA C++ Programming Guide](https://docs.nvidia.com/cuda/cuda-c-programming-guide/)
- [Nsight Systems User Guide](https://docs.nvidia.com/nsight-systems/)
- [Nsight Compute User Guide](https://docs.nvidia.com/nsight-compute/)
