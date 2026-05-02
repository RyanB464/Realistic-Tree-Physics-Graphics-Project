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
git clone https://github.com/YOUR_REPO_HERE
cd YOUR_REPO_HERE

The project uses a simple CMake build.

Build (all platforms)
(bash)
mkdir build
cd build
cmake ..
cmake --build .
This produces an executable:
  tree.exe

Run the Simulation:
From the build directory:

(bash)
./tree.exe

Key	Action:
Esc	Quit

bash
./tree_sim
