# Serialization

Save and load simulation states.

## Binary Format

```cpp
// Save
system.saveState("checkpoint.nbody");

// Load
system.loadState("checkpoint.nbody");
```

## HDF5 Format

```cpp
// Export
system.exportHDF5("simulation.h5");

// Import  
system.importHDF5("simulation.h5");
```

## HDF5 Structure

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

## Python Analysis

```python
import h5py

with h5py.File('simulation.h5', 'r') as f:
    pos = f['/particles/positions'][:]
    vel = f['/particles/velocities'][:]
    mass = f['/particles/masses'][:]
```
