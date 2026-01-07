# Godot Engine

<p align="center">
  <a href="https://godotengine.org">
    <img src="logo_outlined.svg" width="400" alt="Godot Engine logo">
  </a>
</p>

## Overview

**This project is a customized version of the [Godot Engine](https://godotengine.org) 4.5.1, designed to enhance the 2D physics APIs and introduce new joint nodes.** While this repository focuses on extending the engine's capabilities, the actual physics implementation is intended to be powered by the [Rapier Extension for Godot](https://github.com/appsinacup/godot-rapier-physics).

## Compiling from source

[See the official 4.5 docs](https://docs.godotengine.org/en/4.5/engine_details/development/compiling/index.html)
for compilation instructions for every supported platform.

Windows build example. C# enabled and 3D disabled.

```bash
# Build Godot binary
scons platform=windows target=editor disable_3d_physics=yes disable_modules="webp,theora,mobile_vr" module_mono_enabled=yes

# Generate C# glue sources
bin/godot.windows.editor.x86_64.mono --headless --generate-mono-glue modules/mono/glue

# Build .NET assemblies
python ./modules/mono/build_scripts/build_assemblies.py --godot-output-dir=./bin --godot-platform=windows

# Generate extension api
bin/godot.windows.editor.x86_64.mono --headless --dump-extension-api

# Generate doc xml files
bin/godot.windows.editor.x86_64.mono --doctool
```
