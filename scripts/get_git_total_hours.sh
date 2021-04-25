#!/usr/bin/env bash
git log --no-merges --all | grep -oE t=[0-9]+\\.?[0-9]* | grep -oE [0-9]+\\.?[0-9]* | awk '{ SUM += $1} END { printf "%f\n", SUM }'