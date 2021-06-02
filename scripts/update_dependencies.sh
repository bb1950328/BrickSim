#!/usr/bin/env bash

# https://gist.github.com/lukechilds/a83e1d7127b78fef38c2914c4ececc3c
get_latest_release() {
  curl --silent "https://api.github.com/repos/$1/releases/latest" | # Get latest release from GitHub api
    grep '"tag_name":' |                                            # Get tag line
    sed -E 's/.*"([^"]+)".*/\1/'                                    # Pluck JSON value
}

cd src/lib/Catch2 || exit 1
git fetch --tags
git checkout "$(get_latest_release catchorg/Catch2)"

cd ../cpuinfo || exit 1
git checkout master
git pull

cd ../curl || exit 1
git fetch --tags
git checkout "$(get_latest_release curl/curl)"

cd ../glfw || exit 1
git checkout master
git pull

cd ../glm || exit 1
git fetch --tags
#git checkout "$(get_latest_release g-truc/glm)"
git checkout master
git pull

cd ../IconFontCppHeaders || exit 1
git checkout main
git pull

cd ../imgui || exit 1
git checkout docking
git pull

cd ../libzip || exit 1
git fetch --tags
git checkout "$(get_latest_release nih-at/libzip)"

cd ../magic_enum || exit 1
git checkout master
git pull

cd ../rapidjson || exit 1
git checkout master
git pull

cd ../spdlog || exit 1
git checkout v1.x
git pull

cd ../SQLiteCpp || exit 1
git checkout master
git pull

cd ../stb || exit 1
git checkout master
git pull

cd ../tinyfiledialogs || exit 1
git checkout master
git pull