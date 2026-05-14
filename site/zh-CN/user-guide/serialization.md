# 序列化

保存和加载模拟状态。

## 二进制格式

```cpp
// 保存
system.saveState("checkpoint.nbody");

// 加载
system.loadState("checkpoint.nbody");
```

## HDF5 格式

```cpp
// 导出
system.exportHDF5("simulation.h5");

// 导入
system.importHDF5("simulation.h5");
```

## HDF5 结构

```
simulation.h5
├── /particles/
│   ├── positions (Nx3 float)
│   ├── velocities (Nx3 float)
│   └── masses (Nx float)
├── /metadata/
│   ├── config (struct)
│   └── timestamp (int64)
```

## Python 分析

```python
import h5py

with h5py.File('simulation.h5', 'r') as f:
    pos = f['/particles/positions'][:]
    vel = f['/particles/velocities'][:]
    mass = f['/particles/masses'][:]
```
