#!/usr/bin/env bash
if ! command -v cloc &> /dev/null
then
    echo "Install cloc first. (sudo apt install cloc)"
    exit
fi

cloc . --exclude-dir=lib,build,docs