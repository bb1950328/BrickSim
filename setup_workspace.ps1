vcpkg install freeglut zlib eigen3 libccd

Expand-Archive 'src\lib\glad.zip' 'glad_tmp'
Move-Item -Path 'glad_tmp\src\glad.c' -Destination 'src\lib\glad.c'
Move-Item -Path 'glad_tmp\include' -Destination 'src\lib\include'
Remove-Item -Recurse -Force 'glad_tmp'
