# API 参考文档

## 目录

- [核心类](#核心类)
- [数据结构](#数据结构)
- [枚举类型](#枚举类型)
- [工具函数](#工具函数)

---

## 核心类

### ParticleSystem

粒子系统主类，管理仿真的所有组件。

```cpp
namespace nbody {

class ParticleSystem {
public:
    // 构造与析构
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
    
    // 参数设置
    void setForceMethod(ForceMethod method);
    void setGravitationalConstant(float G);
    void setSofteningParameter(float eps);
    void setTimeStep(float dt);
    void setBarnesHutTheta(float theta);
    void setSpatialHashCellSize(float size);
    void setSpatialHashCutoff(float cutoff);
    
    // 参数获取
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

}
```

#### 使用示例

```cpp
// 基本使用
ParticleSystem system;
SimulationConfig config;
config.particle_count = 10000;
system.initialize(config);

// 仿真循环
while (running) {
    if (!system.isPaused()) {
        system.update(0.001f);
    }
}

// 切换算法
system.setForceMethod(ForceMethod::BARNES_HUT);

// 保存/加载状态
system.saveState("checkpoint.nbody");
system.loadState("checkpoint.nbody");
```

---

### ForceCalculator

力计算器抽象基类。

```cpp
namespace nbody {

class ForceCalculator {
public:
    virtual ~ForceCalculator() = default;
    
    // 计算所有粒子的加速度
    virtual void computeForces(ParticleData* d_particles) = 0;
    
    // 获取算法类型
    virtual ForceMethod getMethod() const = 0;
    
    // 参数设置
    void setSofteningParameter(float eps);
    void setGravitationalConstant(float G);
    
    // 参数获取
    float getSofteningParameter() const;
    float getGravitationalConstant() const;
};

}
```

#### 具体实现

##### DirectForceCalculator

```cpp
class DirectForceCalculator : public ForceCalculator {
public:
    DirectForceCalculator(int block_size = 256);
    
    void computeForces(ParticleData* d_particles) override;
    ForceMethod getMethod() const override;
    
    void setBlockSize(int size);
    int getBlockSize() const;
};
```

##### BarnesHutCalculator

```cpp
class BarnesHutCalculator : public ForceCalculator {
public:
    BarnesHutCalculator(float theta = 0.5f);
    
    void computeForces(ParticleData* d_particles) override;
    ForceMethod getMethod() const override;
    
    void setTheta(float theta);  // 开角参数 (0-1)
    float getTheta() const;
    
    BarnesHutTree* getTree();  // 访问内部树结构
};
```

##### SpatialHashCalculator

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
    
    SpatialHashGrid* getGrid();  // 访问内部网格结构
};
```

---

### Integrator

Velocity Verlet 积分器。

```cpp
namespace nbody {

class Integrator {
public:
    Integrator(int block_size = 256);
    
    // 完整积分步骤
    void integrate(ParticleData* d_particles, ForceCalculator* force_calc, float dt);
    
    // 分步操作
    void updatePositions(ParticleData* d_particles, float dt);
    void updateVelocities(ParticleData* d_particles, float dt);
    void storeOldAccelerations(ParticleData* d_particles);
    
    // 能量计算
    float computeKineticEnergy(const ParticleData* d_particles);
    float computePotentialEnergy(const ParticleData* d_particles, float G, float eps);
    float computeTotalEnergy(const ParticleData* d_particles, float G, float eps);
    
    void setBlockSize(int size);
    int getBlockSize() const;
};

}
```

#### Velocity Verlet 算法

```
1. 保存旧加速度: a_old = a
2. 更新位置: x(t+dt) = x(t) + v(t)*dt + 0.5*a(t)*dt²
3. 计算新加速度: a(t+dt) = F(x(t+dt)) / m
4. 更新速度: v(t+dt) = v(t) + 0.5*(a_old + a)*dt
```

---

### Camera

3D 相机控制器。

```cpp
namespace nbody {

class Camera {
public:
    Camera(float fov = 45.0f, float aspect = 16.0f/9.0f, 
           float near = 0.1f, float far = 1000.0f);
    
    // 变换矩阵
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;
    glm::mat4 getViewProjectionMatrix() const;
    
    // 位置设置
    void setPosition(const glm::vec3& pos);
    void setTarget(const glm::vec3& target);
    void setUp(const glm::vec3& up);
    
    // 位置获取
    glm::vec3 getPosition() const;
    glm::vec3 getTarget() const;
    glm::vec3 getForward() const;
    glm::vec3 getRight() const;
    
    // 投影设置
    void setFOV(float fov);
    void setAspectRatio(float aspect);
    void setNearFar(float near, float far);
    
    // 相机控制
    void rotate(float yaw, float pitch);
    void pan(float dx, float dy);
    void zoom(float delta);
    void orbit(float yaw, float pitch);
    void reset();
    
    void setOrbitDistance(float distance);
    float getOrbitDistance() const;
};

}
```

---

### Renderer

OpenGL 粒子渲染器。

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
    
    // 相机
    void setCamera(const Camera& camera);
    Camera& getCamera();
    
    // 渲染设置
    void setColorMode(ColorMode mode);
    void setPointSize(float size);
    void setMaxDepth(float depth);
    void setMaxVelocity(float velocity);
    
    // 窗口事件
    void onResize(int width, int height);
};

}
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
    float* mapPositionBuffer();    // 返回 CUDA 设备指针
    void unmapPositionBuffer();    // 渲染前必须调用
    
    bool isMapped() const;
    GLuint getPositionVBO() const;
    size_t getParticleCount() const;
    
    // 更新位置数据
    void updatePositions(const ParticleData* d_particles);
};

}
```

#### 使用流程

```cpp
CudaGLInterop interop;
interop.initialize(particle_count);

// 仿真循环
while (running) {
    // 更新粒子位置到 VBO
    interop.updatePositions(&d_particles);
    
    // 渲染 (VBO 已自动解映射)
    renderer.render(interop.getPositionVBO(), particle_count);
}
```

---

## 数据结构

### ParticleData

粒子数据 (Structure of Arrays 布局)。

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
    
    // 加速度
    float* acc_x;
    float* acc_y;
    float* acc_z;
    
    // 旧加速度 (Verlet 积分用)
    float* acc_old_x;
    float* acc_old_y;
    float* acc_old_z;
    
    // 质量
    float* mass;
    
    // 粒子数量
    size_t count;
};
```

### SimulationConfig

仿真配置。

```cpp
struct SimulationConfig {
    size_t particle_count = 10000;
    InitDistribution init_distribution = InitDistribution::SPHERICAL;
    ForceMethod force_method = ForceMethod::DIRECT_N2;
    float dt = 0.001f;
    float G = 1.0f;
    float softening = 0.01f;
    float barnes_hut_theta = 0.5f;
    float spatial_hash_cell_size = 1.0f;
    float spatial_hash_cutoff = 2.0f;
    int cuda_block_size = 256;
};
```

### SimulationState

仿真状态 (用于保存/加载)。

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
    
    void serialize(std::ostream& out) const;
    static SimulationState deserialize(std::istream& in);
    bool operator==(const SimulationState& other) const;
};
```

---

## 枚举类型

### ForceMethod

```cpp
enum class ForceMethod {
    DIRECT_N2,      // O(N²) 直接计算
    BARNES_HUT,     // O(N log N) 树算法
    SPATIAL_HASH    // O(N) 空间哈希
};
```

### InitDistribution

```cpp
enum class InitDistribution {
    UNIFORM,        // 均匀分布 (立方体)
    SPHERICAL,      // 球形分布
    DISK            // 圆盘分布 (星系)
};
```

### ColorMode

```cpp
enum class ColorMode {
    DEPTH,          // 按深度着色
    VELOCITY,       // 按速度着色
    DENSITY         // 按密度着色
};
```

---

## 工具函数

### 粒子数据管理

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

}
```

### 粒子初始化

```cpp
namespace nbody {

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

}
```

### 序列化

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

}
```

### 颜色映射

```cpp
namespace nbody {

struct ColorMapper {
    static glm::vec3 velocityToColor(float velocity, float max_velocity);
    static glm::vec3 depthToColor(float depth, float max_depth);
    static glm::vec3 densityToColor(float density, float max_density);
    static glm::vec3 gradientMap(float t, const glm::vec3& low, const glm::vec3& high);
};

}
```

### 错误处理

```cpp
namespace nbody {

// CUDA 错误检查宏
#define CUDA_CHECK(call) ...
#define CUDA_CHECK_KERNEL() ...

// 异常类
class CudaException : public std::runtime_error { ... };
class OpenGLException : public std::runtime_error { ... };
class ValidationException : public std::runtime_error { ... };
class ResourceException : public std::runtime_error { ... };

// 验证函数
void validateSimulationConfig(const SimulationConfig& config);
void validateParticleCount(size_t count);
void validateTimeStep(float dt);
void validateSoftening(float eps);
void validateTheta(float theta);
void validateResourceRequirements(size_t particle_count);
void checkGLError(const char* operation);

}
```
