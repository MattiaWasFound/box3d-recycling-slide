# A box slides off a turning platform when contact recycling is on

A small, standalone reproduction for [Box3D](https://github.com/erincatto/box3d), pinned to upstream
`main` at `5d6364d` (Static query, #214).

## What it does

A kinematic platform (8 × 0.5 × 3 m) spins about the vertical axis at a constant rate. A 25 kg,
0.5 m box rests on it, 1.75 m from the axis, starting with the platform's surface velocity. With
friction it should turn with the platform and stay in place in the platform's frame.

With contact recycling on (the default), the box slides steadily outward. With recycling off on the
platform (`b3BodyDef::enableContactRecycling = false`), it stays put.

## Results (macOS arm64, Release, 60 Hz, 4 substeps)

| Spin rate | Recycling | Distance moved in the platform frame over 60 s |
|---|---|---|
| 0.12 rad/s | on | 153.6 cm (radius 1.76 m → 3.29 m) |
| 0.12 rad/s | off | 1.3 cm |
| 0.06 rad/s | on | 64.7 cm |
| 0.06 rad/s | off | 0.3 cm |

The slide grows with the spin rate and with the distance from the axis, and there is none at the
centre. In an earlier, longer investigation on a dynamic platform it did not change with the time
step (60 or 120 Hz), the substep count (1, 4 or 16), warm starting, friction (0.6 or 1), the box's
mass (1 to 500 kg) or SIMD.

## Build and run

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/repro            # prints the four runs
./build/repro slide.b3rec  # also saves a Box3D recording of the first run
```

CMake fetches Box3D at the pinned commit. `slide.b3rec` in this repository is that recording.

## Likely cause

In `src/physics_world.c`, the "Contact recycling optimization" block reuses a manifold while the
relative pose of the two bodies stays within the recycle distance and neither body has turned more
than about 10° (`B3_CONTACT_RECYCLE_ANGULAR_DISTANCE`) since the manifold was computed. For each
point it updates only the separation ("Keep anchors but update separation"). The anchors, normal and
tangents stay in the world orientation they were computed in.

A box resting on a turning platform keeps a constant pose relative to it while both bodies turn, so
the manifold is recycled until the 10° limit, and the anchors are up to 10° out of date. Friction
then holds the box to the platform's velocity at a point that far behind on the arc. With a mean lag
of about 5°, this predicts an outward slip of about Ω · r · sin 5°. That matches the measured slip
on the dynamic platform.

## License

MIT
