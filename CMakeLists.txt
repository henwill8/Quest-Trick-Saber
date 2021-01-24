cmake_minimum_required(VERSION 3.10)

# set the project name
project(TrickSaber)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)
include_directories(extern)
include_directories(shared)
include_directories(extern/libil2cpp/il2cpp/libil2cpp)
include_directories(extern/codegen/include)
include_directories(extern/beatsaber-hook/shared)
include_directories(extern/modloader/shared)
include_directories(.)

set(CMAKE_CXX_STANDARD 20)

add_compile_definitions("VERSION=\"0.1.0\"")
add_compile_definitions("__GNUC__")
add_compile_definitions("__aarch64__")

file(GLOB SRC src/*.cpp)
add_library(TrickSaber SHARED ${SRC})