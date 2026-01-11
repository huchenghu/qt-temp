#!/usr/bin/env bash

# 构建脚本

build_type=${1:-debug}
build_type=${build_type,,}  # 转全小写
valid_types=("debug" "release" "relwithdebinfo")

# 检查当前目录是否有CMakeLists.txt
if [[ ! -f "CMakeLists.txt" ]]; then
  echo
  echo -e >&2 "\n\e[31m[ERROR]\e[0m: CMakeLists.txt not found in current directory!\n"
  echo "Please run this script from the project root directory"
  exit 1
fi

# 构建类型映射
case "$build_type" in
  "debug")
    cmake_build_type="Debug"
    preset_name="debug"
    ;;
  "release")
    cmake_build_type="Release"
    preset_name="release"
    ;;
  "relwithdebinfo")
    cmake_build_type="RelWithDebInfo"
    preset_name="relwithdebinfo"
    ;;
  *)
    echo "ERROR: Invalid build type '$1'. Valid types: ${valid_types[*]}"
    exit 1
    ;;
esac

build_dir="build/${build_type}"
install_dir="install/${build_type}"

# 检查cmake命令
if ! command -v cmake &> /dev/null; then
  echo "ERROR: cmake is not installed."
  exit 1
fi

# 检查cmake版本
cmake_version=$(cmake --version | awk 'NR==1 {print $3}')
if [[ -z "$cmake_version" ]]; then
  echo "ERROR: Failed to get cmake version."
  exit 1
fi

# 分割版本号 major.minor
IFS='.' read -ra VER <<< "$cmake_version"
major=${VER[0]}
minor=${VER[1]:-0}

# cmake版本大于3.28，则使用preset
use_preset=false
if [[ $major -gt 3 ]] || [[ $major -eq 3 && $minor -ge 28 ]]; then
  use_preset=true
fi

# config & build
if [[ "$use_preset" = true ]]; then
  cmake --preset "$preset_name"
  cmake --build --preset "$preset_name"
else
  cmake -S . -B "$build_dir" \
    -DCMAKE_C_COMPILER=gcc \
    -DCMAKE_CXX_COMPILER=g++ \
    -DBUILD_SHARED_LIBS=ON \
    -DCMAKE_BUILD_TYPE="$cmake_build_type" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCMAKE_INSTALL_PREFIX="$install_dir"

  cmake --build "$build_dir" \
    --config "$cmake_build_type" \
    --clean-first
fi

# install
cmake --install "$build_dir"
