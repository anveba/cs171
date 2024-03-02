Renderer made for CS171 at the California Institute of Technology (Caltech) using OpenGL (GLUT/glew).

The renderer can render scenes using a software renderer or OpenGL. Scenes rendered with the software renderer are outputted as PPM to stdout. Scenes rendered with OpenGL can be interacted with using an arcball. A normal mapping demo is also included and is rendered with OpenGL.

The number keys can be pressed to smooth the meshes in the scene (using implicit fairing). A higher number will smooth the meshes more. Smoothing only works on meshes without boundaries (closed surfaces) and may crash if it is used on other meshes. 

An animation implementation is included in the source code but is currently unavailable to interact with.
