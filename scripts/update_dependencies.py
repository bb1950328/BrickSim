#!/usr/bin/env python

import json
import subprocess
import sys
import urllib.request


def get_latest_release(repo_name: str) -> str:
    response = urllib.request.urlopen(f"https://api.github.com/repos/{repo_name}/releases/latest").read()
    data = json.loads(response)
    return data["tag_name"]


def run_in_dir(working_directory: str, command: str) -> str:
    p = subprocess.Popen(command.split(" "), cwd=working_directory)
    stdout, stderr = p.communicate()
    return stdout


def update_to_latest_release(directory, repo_name):
    release = get_latest_release(repo_name)
    run_in_dir(directory, "git fetch --tags")
    run_in_dir(directory, f"git checkout {release}")


def pull_branch(directory: str, branch: str):
    run_in_dir(directory, f"git checkout {branch}")
    run_in_dir(directory, "git pull")


if __name__ == '__main__':
    funcs = {
        "catch2": lambda: update_to_latest_release("src/lib/Catch2", "catchorg/Catch2"),
        "cpuinfo": lambda: pull_branch("src/lib/cpuinfo", "main"),
        "curl": lambda: update_to_latest_release("src/lib/curl", "curl/curl"),
        "earcut": lambda: (update_to_latest_release("src/lib/earcut.hpp", "mapbox/earcut.hpp"),
                           run_in_dir("src/lib/earcut.hpp", "git submodule update --recursive")),
        "efsw": lambda: pull_branch("src/lib/efsw", "master"),
        "fast_float": lambda: update_to_latest_release("src/lib/fast_float", "fastfloat/fast_float"),
        "fcl": lambda: pull_branch("src/lib/fcl", "master"),
        "glfw": lambda: pull_branch("src/lib/glfw", "master"),
        "glm": lambda: pull_branch("src/lib/glm", "master"),
        "IconFontCppHeaders": lambda: pull_branch("src/lib/IconFontCppHeaders", "main"),
        "imgui": lambda: pull_branch("src/lib/imgui", "docking"),
        "ImGuiColorTextEdit": lambda: pull_branch("src/lib/ImGuiColorTextEdit", "no-comment-fix"),
        "libzip": lambda: update_to_latest_release("src/lib/libzip", "nih-at/libzip"),
        "magic_enum": lambda: pull_branch("src/lib/magic_enum", "master"),
        "palanteer": lambda: update_to_latest_release("src/lib/palanteer", "dfeneyrou/palanteer"),
        "rapidjson": lambda: pull_branch("src/lib/rapidjson", "master"),
        "unordered_dense": lambda: update_to_latest_release("src/lib/unordered_dense", "martinus/unordered_dense"),
        "spdlog": lambda: pull_branch("src/lib/spdlog", "v1.x"),
        "SQLiteCpp": lambda: update_to_latest_release("src/lib/SQLiteCpp", "SRombauts/SQLiteCpp"),
        "stb": lambda: pull_branch("src/lib/stb", "master"),
        "tinyfiledialogs": lambda: pull_branch("src/lib/tinyfiledialogs", "master"),
    }
    if len(sys.argv) == 1:
        print("Updating all dependencies")
    for name, f in funcs.items():
        if len(sys.argv) > 1 and name not in sys.argv[1:]:
            continue
        print(f"Updating {name}... ", end="")
        f()
        print("finished")
