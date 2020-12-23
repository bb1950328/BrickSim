#!/usr/bin/env bash

declare -a files=("RobotoMono-Regular.ttf"
                "Roboto-Regular.ttf"
                "fa-solid-900.ttf"
                )

if [ ! -d "src" ]; then
    echo "please change the working directory to the repository root."
    exit 1
fi

echo -e "#pragma once\nnamespace resource {\n" > "src/resources.h"

for f in "${files[@]}"
do
  xxd -i "$f" >> src/resources.h
done

echo -e "\n}" >> src/resources.h