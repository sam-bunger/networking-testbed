#!/bin/bash
set -e

unameOut="$(uname -s)"
case "${unameOut}" in
    Linux*)     machine=Linux;;
    Darwin*)    machine=Mac;;
    CYGWIN*)    machine=Cygwin;;
    MINGW*)     machine=MinGw;;
    *)          machine="UNKNOWN:${unameOut}"
esac

if which conan >/dev/null && [ "$machine" == "Linux" ]
then
  echo "Building..."
else
  echo "Skipping Build => This machine isn't set up to build c++ code."
  exit 0
fi

mkdir -p build
mkdir -p build/prod
cd build/prod
 
conan install ../.. --profile:host=emscripten --profile:build=default --build missing -s build_type=Release
cmake ../.. -DOPTMZ=1 
cmake --build . --config Release 

cd ..

mkdir -p main
mkdir -p main/bin

cp prod/bin/index.js main/bin/index.js
cp prod/bin/index.wasm main/bin/index.wasm

# Build Source Map
[ -d build/sourcemap/client ] || mkdir -p main/sourcemap/client
cp -r -T ../src main/sourcemap/client/src
cp -r -T ${HOME}/.conan/data/emsdk/3.1.23/_/_/package/2880313eadc30db92089af7733fe8364772ee5c8/bin/upstream/emscripten/cache/sysroot/include/c++/v1 main/sourcemap/emsdk

cd ..
webidl2ts -i index.idl -o build/main/index.d.ts -ed -n Main

