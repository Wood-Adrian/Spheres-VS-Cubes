# Spheres VS Cubes
## A 3D Tower-Defence-Style Wii Game
A basic game template made for the Nintendo Wii. Consists of building towers of cubes that shoot at the spheres travelling along the set path on the map.
## Controls

### Main Menu

A - Select

B - Deselect

Left Stick / Nunchuck Stick - Move Up/Down Menu

### In-Game

A - Select Tower / Build Cube (when tower selected) / Start New Round

B - Deselect Tower

1 - Move Up Tower

2 - Move Down Tower

C / Left Trigger - Move Camera Up

Z / Right Trigger - Move Camera Down

Nunchuck Stick / Left Stick - Move Camera

Wiimote D-Pad / Right Stick - Rotate Camera

Home Button - (Un)Pause Game / Quit Game on Win/Pause/Lose Screen

## About This Project
This project began in June of 2024 and continued until August. The purpose of this project was to, aswell as learn how Wii homebrew games are created, learn C++ in an applied project. I learned about the Wii's graphical engine (GX) and how to use it (with the help of GRRLIB), as well as critical C++ concepts, such as header files, smart pointers, and vectors (C++'s list structure).

This project has been made available to view to the public for those wishing to learn Wii homebrew development, so they may read this project and potentially learn something new.

## Building
### Requirements
- [devkitPro](https://github.com/devkitPro/installer)
- [GRRLIB](https://github.com/GRRLIB/GRRLIB)

### Development Environment
I used Visual Studio 2022 with the following project include directories:
```
path\to\your\devkitPro\libogc\include
path\to\your\GRRLIB\GRRLIB-4.5.1\GRRLIB\GRRLIB
path\to\current\project\build
path\to\current\project\include
```
### Build
Before building, ensure that you have devkitPro (including devkitPPC) and GRRLIB installed (i.e., devkitPPC available in the environment, and GRRLIB installed inside `devkitPro\portlibs\wii\`).
Run the Makefile provided at the root of this project (`make` inside of the console). A .elf and .dol file will be created in the root of the project. Rename the .dol file to `boot.dol` and place it inside of a folder inside the \apps\ folder of your wii's sd card (`sd:\apps\exampleFolder\boot.dol`). This can then be ran through the Homebrew Channel.

Note that in its current form, this project requires extra files provided through the releases page, otherwise elements of the game will not work (or the game will crash, i haven't tested it).

## Credits / Tools Used
- [devkitPro](https://devkitpro.org/)
- [GRRLIB](https://github.com/GRRLIB/GRRLIB)
- [Nlohmann's JSON for Modern C++](https://json.nlohmann.me/)
- [Three Hundred Mechanics](https://www.squidi.net/three/index.php)
