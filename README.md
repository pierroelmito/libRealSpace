libRealSpace
============

Tools to parse and render Origin Systems RealSpace engine assets.

- Full TRE Support.
- Full IFF Support.
- Full PAK Support.

- Render Textures 3D Jets for all Level Of Details
- Render animation layers (individually).
- Render Map Textures.

- Guess PAK contents:
     - VOC sound effects.
     - XMidi music
     - Animations.
     - IFF
     - Images
     - Map Textures
     - Sub-pack structure

Support MacOS X, Windows and Linux 

Goal :
======

The long term goal is to be able to play Strike Commander with a Virtual Reality Headset
(Oculus Rift).

Dependencies :
==============

- [Raylib](https://www.raylib.com)
- [Open Asset Import Library (assimp)](https://github.com/assimp/assimp)
- [TinyDDSLoader](https://github.com/benikabocha/tinyddsloader)

Building :
==============

Both MSYS2 and linux builds are supported
- Install mcpp, raylib, assimp and xmake.
- Copy the original game data into data/gamefiles
- Run "xmake run"

Eye Candies :
=============

![Alt text](pics/nice_sc_scene.png)
![Alt text](/pics/F-22.png)
![Alt text](/pics/face.png)

New graphic pipeline :
======================

![Alt text](/pics/object_viewer_00.png)
![Alt text](/pics/sea_and_sky_00.png)
![Alt text](/pics/chase_00.png)

