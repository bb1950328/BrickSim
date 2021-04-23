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

install_32bit_packages=true
install_64bit_packages=true

if [[ $# -ge 1 ]]
then
  if [[ "$1" -eq 'only32bit' ]]
  then
    install_64bit_packages=false
  fi
  if [[ "$1" -eq 'only64bit' ]]
  then
    install_32bit_packages=false
  fi
fi

#installing dependencies
if [[ "$OS" == "linux" ]]; then
  echo "Installing packages using apt-get..."
  sudo apt-get update
  sudo apt-get install -y build-essential mesa-utils freeglut3-dev libxinerama-dev \
                          libxrandr-dev libxcursor-dev libxi-dev libcurl4-openssl-dev \
                          gcc-10 g++-10 cmake libtbb-dev
  echo "packages installed."
fi

if [[ "$OS" == "windows" ]]; then
  echo "Installing packages using pacman..."
  pacman -S unzip base-devel msys2-devel libcurl-devel --noconfirm --needed
  if [ "$install_32bit_packages" ]; then
      pacman -S mingw-w64-i686-toolchain mingw-w64-i686-cmake mingw-w64-i686-freeglut --noconfirm --needed
  fi
  if [ "$install_64bit_packages" ]; then
      pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake mingw-w64-x86_64-freeglut --noconfirm --needed
  fi
  #pacman -S "$(pacman -Ssq freeglut)" --noconfirm --needed
  #pacman -S openssl-devel # todo find out if these are needed
  #pacman -S mingw-w64-openssl
  #pacman -S mingw-w64-x86_64-openssl
  echo "packages installed."
fi

#if [[ "$OS" == "mac" ]]; then
#  which -s brew
#  if [[ $? != 0 ]] ; then
#    echo "installing homebrew"
#    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
#  else
#    echo "brew already installed."
#  fi
#
#  brew install glm
#fi
###########################################

#unzipping libraries
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
