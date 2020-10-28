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


## TODO
- render arrows to translate and rotate object
- add save functionality
- optimize conditional lines (use EBO, geometry shader)
- add automated build for windows
- add camera pan
- make ldraw directory listing faster on windows
- add parts palette
- add manual
- make lines thicker