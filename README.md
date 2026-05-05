# Realistic-Tree-Physics-Graphics-Project
A render engine that is supposed to simulate realistic physics in reference to trees.

Windows (recommended: MSYS2)
Install MSYS2:
https://www.msys2.org/

Open MSYS2 MinGW64 terminal and run:

(bash)
pacman -Syu
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake \
          mingw-w64-x86_64-glfw mingw-w64-x86_64-glad \
          mingw-w64-x86_64-make git
          
Clone the repo:
(bash)
git clone https://github.com/RyanB464/Realistic-Tree-Physics-Graphics-Project/edit/main/README.md

Build (in the MSYS2 MinGW64 terminal)
   g++ *.cpp glad.c -I include -L lib -lglfw3 -lopengl32 -lgdi32 -o tree.exe
       
This produces an executable:
  tree.exe

Run the Simulation:
From the build directory:

./tree.exe

Key	Action:
Esc	Quit
drag mouse to move camera
wasd to move lateraly

bash
./tree_sim
