#!/usr/bin/env bash

# cloning git submodules if not already done
git submodule update --init --recursive


# checking working directory
if [[ ! -d src ]]; then
  echo "execute this script in the BrickSim repository root!" >&2
  exit 1
fi
######################################################

# Detecting OS
unameValue="$(uname -s)"
case "${unameValue}" in
    Linux*)           OS="linux"; echo "linux detected";;
    Darwin*)          OS="mac"; echo "macOS detected";;
    CYGWIN* | MINGW* | MSYS*) OS="windows"; echo "MinGW/MSYS or cygwin detected";;
    *)                echo "unknown OS: ${unameValue}" >&2; exit 2;;
esac
#######################################################

#installing dependencies
if [[ "$OS" == "linux" ]]; then
  echo "Installing packages using apt-get..."
  sudo apt-get update
  sudo apt-get install build-essential mesa-utils freeglut3-dev libxinerama-dev libxrandr-dev libxcursor-dev libxi-dev libglew-dev libcurl4-openssl-dev libglm-dev libzip-dev gcc-10 g++-10 libspdlog-dev
  sudo apt-get install libzip5
  echo "packages installed."
fi

if [[ "$OS" == "windows" ]]; then
  echo "Installing packages using pacman..."
  pacman -S unzip --noconfirm --needed
  pacman -S mingw-w64-x86_64-toolchain mingw-w64-i686-toolchain --noconfirm --needed
  pacman -S base-devel mingw-w64-x86_64-cmake mingw-w64-i686-cmake --noconfirm --needed
  pacman -S libcurl-devel --noconfirm --needed
  pacman -S "$(pacman -Ssq freeglut)" --noconfirm --needed
  pacman -S mingw-w64-x86_64-glm mingw-w64-i686-glm --noconfirm --needed
  pacman -S mingw-w64-x86_64-libzip mingw-w64-i686-libzip --noconfirm --needed
  pacman -S mingw-w64-x86_64-spdlog --noconfirm --needed
  #pacman -S openssl-devel # todo find out if these are needed
  #pacman -S mingw-w64-openssl
  #pacman -S mingw-w64-x86_64-openssl
  echo "packages installed."
fi

if [[ "$OS" == "mac" ]]; then
  if hash brew 2>/dev/null; then
    echo "installing homebrew"
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
  else
    echo "brew already installed."
  fi

  brew install glm
fi
###########################################

#unzipping libraries
rm -rf src/lib/glew
unzip src/lib/glew-2.1.0.zip | grep -v inflating:
mv glew-2.1.0 src/lib/glew

rm -rf src/lib/rapidjson
unzip src/lib/rapidjson.zip | grep -v inflating:
mv rapidjson src/lib/rapidjson


unzip -o -j "src/lib/glad.zip" "src/glad.c" -d "src/lib"

rm -rf src/lib/include
mkdir "src/lib/include"
cd src/lib/include || exit 3
unzip ../glad.zip
cp -r include/glad .
cp -r include/KHR .
rm -rf src include
cd ../../..
###########################################

#symlinking LICENSE to make WiX accept it
ln -f LICENSE LICENSE.txt
