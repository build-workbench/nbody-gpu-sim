---
layout: default
title: 架构概览
parent: 文档
nav_order: 2
---

# 架构概览

本文档介绍 N-Body 粒子仿真系统的系统架构、组件交互和设计模式。

---

## 📐 高层架构

### 系统分层

```
┌─────────────────────────────────────────────────────────┐
│                      应用层                              │
│  • 窗口管理 (GLFW)                                      │
│  • 输入处理                                             │
│  • 主事件循环                                           │
├─────────────────────────────────────────────────────────┤
│                      仿真层                              │
│  • ParticleSystem (协调器)                              │
│  • ForceCalculator (策略模式)                           │
│  • Integrator (Velocity Verlet)                         │
│  • CudaGLInterop (CUDA-OpenGL 桥接)                     │
├─────────────────────────────────────────────────────────┤
│                      渲染层                              │
│  • Renderer (OpenGL)                                    │
│  • Camera (轨道控制)                                    │
│  • 着色器管理                                           │
├─────────────────────────────────────────────────────────┤
│                      GPU 内存层                          │
│  • ParticleData (SoA 布局)                              │
│  • Shared VBO (零拷贝互操作)                            │
│  • 加速结构                                             │
└─────────────────────────────────────────────────────────┘
```

---

## 🔧 核心组件

### 1. ParticleSystem

管理所有仿真组件的中央协调器。

```cpp
class ParticleSystem {
public:
    void initialize(const SimulationConfig& config);
    void update(float dt);
    void pause(); / resume();
    void reset();
    
    void saveState(const std::string& filename);
    void loadState(const std::string& filename);
    
private:
    ParticleData d_particles_;                    // GPU 数据
    std::unique_ptr<ForceCalculator> force_calc_;
    std::unique_ptr<Integrator> integrator_;
    std::unique_ptr<CudaGLInterop> interop_;
};
```

**职责：**
- 初始化粒子分布
- 协调仿真步骤
- 管理组件生命周期
- 处理状态持久化

### 2. ForceCalculator (策略模式)

力计算算法的抽象接口。

```cpp
class ForceCalculator {
public:
    virtual void computeForces(ParticleData* d_particles) = 0;
    virtual ForceMethod getMethod() const = 0;
    
    void setGravitationalConstant(float G);
    void setSofteningParameter(float eps);
};
```

**实现类：**

| 类名 | 复杂度 | 适用场景 |
|------|--------|----------|
| `DirectForceCalculator` | O(N²) | 小规模系统，精确结果 |
| `BarnesHutCalculator` | O(N log N) | 大规模引力仿真 |
| `SpatialHashCalculator` | O(N) | 短程力仿真 |

### 3. Integrator

Velocity Verlet 辛积分器。

```cpp
class Integrator {
public:
    void integrate(ParticleData* d_particles, 
                   ForceCalculator* force_calc, 
                   float dt);
    
private:
    void updatePositions(ParticleData* d_particles, float dt);
    void updateVelocities(ParticleData* d_particles, float dt);
    void storeOldAccelerations(ParticleData* d_particles);
};
```

**算法：**
```
1. a_old = a(t)                           // 保存旧加速度
2. x(t+dt) = x(t) + v(t)·dt + ½·a(t)·dt² // 更新位置
3. a(t+dt) = F(x(t+dt))/m                // 计算新力
4. v(t+dt) = v(t) + ½·(a_old + a(t+dt))·dt // 更新速度
```

### 4. CudaGLInterop

实现 CUDA 和 OpenGL 之间的零拷贝共享。

```cpp
class CudaGLInterop {
public:
    void initialize(size_t particle_count);
    float* mapPositionBuffer();     // 获取 CUDA 设备指针
    void unmapPositionBuffer();     // 释放给 OpenGL
    void updatePositions(const ParticleData* d_particles);
    
    GLuint getPositionVBO() const;
    
private:
    GLuint position_vbo_;
    cudaGraphicsResource* cuda_vbo_;
};
```

**数据流：**
```
CUDA 计算 → 映射 VBO → 复制位置 → 解映射 VBO → OpenGL 渲染
     ↑                                                  │
     └─────────── 零拷贝（同一 GPU 内存）───────────────┘
```

---

## 💾 内存架构

### ParticleData (数组结构)

```cpp
struct ParticleData {
    // 位置 (3N floats)
    float* pos_x; float* pos_y; float* pos_z;
    
    // 速度 (3N floats)
    float* vel_x; float* vel_y; float* vel_z;
    
    // 加速度 (6N floats - 当前 + 旧值)
    float* acc_x; float* acc_y; float* acc_z;
    float* acc_old_x; float* acc_old_y; float* acc_old_z;
    
    // 质量 (N floats)
    float* mass;
    
    size_t count;
};
```

**每个粒子内存：** 52 字节（13 个 float）

### 内存预算（100万粒子）

