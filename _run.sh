#!/usr/bin/env bash

### cannot use another port in WSL (Windows Subystem for Linux)???
url="http://localhost:8080"

case "$OSTYPE" in
  msys)
    start $url;;
  cygwin)
    cygstart $url;;
  darwin*)
    open $url;;
  *)
    xdg-open $url;;
esac
