# FluidSim 3D

FluidSim 3D is a macOS Raylib + Metal particle simulator with water tank, gas tank, wind tunnel, imported OBJ obstacles and acoustics controls.

## Screenshots

- Water Tank: [docs/images/water.png](docs/images/water.png)
- Gas Tank: [docs/images/gas.png](docs/images/gas.png)
- Wind Tunnel: [docs/images/wind.png](docs/images/wind.png)
- Streamlines: [docs/images/lines.png](docs/images/lines.png)
- Pathlines: [docs/images/smallerlines.png](docs/images/smallerlines.png)
- Pressure View: [docs/images/pressure.png](docs/images/pressure.png)
- Imported Model: [docs/images/f22.png](docs/images/f22.png)
- Acoustics / Mic: [docs/images/mic.png](docs/images/mic.png)

## How It Works

The app runs a 3D SPH solver with a uniform grid for neighbor search and CPU or Metal GPU simulation backends for density, force, and integration passes. Rendering stays in Raylib, with particles, slices, streamlines, pathlines, tank geometry, and imported OBJ previews driven by the same simulation state, while imported models collide through a voxelized signed-distance field.

## Build

```bash
make
```

## Run

```bash
make run
```
