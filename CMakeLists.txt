cmake_minimum_required(VERSION 3.20)
project(msys2_ucrt64_shell LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(msys2_ucrt64_shell SHARED
    src/dll.cpp
    src/ClassFactory.cpp
    src/ExplorerCommand.cpp
)

add_executable(msys2_ucrt64_shell_stub src/stub.cpp)

target_link_libraries(msys2_ucrt64_shell PRIVATE ole32 shell32 shlwapi)
target_compile_definitions(msys2_ucrt64_shell PRIVATE UNICODE _UNICODE)
set_target_properties(msys2_ucrt64_shell PROPERTIES PREFIX "")                          # avoid "lib" prefix, keep plain .dll name
