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

install_32bit_packages=false

if [[ $# -ge 1 ]]
then
  if [ "$1" = "with32bit" ]
  then
    install_32bit_packages=true
  fi
fi

#installing dependencies
if [[ "$OS" == "linux" ]]; then
  echo "Installing packages using apt-get..."
  sudo apt-get update
  sudo apt-get install -y build-essential gcc-12 g++-12 gcc-multilib g++-multilib gcc-12-multilib g++-12-multilib \
                          cmake mesa-utils libxinerama-dev libxrandr-dev libxcursor-dev libxi-dev ninja-build \
                          freeglut3-dev libcurl4-openssl-dev libtbb-dev libssl-dev libeigen3-dev libccd-dev \
                          zlib1g-dev libwayland-dev libxkbcommon-dev

  if $install_32bit_packages; then
    sudo apt-get install -y libgl1-mesa-dev:i386 libglu1-mesa-dev:i386 freeglut3-dev:i386 libzstd-dev:i386  \
                            zlib1g-dev:i386 libssl-dev:i386 libcurl4-openssl-dev:i386 libtbb-dev:i386 \
                            libbz2-dev:i386 liblzma-dev:i386 mesa-common-dev:i386
  fi
  echo "packages installed."
fi

if [[ "$OS" == "windows" ]]; then
  echo "Installing packages using pacman..."
  pacman -S unzip base-devel msys2-devel libcurl-devel mingw-w64-x86_64-toolchain mingw-w64-x86_64-cmake \
            mingw-w64-x86_64-freeglut --noconfirm --needed
  if $install_32bit_packages; then
      pacman -S mingw-w64-i686-toolchain mingw-w64-i686-cmake mingw-w64-i686-freeglut --noconfirm --needed
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

  brew install openssl ninja libccd eigen
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
