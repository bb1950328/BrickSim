# BrickSim

## Installation Instructions
### Linux
1. Go to [CMake Linux Build](https://github.com/bb1950328/BrickSim/actions?query=branch%3Astable+workflow%3A%22CMake+Linux+Build%22), select the topmost run and download the "BrickSimLinux" artifact.
1. Unzip it somewhere
1. Open the terminal and go to the folder you unzipped the files
1. Mark the program as executable with the following command: `chmod a+x BrickSim`
1. Install some dependencies: `sudo apt install libgl1-mesa-dev libglew-dev`
1. Run the program with `./BrickSim`
### Windows
coming soon!
### OS X
currently, the only way to run BrickSim on OS X is to build it from source, because I have zero knowledge about OS X. If you know how to build it, please add a GitHub Actions workflow file.


## Planned features
- render arrows to translate and rotate object
- render lines in another color if mesh instance is selected (add boolean to lineInstanceBuffer)
- add save functionality
- add automated build for windows
- add camera pan
- make ldraw directory listing faster on windows
- add parts palette
- add manual
- make lines thicker (geometry shader)

## How to get the `codes.txt` file
The `codes.txt` file contains all parts with the colors they are available in. Because I don't whether it's allowed to distribute that file together with the program, you have to download it. But don't worry, it's really simple: 
1. Go to [https://www.bricklink.com/catalogDownload.asp](https://www.bricklink.com/catalogDownload.asp)
1. Select "Part and Color Codes". Also make sure "Tab-Delimited File" is selected.
1. Click on the download button and save the file in the BrickSim folder

## How to setup workspace for development
### Linux
1. Update your package index: `sudo apt update`
1. Install some tools for C++ development (if you haven't already): `sudo apt install build-essential`
1. Clone out this repository with `git clone --recurse-submodules -j8 git://github.com/bb1950328/BrickSim.git`
1. Execute the following commant to unzip glew: `unzip glew-2.1.0.zip && mv glew-2.1.0 glew`
1. Copy the glad includes: `sudo unzip -o glad.zip "include/*" -d "/usr"`
1. Install some libraries: `sudo apt-get install mesa-utils freeglut3-dev libxinerama-dev libxrandr-dev libxcursor-dev libxi-dev libglew-dev libcurl4-openssl-dev libglm-dev libzip-dev`
1. Open your project with your favourite IDE (I recommend CLion). It should support CMake.
### Windows
1. Download MSYS2 from [https://www.msys2.org/](https://www.msys2.org/) and follow the installation instructions there
1. Execute the following command in a MSYS2 Shell: `pacman -S git mingw-w64-x86_64-toolchain mingw-w64-i686-toolchain base-devel mingw-w64-x86_64-cmake mingw-w64-i686-cmake libcurl-devel $(pacman -Ssq freeglut) mingw-w64-x86_64-glm mingw-w64-i686-glm mingw-w64-x86_64-libzip mingw-w64-i686-libzip`
1. Clone out this repository with `git clone --recurse-submodules -j8 git://github.com/bb1950328/BrickSim.git`
1. Unzip `glew-2.1.0.zip` and rename the resulting folder from `glew-2.1.0` to `glew`
1. Unzip the include folder of `glad.zip` and copy the `glad` and `KHR` folders to `C:\msys64\mingw32\include` and to `C:\msys64\mingw64\include`
1. Open your project with your favourite IDE (I recommend CLion). It should support CMake.