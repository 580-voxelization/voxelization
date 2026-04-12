# Stylized Voxel Renderer: Option 1 TODO Checklist

This checklist is based on the current framework audit.
Goal: get the project to a state where two voxelization methods can be implemented and compared fairly.

## Priority 0: Critical blockers

- [ ] Create a shared `VoxelGrid` representation
  Acceptance:
  - Stores grid origin / bounds in world space
  - Stores grid resolution `(nx, ny, nz)`
  - Stores voxel size or derived cell spacing
  - Stores occupancy in one consistent format for both methods
  Why:
  - Right now `VoxelizedMesh` only has `voxel_size` and `std::vector<vec3> voxels`, which is not enough for fair A/B comparison.

- [ ] Create a shared `VoxelizationConfig`
  Acceptance:
  - One place defines mesh input, grid resolution, and bounds policy
  - Both methods consume the exact same config
  Why:
  - Without this, Method A and Method B can silently voxelize different domains.

- [ ] Add explicit scene normalization / bounding box handling
  Acceptance:
  - Mesh bounds are computed once
  - Grid bounds are chosen once
  - Both methods voxelize against the same normalized space or same world-space bounds
  Why:
  - This is required for a fair comparison.

- [ ] Replace single `Voxelize()` logic with a clean multi-method interface
  Acceptance:
  - At least two method implementations exist
  - Both return the same `VoxelGrid` type
  - Method choice is selectable without rewriting rendering code
  Why:
  - The current design only leaves room for one ad hoc voxelizer inside `VoxelizedMesh`.

- [ ] Separate voxelization from rendering
  Acceptance:
  - Voxelizers produce data only
  - Renderer consumes voxel data only
  - Rendering differences are not entangled with algorithm differences
  Why:
  - Otherwise your compare/contrast results will be contaminated by different render paths.

## Priority 1: Required for Option 1 evaluation

- [ ] Make Method A and Method B plug into the same pipeline
  Acceptance:
  - Same mesh input
  - Same bounds
  - Same grid resolution
  - Same camera
  - Same shaders / lighting / render settings
  Why:
  - This is the minimum bar for a fair compare/contrast section.

- [ ] Add voxelization timing output
  Acceptance:
  - Log voxelization time per method
  - Time voxelization separately from rendering
  Why:
  - Runtime is one of the required comparison axes.

- [ ] Add memory / occupancy reporting
  Acceptance:
  - Log occupied voxel count
  - Log total grid size
  - Log simple memory estimate or actual storage usage
  Why:
  - Memory cost is part of the required comparison.

- [ ] Add a consistent experiment runner path
  Acceptance:
  - Can run the same scene with Method A or Method B by changing only a method selector
  - Output filenames or logs clearly identify the method and resolution
  Why:
  - Manual, inconsistent testing will make the final comparison weak.

- [ ] Define observable comparison criteria
  Acceptance:
  - Correctness: visual agreement with the source mesh or expected occupancy
  - Visual quality: silhouettes / appearance under the same render settings
  - Artifacts: holes, overfill, missing parts
  - Thin-feature preservation: visibly noted on suitable models
  Why:
  - If you do not define the criteria now, the report will become hand-wavy later.

## Priority 2: Debugging support

- [ ] Add artifact debugging support
  Acceptance:
  - Can inspect missing voxels / holes
  - Can inspect overfilled regions
  - Can inspect thin features
  Why:
  - These are core failure modes for voxelization methods.

- [ ] Add at least one direct voxel debug output
  Good options:
  - Occupancy slice dump
  - Voxel count per layer
  - Wireframe mesh over voxel render
  - Side-by-side output image per method
  Why:
  - You need evidence when one method fails.

- [ ] Finish bounding-box support used by voxel rendering
  Acceptance:
  - `Bounding_Box` / `Box` code is functional for voxel objects
  Why:
  - The current base framework still has unfinished box-related pieces.

## Priority 3: Rendering and presentation

- [ ] Finish `VoxelizedMesh` rendering path
  Acceptance:
  - `Intersection`, `Normal`, and `Bounding_Box` work for voxel output
  - Voxelized result can be rendered reproducibly
  Why:
  - Right now the voxel object is only a stub.

- [ ] Decide what "stylized" means in implementation terms
  Acceptance:
  - One concrete stylized look is chosen
  - The style is applied equally to both methods
  Why:
  - Stylization is part of the project, but it must not interfere with fair comparison.

- [ ] Lock a fixed comparison camera setup
  Acceptance:
  - At least one standard view per benchmark model
  - Same view used for both methods
  Why:
  - Consistent images make method differences visible.

## Priority 4: Benchmark coverage

- [ ] Add benchmark meshes beyond `hedron.obj`
  Minimum suggested set:
  - One simple closed solid
  - One model with thin parts
  - One model with more geometric complexity
  Why:
  - The current single sample mesh is too weak for evaluating artifacts and thin-feature preservation.

- [ ] Pick 2-3 fixed grid resolutions for comparison
  Acceptance:
  - The same resolutions are used for both methods on all benchmark models
  Why:
  - Resolution changes can hide or exaggerate method differences.

## De-scope / avoid spending time here for now

- [ ] Do not prioritize hierarchy acceleration unless voxel comparison is already working end to end
- [ ] Do not prioritize reflective or extra lighting effects over voxelization and comparison infrastructure
- [ ] Do not build a large interaction system unless the professor explicitly asked for it
- [ ] Do not over-engineer a full scene system before the A/B pipeline works

## Suggested implementation order

- [ ] 1. Shared `VoxelGrid`
- [ ] 2. Shared `VoxelizationConfig`
- [ ] 3. Bounds / normalization
- [ ] 4. Method interface
- [ ] 5. Method A
- [ ] 6. Method B
- [ ] 7. One common voxel renderer
- [ ] 8. Timing / memory / occupancy logging
- [ ] 9. Artifact debugging outputs
- [ ] 10. Benchmark scenes and fixed comparison runs

## Current framework status snapshot

- [x] Mesh loading exists
- [x] Scene parser exists
- [x] Camera basics exist
- [x] Generic renderer exists
- [ ] Shared voxel grid does not exist yet
- [ ] Two-method abstraction does not exist yet
- [ ] Fair comparison pipeline does not exist yet
- [ ] Performance logging does not exist yet
- [ ] Artifact debugging support does not exist yet
- [ ] `VoxelizedMesh` implementation is not complete yet
