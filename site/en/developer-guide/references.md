# References

Academic papers and related resources.

## Core Algorithms

### Barnes-Hut Algorithm

::: info Original Paper
Barnes, J., & Hut, P. (1986). **A hierarchical O(N log N) force-calculation algorithm**. *Nature*, 324(6096), 446-449. [DOI: 10.1038/324446a0](https://doi.org/10.1038/324446a0)
:::

### Velocity Verlet Integration

::: info Original Paper
Swope, W. C., Andersen, H. C., Berens, P. H., & Wilson, K. R. (1982). **A computer simulation method for the calculation of equilibrium constants for the formation of physical clusters of molecules: Application to small water clusters**. *The Journal of Chemical Physics*, 76(1), 637-649. [DOI: 10.1063/1.442716](https://doi.org/10.1063/1.442716)
:::

### Spatial Hashing

::: info Reference
Green, S. (2008). **Particle simulation using CUDA**. *NVIDIA Whitepaper*. [PDF](https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch29.html)
:::

## CUDA Programming

### Best Practices

- NVIDIA. **CUDA C++ Best Practices Guide**. [Documentation](https://docs.nvidia.com/cuda/cuda-c-best-practices-guide/)
- NVIDIA. **CUDA C++ Programming Guide**. [Documentation](https://docs.nvidia.com/cuda/cuda-c-programming-guide/)

### Performance Optimization

- Harris, M. **Optimizing Parallel Reduction in CUDA**. [PDF](https://developer.download.nvidia.com/assets/cuda/files/reduction.pdf)
- NVIDIA. **CUDA Memory Model**. [Documentation](https://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html#memory-model)

## OpenGL & Visualization

### Point Sprites

- OpenGL Wiki. **Point Sprites**. [Reference](https://www.khronos.org/opengl/wiki/Point_Sprite)
- NVIDIA. **Using Vertex Buffer Objects (VBOs)**. [Whitepaper](https://developer.nvidia.com/sites/default/files/akamai/opengl/files/Vertex_Buffer_Objects.pdf)

### CUDA-OpenGL Interop

- NVIDIA. **CUDA-OpenGL Interop**. [Documentation](https://docs.nvidia.com/cuda/cuda-runtime-api/group__CUDART__OPENGL.html)

## Related Projects

### N-Body Simulations

| Project | Language | Description |
|---------|----------|-------------|
| [Barnes-Hut Galaxy Simulator](https://github.com/manthey/barnes-hut) | C | Classic Barnes-Hut implementation |
| [NBodySimulator.jl](https://github.com/JuliaDynamics/NBodySimulator.jl) | Julia | Julia package for N-body |
| [Galaxy](https://github.com/damian0604/galaxy) | C++ | GPU Barnes-Hut |

### GPU Computing

| Project | Language | Description |
|---------|----------|-------------|
| [AMGX](https://github.com/NVIDIA/AMGX) | CUDA | NVIDIA's algebraic multigrid |
| [cuDF](https://github.com/rapidsai/cudf) | CUDA | GPU DataFrame library |
| [Thrust](https://github.com/NVIDIA/thrust) | CUDA | C++ template library for CUDA |

### Visualization

| Project | Language | Description |
|---------|----------|-------------|
| [Dear ImGui](https://github.com/ocornut/imgui) | C++ | Immediate mode GUI |
| [GLFW](https://www.glfw.org/) | C | Window and input handling |
| [GLEW](https://glew.sourceforge.net/) | C/C++ | OpenGL extension wrangler |

## Data Formats

### HDF5

- The HDF Group. **HDF5 Documentation**. [Reference](https://www.hdfgroup.org/solutions/hdf5/)
- Python: `h5py`. [Documentation](https://docs.h5py.org/)

## Books

- **Numerical Recipes** by Press, Teukolsky, Vetterling, and Flannery
  - Chapter on N-body methods and numerical integration
- **GPU Gems 3** by Nguyen (ed.)
  - Chapter 29: Particle Simulation using CUDA
- **The Art of Molecular Dynamics Simulation** by Rapaport
  - Detailed treatment of spatial hashing and short-range forces

## Citation

If you use this project in your research, please cite:

```bibtex
@software{nbody2026,
  title = {N-Body: Million-Particle GPU Physics Engine},
  author = {AICL-Lab},
  year = {2026},
  url = {https://github.com/AICL-Lab/n-body},
  version = {2.1.0},
  note = {CUDA-accelerated N-body simulation with real-time visualization}
}
```