| 组件 | 大小 | 说明 |
|------|------|------|
| ParticleData | 52 MB | 核心粒子状态 |
| Shared VBO | 12 MB | CUDA-OpenGL 互操作 |
| Barnes-Hut 树 | ~100 MB | 八叉树结构 |
| Spatial Hash 网格 | ~20 MB | 单元格查找 |
| **总计** | **~184 MB** | 最大使用 |

---

## 🔄 数据流

### 仿真循环

```
┌────────────────────────────────────────┐
│  1. 输入处理                           │
│     • 轮询 GLFW 事件                   │
│     • 更新相机                         │
│     • 处理控制                         │
└───────────────┬────────────────────────┘
                ▼
┌────────────────────────────────────────┐
│  2. 物理更新（如未暂停）               │
│     • Velocity Verlet 积分             │
│     • 力计算                           │
└───────────────┬────────────────────────┘
                ▼
┌────────────────────────────────────────┐
│  3. 数据传输                           │
│     • 通过互操作更新 VBO               │
│     • SoA → 交错格式转换               │
└───────────────┬────────────────────────┘
                ▼
┌────────────────────────────────────────┐
│  4. 渲染                               │
│     • 绑定着色器/绑定                  │
│     • 绘制点精灵                       │
│     • 交换缓冲区                       │
└───────────────┬────────────────────────┘
                ▼
             [以 60Hz 重复]
```

### 算法专用流

#### Direct N²
```
ParticleData → [加载瓦片到共享内存] → [计算所有粒子对] → 力
```

#### Barnes-Hut
```
ParticleData → [边界盒] → [Morton 码] → [排序] → [构建树]
                                                  → [遍历] → 力
```

#### Spatial Hash
```
ParticleData → [计算单元格 ID] → [排序] → [构建范围]
                                              → [邻居搜索] → 力
```

---

## 🧩 设计模式

### 策略模式：ForceCalculator

用于运行时切换算法。

```cpp
// 接口
class ForceCalculator {
    virtual void computeForces(ParticleData* d_particles) = 0;
};

// 具体策略
class DirectForceCalculator : public ForceCalculator { };
class BarnesHutCalculator : public ForceCalculator { };
class SpatialHashCalculator : public ForceCalculator { };

// 使用
std::unique_ptr<ForceCalculator> force_calc;
force_calc = std::make_unique<BarnesHutCalculator>(theta);
force_calc->computeForces(d_particles);
```

### 桥接模式：CudaGLInterop

解耦 CUDA 计算和 OpenGL 渲染。

```cpp
// CUDA 端（写入）
float* d_vbo = interop.mapPositionBuffer();
kernel<<<grid, block>>>(d_particles, d_vbo);
interop.unmapPositionBuffer();

// OpenGL 端（读取）
renderer.render(interop.getPositionVBO(), count);
```

### 外观模式：ParticleSystem

简化复杂子系统交互。

```cpp
// 客户端代码无需了解：
// - 核函数启动
// - 内存管理
// - 互操作细节

ParticleSystem system;
system.initialize(config);
system.update(dt);  // 简洁、清晰的接口
```

---

## 🔌 扩展点

### 添加新力算法

1. **创建新类：**

```cpp
// include/nbody/my_force_calculator.hpp
class MyForceCalculator : public ForceCalculator {
public:
    void computeForces(ParticleData* d_particles) override;
    ForceMethod getMethod() const override { return ForceMethod::MY_METHOD; }
};
```

2. **注册到工厂：**

```cpp
// src/core/force_calculator.cpp
std::unique_ptr<ForceCalculator> createForceCalculator(ForceMethod method) {
    switch (method) {
        // ... 现有 case
        case ForceMethod::MY_METHOD:
            return std::make_unique<MyForceCalculator>(config);
    }
}
```

3. **添加 UI 绑定：**

```cpp
// src/main.cpp
case GLFW_KEY_4:
    system.setForceMethod(ForceMethod::MY_METHOD);
    break;
```

---

## 📈 性能考虑

### 关键路径

1. **力计算** - 主导运行时间
   - Direct N²：内存带宽受限
   - Barnes-Hut：树遍历受限
   - Spatial Hash：单元格迭代受限

2. **VBO 更新** - 数据传输开销
   - SoA 到交错格式转换
   - 零拷贝最小化开销

### 优化清单

- [ ] 使用 Release 构建 (`-O3`)
- [ ] 启用 CUDA fast math (`-use_fast_math`)
- [ ] 设置正确的 GPU 架构 (`-arch=sm_86`)
- [ ] 为粒子数选择最佳算法
- [ ] 调整 Barnes-Hut θ 参数
- [ ] 设置合适的 Spatial Hash 单元格大小
---

## 📚 相关文档

- [API 参考](./api.md) - 详细 API 文档
- [算法详解](./algorithms.md) - 算法说明
- [性能指南](./performance.md) - 优化策略
- [快速入门](../setup/getting-started.md) - 安装和使用
