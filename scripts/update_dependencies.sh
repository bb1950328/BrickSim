#!/usr/bin/env bash

# https://gist.github.com/lukechilds/a83e1d7127b78fef38c2914c4ececc3c
get_latest_release() {
  curl --silent "https://api.github.com/repos/$1/releases/latest" | # Get latest release from GitHub api
    grep '"tag_name":' |                                            # Get tag line
    sed -E 's/.*"([^"]+)".*/\1/'                                    # Pluck JSON value
}

echo -e "\033[0;32mINFO: Updating Catch2\033[0m"
cd src/lib/Catch2 || exit 1
git fetch --tags
git checkout "$(get_latest_release catchorg/Catch2)"

echo -e "\033[0;32mINFO: Updating cpuinfo\033[0m"
cd ../cpuinfo || exit 1
git checkout main
git pull

echo -e "\033[0;32mINFO: Updating curl\033[0m"
cd ../curl || exit 1
git fetch --tags
git checkout "$(get_latest_release curl/curl)"

echo -e "\033[0;32mINFO: Updating earcut\033[0m"
cd ../earcut.hpp || exit 1
git fetch --tags
git checkout "$(get_latest_release mapbox/earcut.hpp)"
git submodule update --recursive

echo -e "\033[0;32mINFO: Updating efsw\033[0m"
cd ../efsw || exit 1
git checkout master
git pull

echo -e "\033[0;32mINFO: Updating fast_float\033[0m"
cd ../fast_float || exit 1
git fetch --tags
git checkout "$(get_latest_release fastfloat/fast_float)"

echo -e "\033[0;32mINFO: Updating glfw\033[0m"
cd ../glfw || exit 1
git checkout master
git pull

echo -e "\033[0;32mINFO: Updating glm\033[0m"
cd ../glm || exit 1
git fetch --tags
#git checkout "$(get_latest_release g-truc/glm)"
git checkout master
git pull

echo -e "\033[0;32mINFO: Updating IconFontCppHeaders\033[0m"
cd ../IconFontCppHeaders || exit 1
git checkout main
git pull

echo -e "\033[0;32mINFO: Updating imgui\033[0m"
cd ../imgui || exit 1
git checkout docking
git pull

echo -e "\033[0;32mINFO: Updating libzip\033[0m"
cd ../libzip || exit 1
git fetch --tags
git checkout "$(get_latest_release nih-at/libzip)"

echo -e "\033[0;32mINFO: Updating magic_enum\033[0m"
cd ../magic_enum || exit 1
git checkout master
git pull

echo -e "\033[0;32mINFO: Updating palanteer\033[0m"
cd ../palanteer || exit 1
git fetch --tags
git checkout "$(get_latest_release dfeneyrou/palanteer)"

echo -e "\033[0;32mINFO: Updating rapidjson\033[0m"
cd ../rapidjson || exit 1
git checkout master
git pull

echo -e "\033[0;32mINFO: Updating unordered_dense\033[0m"
cd ../unordered_dense || exit 1
git fetch --tags
git checkout "$(get_latest_release martinus/unordered_dense)"

echo -e "\033[0;32mINFO: Updating spdlog\033[0m"
cd ../spdlog || exit 1
git checkout v1.x
git pull

echo -e "\033[0;32mINFO: Updating SQLiteCpp\033[0m"
cd ../SQLiteCpp || exit 1
git checkout "$(get_latest_release SRombauts/SQLiteCpp)"
#git submodule update --recursive
git pull

echo -e "\033[0;32mINFO: Updating stb\033[0m"
cd ../stb || exit 1
git checkout master
git pull

echo -e "\033[0;32mINFO: Updating tinyfiledialogs\033[0m"
cd ../tinyfiledialogs || exit 1
git checkout master
git pull

echo -e "\033[0;32mINFO: Updating fcl\033[0m"
cd ../fcl || exit 1
git checkout master
git pull

