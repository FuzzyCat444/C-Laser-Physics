# C Laser Physics

![Screenshot 1](screenshots/screenshot1.png)

![Screenshot 2](screenshots/screenshot2.png)

![Screenshot 3](screenshots/screenshot3.png)

### [-> YouTube Video](https://youtu.be/Ckl28zhAK58)

From YouTube description:

The software rendering and "physics" in this video were written entirely from scratch in C by me, Harrison (aka FuzzyCat). SDL2 was used to open the window.

This project features:
- Array of polygons to represent walls
- Dynamic line segments that reflect off polygons to represent bouncing lasers
- Collision between circular player and polygons using variable timestep
- Collision between lasers and polygons using exact ray trace
- 2.5D parallax rendering using repeated line rasterization at varying scale + polygon rasterization for the tops
- Gaussian blur filter to make lasers "glow"
- Simple python script to convert Inkscape "plain SVG" format to C code for level design

This program runs at about 500 fps on my machine without the Gaussian blur, and sinks to about 60 fps with Gaussian blur on.

## Compilation Instructions
To compile this application, create a folder called "project" or similar and place the src folder and build.bat into the folder. Install the SDL2-2.28.3 dev library for visual studio at "C:\SDL2-2.28.3" or wherever you want, updating the build.bat file to refer to the correct library directory. Copy the SDL.dll file from "C:\SDL2-2.28.3\lib\x64" into the "project" folder. Then open the "x64 Native Tools Command Prompt for VS 2022" application, navigate to the "project" folder, and run the "build.bat" file.
