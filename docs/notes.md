Gist
====

* Flat terrain plus objects and characters
* PC and NPC characters
* Diablo-like controls
* Simple multiplayer
* Free art

Characters
----------

* Animated 3D models with some (non-blended) animations

World
-----

* Tiled flat terrain with 2D coordinates
* Simple objects with collision bodies

Simulation
----------

* Deterministic
* Lock-step

Network
-------

* Input only
* Every client simulates the entire world

Graphics
--------

* Simple. OpenGL-based
* Texturised models
* Shadow mapping
* Several simple materials

Sound
-----

* Simple but 3D
* No music

GUI
---

* Minimalistic. Debug-like (quite-likely ImGui)

Architecture
------------

* Target platforms: PC + Browser
* Third party
    * GLFW
    * Something to load models (GLTF?, Collada?, Assimp?)
* Network ???
* Sound ???