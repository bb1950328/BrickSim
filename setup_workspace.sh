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
  if [ "$1" = "only32bit" ]
  then
    install_64bit_packages=false
  fi
  if [ "$1" = "only64bit" ]
  then
    install_32bit_packages=false
  fi
fi

#installing dependencies
if [[ "$OS" == "linux" ]]; then
  echo "Installing packages using apt-get..."
  sudo apt-get update
  sudo apt-get install -y build-essential gcc-10 g++-10 gcc-multilib g++-multilib gcc-10-multilib g++-10-multilib \
                          cmake mesa-utils libxinerama-dev libxrandr-dev libxcursor-dev libxi-dev ninja-build

  if $install_32bit_packages; then
    sudo apt-get install -y libgl1-mesa-dev:i386 libglu1-mesa-dev:i386 freeglut3-dev:i386 libzstd-dev:i386  \
                            zlib1g-dev:i386 libssl-dev:i386 libcurl4-openssl-dev:i386 libtbb-dev:i386 \
                            libbz2-dev:i386 liblzma-dev:i386
  fi
  if $install_64bit_packages; then
    # there are probably more dependencies but i didn't have a 32 bit pc to try it out
    sudo apt-get install -y freeglut3-dev:amd64 libcurl4-openssl-dev:amd64 libtbb-dev:amd64
  fi
  echo "packages installed."
fi

if [[ "$OS" == "windows" ]]; then
  echo "Installing packages using pacman..."
  pacman -S unzip base-devel msys2-devel libcurl-devel --noconfirm --needed
  if $install_32bit_packages; then
      pacman -S mingw-w64-i686-toolchain mingw-w64-i686-cmake mingw-w64-i686-freeglut --noconfirm --needed
  fi
  if $install_64bit_packages; then
      pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake mingw-w64-x86_64-freeglut --noconfirm --needed
  fi
  echo "packages installed."
fi

if [[ "$OS" == "mac" ]]; then
  if ! which -s brew; then
    echo "installing homebrew"
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
  else
    echo "brew already installed."
  fi

  brew install openssl ninja
fi
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
