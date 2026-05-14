# 粒子分布

初始粒子配置。

## 内置分布

| 分布 | 描述 | 用途 |
|------|------|------|
| `SPHERICAL` | 均匀球体 | 星系团 |
| `DISK` | 薄旋转盘 | 旋涡星系 |
| `CUBE` | 均匀立方体 | 通用测试 |
| `RANDOM` | 随机位置 | 压力测试 |

## 自定义分布

```cpp
ParticleData data;
data.resize(n);

for (size_t i = 0; i < n; ++i) {
    // 手动设置位置、速度、质量
    data.position_x[i] = /* 自定义 */;
    data.velocity_x[i] = /* 自定义 */;
    data.mass[i] = /* 自定义 */;
}

ParticleSystem system;
system.initialize(data, method);
```
