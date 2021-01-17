#!/usr/bin/env bash

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  echo "linux detected"
fi
if [[ "$OSTYPE" == "darwin"* ]]; then
  echo "macOS detected"
fi

if [[ "$OSTYPE" =~ ^(cygwin|msys)$ ]]; then
  echo "MinGW/MSYS or cygwin detected"
fi
