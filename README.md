# BrickSim

## Planned features / TODOs
- render arrows to translate and rotate object
- render lines in another color if mesh instance is selected (add boolean to lineInstanceBuffer)
- add save functionality
- add automated build for windows
- add camera pan
- add manual
- make lines thicker (geometry shader)
- add more sophisticated search in part palette to handle queries like `title=Hello OR name=World`
- add log library, [https://github.com/emilk/loguru](https://github.com/emilk/loguru) looks interesting

## Known bugs
- If you change the layer of a part in the element properties, all instances but the selected disappear, (the changed instance is drawn over the other things correctly)

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
1. Execute the following commant to unzip glew: `unzip src/lib/glew-2.1.0.zip && mv glew-2.1.0 src/lib/glew`
1. Copy the glad includes: `sudo unzip -o src/lib/glad.zip "include/*" -d "/usr"`
1. Install some libraries: `sudo apt-get install mesa-utils freeglut3-dev libxinerama-dev libxrandr-dev libxcursor-dev libxi-dev libglew-dev libcurl4-openssl-dev libglm-dev libzip-dev`
1. Open your project with your favourite IDE (I recommend CLion). It should support CMake.
### Windows
1. Download MSYS2 from [https://www.msys2.org/](https://www.msys2.org/) and follow the installation instructions there
1. Execute the following command in a MSYS2 Shell: `pacman -S git mingw-w64-x86_64-toolchain mingw-w64-i686-toolchain base-devel mingw-w64-x86_64-cmake mingw-w64-i686-cmake libcurl-devel $(pacman -Ssq freeglut) mingw-w64-x86_64-glm mingw-w64-i686-glm mingw-w64-x86_64-libzip mingw-w64-i686-libzip`
1. Clone out this repository with `git clone --recurse-submodules -j8 git://github.com/bb1950328/BrickSim.git`
1. Unzip `src/lib/glew-2.1.0.zip` and rename the resulting folder from `glew-2.1.0` to `glew`
1. Unzip the include folder of `src/lib/glad.zip` and copy the `glad` and `KHR` folders to `C:\msys64\mingw32\include` and to `C:\msys64\mingw64\include`
1. Open your project with your favourite IDE (I recommend CLion). It should support CMake.