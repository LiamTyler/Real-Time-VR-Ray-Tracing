# Real Time VR Ray Tracing

## Description
There are a few goals for this project, prioritized in the following order:
1. Use OpenGL's compute shader to do ray tracing on the GPU.
2. Be able to move around a static scene in real time.
3. Render the scene twice with two slightly offset cameras in real time to do VR
4. Use the VIVE to actually make it VR

## Installation / Usage
- Make sure you have SDL2, OpenGL, GLEW, and GLM installed with system header file, or tweak the includes
- Just download the repository and type 'make' in the main directory
- To run the program, be in the main directory and type 'make run' or './build/bin/proj'
- Note: Will not work on Mac computers because OpenGL compute shaders require OpenGL 4.3 or higher, and Mac only goes up to 4.1
