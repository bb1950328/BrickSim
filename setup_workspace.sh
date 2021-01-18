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
OS="unknown"

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  echo "linux detected"
  OS="linux"
fi

if [[ "$OSTYPE" == "darwin"* ]]; then
  echo "macOS detected"
  OS="mac"
fi

if [[ "$OSTYPE" =~ ^(cygwin|msys)$ ]]; then
  echo "MinGW/MSYS or cygwin detected"
  OS="windows"
fi

if [[ "$OS" == "unknown" ]]; then
  echo "unknown operating system, exiting" >&2
  exit 2
fi
#######################################################

#installing dependencies
if [[ "$OS" == "linux" ]]; then
  echo "Installing packages using apt-get..."
  sudo apt-get update
  #sudo apt-get install mesa-utils freeglut3-dev libxinerama-dev libxrandr-dev libxcursor-dev libxi-dev libglew-dev libcurl4-openssl-dev libglm-dev libzip-dev gcc-10 g++-10 libglew xorg-dev ## glew-utils libxrandr-dev
  sudo apt-get install mesa-utils freeglut3-dev libxinerama-dev libxrandr-dev libxcursor-dev libxi-dev libglew-dev libcurl4-openssl-dev libglm-dev libzip-dev gcc-10 g++-10
  echo "packages installed."
fi

if [[ "$OS" == "windows" ]]; then
  echo "Installing packages using pacman..."
  pacman -S unzip --noconfirm
  pacman -S libcurl-devel --noconfirm
  pacman -S "$(pacman -Ssq freeglut)" --noconfirm
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
unzip src/lib/glew-2.1.0.zip
mv glew-2.1.0 src/lib/glew

rm -rf src/lib/rapidjson
unzip src/lib/rapidjson.zip
mv rapidjson src/lib/rapidjson

unzip -j "src/lib/glad.zip" "src/glad.c" -d "src/lib"
unzip -o src/lib/glad.zip "include/*" -d "src/lib"
