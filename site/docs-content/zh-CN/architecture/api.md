---
layout: default
title: API 参考
parent: 文档
nav_order: 3
---

# API 参考

N-Body 粒子仿真库的完整 API 参考。

---

## 📑 目录

- [核心类](#核心类)
  - [ParticleSystem](#particlesystem)
  - [ForceCalculator](#forcecalculator)
  - [Integrator](#integrator)
  - [Camera](#camera)
  - [Renderer](#renderer)
  - [CudaGLInterop](#cudaglintinterop)
- [数据结构](#数据结构)
  - [ParticleData](#particledata)
  - [SimulationConfig](#simulationconfig)
  - [SimulationState](#simulationstate)
- [枚举类型](#枚举类型)
- [工具函数](#工具函数)

---

## 核心类

### ParticleSystem

管理整个仿真的主协调器类。

```cpp
namespace nbody {

class ParticleSystem {
public:
    // 生命周期
    ParticleSystem();
    ~ParticleSystem();
    
    // 初始化
    void initialize(const SimulationConfig& config);
    void initializeWithDistribution(size_t particle_count, InitDistribution dist);
    
    // 仿真控制
    void update(float dt);
    void pause();
    void resume();
    void reset();
    bool isPaused() const;
    
    // 参数配置
    void setForceMethod(ForceMethod method);
    void setGravitationalConstant(float G);
    void setSofteningParameter(float eps);
    void setTimeStep(float dt);
    void setBarnesHutTheta(float theta);
    void setSpatialHashCellSize(float size);
    void setSpatialHashCutoff(float cutoff);
    
    // 参数访问器
    ForceMethod getForceMethod() const;
    float getGravitationalConstant() const;
    float getSofteningParameter() const;
    float getTimeStep() const;
    float getSimulationTime() const;
    size_t getParticleCount() const;
    
    // 数据访问
    ParticleData* getDeviceData();
    void copyToHost(ParticleData& h_particles) const;
    
    // 状态管理
    void saveState(const std::string& filename) const;
    void loadState(const std::string& filename);
    SimulationState getState() const;
    void setState(const SimulationState& state);
    
    // 能量计算
    float computeKineticEnergy() const;
    float computePotentialEnergy() const;
    float computeTotalEnergy() const;
    
    // CUDA-GL 互操作
    CudaGLInterop* getInterop();
    void initializeInterop();
    void updateInteropBuffer();
};

} // namespace nbody
```

#### 使用示例

```cpp
#include "nbody/particle_system.hpp"

using namespace nbody;

int main() {
    // 配置仿真
    SimulationConfig config;
    config.particle_count = 100000;
    config.force_method = ForceMethod::BARNES_HUT;
    config.dt = 0.001f;
    config.G = 1.0f;
    
    // 初始化系统
    ParticleSystem system;
    system.initialize(config);
    
    // 运行仿真
    for (int i = 0; i < 1000; ++i) {
        system.update(system.getTimeStep());
        
        // 每 100 步监控能量
        if (i % 100 == 0) {
            float E = system.computeTotalEnergy();
            printf("Step %d: Total Energy = %.6f\n", i, E);
        }
    }
    
    // 保存检查点
    system.saveState("checkpoint.nbody");
    
    return 0;
}
```

---

### ForceCalculator

力计算算法的抽象基类。

```cpp
namespace nbody {

class ForceCalculator {
public:
    virtual ~ForceCalculator() = default;
    
    // 计算所有粒子的力
    virtual void computeForces(ParticleData* d_particles) = 0;
    
    // 获取算法类型
    virtual ForceMethod getMethod() const = 0;
    
    // 参数
    void setGravitationalConstant(float G);
    void setSofteningParameter(float eps);
    
    float getGravitationalConstant() const;
    float getSofteningParameter() const;
    
protected:
    float G_ = 1.0f;           // 引力常数
    float softening_eps_ = 0.1f;  // 软化长度
    float softening_eps2_ = 0.01f; // 软化长度平方
};

} // namespace nbody
```

#### DirectForceCalculator

```cpp
class DirectForceCalculator : public ForceCalculator {
public:
    explicit DirectForceCalculator(int block_size = 256);
    
    void computeForces(ParticleData* d_particles) override;
    ForceMethod getMethod() const override;
    
    void setBlockSize(int size);
    int getBlockSize() const;
};
```

#### BarnesHutCalculator

```cpp
class BarnesHutCalculator : public ForceCalculator {
public:
    explicit BarnesHutCalculator(float theta = 0.5f);
    
    void computeForces(ParticleData* d_particles) override;
    ForceMethod getMethod() const override;
    
    // 开角参数（0 = 精确，1 = 最快）
    void setTheta(float theta);
    float getTheta() const;
    
    BarnesHutTree* getTree() const;
};
```

#### SpatialHashCalculator

```cpp
class SpatialHashCalculator : public ForceCalculator {
public:
    SpatialHashCalculator(float cell_size = 1.0f, float cutoff_radius = 2.0f);
    
    void computeForces(ParticleData* d_particles) override;
    ForceMethod getMethod() const override;
    
    void setCellSize(float size);
    void setCutoffRadius(float radius);
    float getCellSize() const;
    float getCutoffRadius() const;
    
    SpatialHashGrid* getGrid() const;
};
```

---

### Integrator

Velocity Verlet 辛积分器。

```cpp
namespace nbody {

class Integrator {
public:
    explicit Integrator(int block_size = 256);
    
    // 完整积分步骤
    void integrate(ParticleData* d_particles, 
                   ForceCalculator* force_calc, 
                   float dt);
    
    // 分步组件（用于自定义方案）
    void updatePositions(ParticleData* d_particles, float dt);
    void updateVelocities(ParticleData* d_particles, float dt);
    void storeOldAccelerations(ParticleData* d_particles);
    
    // 能量计算
    float computeKineticEnergy(const ParticleData* d_particles);
    float computePotentialEnergy(const ParticleData* d_particles, float G, float eps);
    float computeTotalEnergy(const ParticleData* d_particles, float G, float eps);
    
    // 配置
    void setBlockSize(int size);
    int getBlockSize() const;
};

} // namespace nbody
```

---

### Camera

3D 轨道相机控制器。

```cpp
namespace nbody {

class Camera {
public:
    Camera(float fov = 45.0f, float aspect = 16.0f/9.0f, 
           float near_plane = 0.1f, float far_plane = 1000.0f);
    
    // 变换矩阵
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;
    glm::mat4 getViewProjectionMatrix() const;
    
    // 定位
    void setPosition(const glm::vec3& pos);
    void setTarget(const glm::vec3& target);
    void setUp(const glm::vec3& up);
    
    glm::vec3 getPosition() const;
    glm::vec3 getTarget() const;
    glm::vec3 getForward() const;
    glm::vec3 getRight() const;
    
    // 投影
    void setFOV(float fov);
    void setAspectRatio(float aspect);
    void setNearFar(float near_plane, float far_plane);
    
    // 控制
    void rotate(float yaw, float pitch);
    void pan(float dx, float dy);
    void zoom(float delta);
    void orbit(float yaw, float pitch);
    void reset();
    
    void setOrbitDistance(float distance);
    float getOrbitDistance() const;
};

} // namespace nbody
```

---

### Renderer

基于 OpenGL 的粒子渲染器。

```cpp
namespace nbody {

class Renderer {
public:
    Renderer();
    ~Renderer();
    
    void initialize(int width, int height);
    void cleanup();
    
    // 渲染
    void render(GLuint position_vbo, size_t particle_count);
    void render(const ParticleData* d_particles);
    
    // 相机
    void setCamera(const Camera& camera);
    Camera& getCamera();
    
    // 设置
    void setColorMode(ColorMode mode);
    void setPointSize(float size);
    void setMaxDepth(float depth);
    void setMaxVelocity(float velocity);
    
    // 窗口事件
    void onResize(int width, int height);
    
    // 信息
    int getWidth() const;
    int getHeight() const;
};

} // namespace nbody
```

---

### CudaGLInterop

CUDA-OpenGL 零拷贝互操作。

```cpp
namespace nbody {

class CudaGLInterop {
public:
    CudaGLInterop();
    ~CudaGLInterop();
    
    void initialize(size_t particle_count);
    void cleanup();
    
    // 映射/解映射
    float* mapPositionBuffer();     // 返回 CUDA 设备指针
    void unmapPositionBuffer();     // 释放给 OpenGL
    
    bool isMapped() const;
    GLuint getPositionVBO() const;
    size_t getParticleCount() const;
    
    // 从 ParticleData 更新 VBO
    void updatePositions(const ParticleData* d_particles);
};

} // namespace nbody
```

---

## 数据结构

### ParticleData

数组结构（SoA）布局，适合 GPU。

```cpp
struct ParticleData {
    // 位置
    float* pos_x;
    float* pos_y;
    float* pos_z;
    
    // 速度
    float* vel_x;
    float* vel_y;
    float* vel_z;
    
    // 当前加速度
    float* acc_x;
    float* acc_y;
    float* acc_z;
    
    // 旧加速度（用于 Verlet）
    float* acc_old_x;
    float* acc_old_y;
    float* acc_old_z;
    
    // 质量
    float* mass;
    
    // 数量
    size_t count;
};
```

**内存使用：** 每个粒子 52 字节（13 个 float × 4 字节）

---

### SimulationConfig

仿真设置配置。

```cpp
struct SimulationConfig {
    // 粒子设置
    size_t particle_count = 10000;
    InitDistribution init_distribution = InitDistribution::SPHERICAL;
    
    // 物理设置
    ForceMethod force_method = ForceMethod::DIRECT_N2;
    float dt = 0.001f;
    float G = 1.0f;
    float softening = 0.01f;
    
    // 算法专用设置
    float barnes_hut_theta = 0.5f;
    float spatial_hash_cell_size = 1.0f;
    float spatial_hash_cutoff = 2.0f;
    
    // 性能设置
    int cuda_block_size = 256;
};
```

---

### SimulationState

可序列化的仿真状态。

```cpp
struct SimulationState {
    std::vector<float> pos_x, pos_y, pos_z;
    std::vector<float> vel_x, vel_y, vel_z;
    std::vector<float> mass;
    
    size_t particle_count;
    float simulation_time;
    float dt;
    float G;
    float softening;
    ForceMethod force_method;
    
    // 序列化
    void serialize(std::ostream& out) const;
    static SimulationState deserialize(std::istream& in);
    
    bool operator==(const SimulationState& other) const;
};
```

---

## 枚举类型

### ForceMethod

可用的力计算算法。

```cpp
enum class ForceMethod {
    DIRECT_N2,      // O(N²) 精确计算
    BARNES_HUT,     // O(N log N) 树近似
    SPATIAL_HASH    // O(N) 基于网格（短程）
};
```

### InitDistribution

粒子初始分布类型。

```cpp
enum class InitDistribution {
    UNIFORM,        // 均匀盒
    SPHERICAL,      // 均匀球
    DISK            // 带旋转的扁平盘
};
```

### ColorMode

粒子着色模式。

```cpp
enum class ColorMode {
    DEPTH,          // 按深度（z 坐标）着色
    VELOCITY,       // 按速度大小着色
    DENSITY         // 按局部密度着色
};
```

---

## 工具函数

### ParticleDataManager

内存管理工具。

```cpp
namespace nbody {

class ParticleDataManager {
public:
    // 设备内存
    static void allocateDevice(ParticleData& data, size_t count);
    static void freeDevice(ParticleData& data);
    
    // 主机内存
    static void allocateHost(ParticleData& data, size_t count);
    static void freeHost(ParticleData& data);
    
    // 数据传输
    static void copyToDevice(ParticleData& d_data, const ParticleData& h_data);
    static void copyToHost(ParticleData& h_data, const ParticleData& d_data);
};

} // namespace nbody
```

### ParticleInitializer

粒子分布初始化。

```cpp
namespace nbody {

struct UniformDistParams {
    glm::vec3 min_bounds;
    glm::vec3 max_bounds;
    float min_mass = 1.0f;
    float max_mass = 1.0f;
};

struct SphericalDistParams {
    glm::vec3 center;
    float radius;
    float mass = 1.0f;
};

struct DiskDistParams {
    glm::vec3 center;
    float radius;
    float thickness;
    float rotation_speed;
    float mass = 1.0f;
};

class ParticleInitializer {
public:
    static void initUniform(ParticleData& h_data, const UniformDistParams& params,
                           unsigned int seed = 42);
    static void initSpherical(ParticleData& h_data, const SphericalDistParams& params,
                             unsigned int seed = 42);
    static void initDisk(ParticleData& h_data, const DiskDistParams& params,
                        unsigned int seed = 42);
    
    static void zeroVelocities(ParticleData& h_data);
    static void zeroAccelerations(ParticleData& h_data);
};

} // namespace nbody
```

### Serializer

状态持久化工具。

```cpp
namespace nbody {

class Serializer {
public:
    static void save(const std::string& filename, const SimulationState& state);
    static SimulationState load(const std::string& filename);
    
    static void save(std::ostream& out, const SimulationState& state);
    static SimulationState load(std::istream& in);
    
    static bool validateFile(const std::string& filename);
};

} // namespace nbody
```

### 错误处理

```cpp
namespace nbody {

// 异常类型
class CudaException : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class OpenGLException : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

class ValidationException : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

// CUDA 错误检查
#define CUDA_CHECK(call) \
    do { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            throw CudaException(cudaGetErrorString(err)); \
        } \
    } while(0)

#define CUDA_CHECK_KERNEL() \
    do { \
        cudaError_t err = cudaGetLastError(); \
        if (err != cudaSuccess) { \
            throw CudaException(cudaGetErrorString(err)); \
        } \
    } while(0)

// 验证函数
void validateSimulationConfig(const SimulationConfig& config);
void validateParticleCount(size_t count);
void validateTimeStep(float dt);
void validateSoftening(float eps);

} // namespace nbody
```

---

## 📚 相关文档

- [快速入门](./getting-started.md) - 安装和使用指南
- [架构概览](./architecture.md) - 系统设计概览
- [算法详解](./algorithms.md) - 算法说明
- [性能指南](./performance.md) - 优化策略
