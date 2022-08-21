#!/usr/bin/env bash

wget -O - -o /dev/null https://installbuilder.com/download-step-2.html |
  grep -Po '(?<=href=")[^"]*' |
  grep "installbuilder-enterprise" |
  grep -Po "\d+.\d+.\d+" |
  head -n 1
