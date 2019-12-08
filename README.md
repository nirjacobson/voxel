# Voxel
Voxel is an engine demo that allows you to paint with volumetric pixels, or voxels. It’s similar to the original MS Paint program in a 3D, Minecraft-esque environment. It has an infinite draw area that is saved to a file as it is explored.

![Voxel](https://nirjacobson.com/wp-content/uploads/2019/12/voxel.png)

There is a branch for the Raspberry Pi 3 and the Raspberry Pi 4.

## Dependencies

Voxel depends on the [cairo](https://www.cairographics.org/) graphics library. [GLFW](https://www.glfw.org/) is also a dependency on the Raspberry Pi 4.

```
sudo apt install libcairo2-dev
sudo apt install libglfw3-dev    # (Raspberry Pi 4 only)
```

## Installation

```
git clone https://github.com/nirjacobson/voxel.git
cd voxel/
make
./voxel
```

## Controls
Use **W**, **A**, **S**, **D**, **↑**, **↓**, **←**, **→** to navigate.

**TAB** toggles the adjacency mode of the picker. The picker can operate on blocks or the positions adjacent to them.  
**ESC** exits the program.

The picker tools have the following hotkeys:
- **1** - Pencil
- **2** - Eraser
- **Q** - Eyedropper
- **Z** - Region select
- **X** - Stamp (copy selection)
- **C** - Move (cut selection)
  
The **pencil**, **eraser** and **select** tools support click-and-drag.  
When placing a selection with the **stamp** or **move** tools, right-click will rotate the target.  
With the **select** tool, **SHIFT+Click** adds to an existing selection.
