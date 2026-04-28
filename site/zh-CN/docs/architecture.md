---
layout: docs-zh
title: 架构概览
lang: zh-CN
description: N-Body 粒子仿真系统的系统架构、组件交互和设计模式
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

**职责：**
- 初始化粒子分布
- 协调仿真步骤
- 管理组件生命周期
- 处理状态持久化

### 2. ForceCalculator (策略模式)

力计算算法的抽象接口。

| 类名 | 复杂度 | 适用场景 |
|------|--------|----------|
| `DirectForceCalculator` | O(N²) | 小规模系统，精确结果 |
| `BarnesHutCalculator` | O(N log N) | 大规模引力仿真 |
| `SpatialHashCalculator` | O(N) | 短程力仿真 |

### 3. Integrator

Velocity Verlet 辛积分器，长期保持能量守恒。

### 4. CudaGLInterop

实现 CUDA 和 OpenGL 之间的零拷贝共享。

---

## 💾 内存架构

### ParticleData (数组结构)

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

## 🧩 设计模式

### 策略模式：ForceCalculator
用于运行时切换算法。

### 桥接模式：CudaGLInterop
解耦 CUDA 计算和 OpenGL 渲染。

### 外观模式：ParticleSystem
简化复杂子系统交互。

---

## 📚 相关文档

- [API 参考](./api/) - 详细 API 文档
- [算法详解](./algorithms/) - 算法说明
- [性能指南](./performance/) - 优化策略
- [快速入门](./getting-started/) - 安装和使用
