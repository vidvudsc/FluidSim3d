# FluidSim 3D Roadmap

## Bake-Only Quality

- Add bake obstacle quality presets for imported OBJ SDFs: keep real-time SDFs low resolution, but rebuild imported obstacles at higher SDF resolutions for bakes.
- Add bake-only obstacle coupling controls: stronger push-out iterations, tighter surface offsets, near-wall damping, and optional no-slip wall behavior.
- Add open-mesh handling for OBJ imports by thickening non-watertight surfaces instead of relying only on inside/outside SDF signs.

## Performance

- Replace CPU-side particle billboard drawing with GPU-instanced particle rendering.
- Avoid per-frame CPU depth sorting when possible by using weighted blended order-independent transparency or fixed-depth buckets.
- Keep preview playback renderable by using deterministic visible-particle subsets and cached GPU-friendly frame buffers.
- Move grid prefix sums fully onto Metal to remove the CPU round trip between cell counting and scatter.
- Fuse GPU kernels where practical to reduce Metal command-buffer overhead during high-particle bakes.
- Add lightweight timing breakdowns for simulation, rendering, diagnostics, bake cache writes, and UI.
