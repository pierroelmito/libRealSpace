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

- [GLFW](https://www.glfw.org/)
- [Sokol GFX/Audio](https://github.com/floooh/sokol)
- [HandmadeMath](https://github.com/HandmadeMath/Handmade-Math)
- [TinyDDSLoader](https://github.com/benikabocha/tinyddsloader)

Building :
==============

- MSYS2 and linux are both working. GLFW and xmake need to be installed first, then just run "xmake"
- Copy or link the "assets" directory in the game directory.

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

