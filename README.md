# FluidSim 3D

FluidSim 3D is a macOS Raylib + Metal particle simulator with water tank, gas tank, wind tunnel, imported OBJ obstacles and acoustics controls.

## Preview

### Water tank
![Water tank](docs/images/water.png)

### Gas tank
![Gas tank](docs/images/gas.png)

### Wind tunnel
![Wind tunnel](docs/images/wind.png)

### Streamlines
![Streamlines](docs/images/lines.png)

### Pathlines
![Pathlines](docs/images/smallerlines.png)

### Pressure view
![Pressure view](docs/images/pressure.png)

### Imported model
![Imported model](docs/images/f22.png)

### Acoustics / mic
![Acoustics / mic](docs/images/mic.png)

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
