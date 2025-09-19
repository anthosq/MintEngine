cmake_minimum_required(VERSION 3.19 FATAL_ERROR)


project(Mint VERSION 0.0.1)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(BUILD_SHARED_LIBS OFF)

include(CMakeDependentOption)



set(MINT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SOURCE_DIR "${MINT_ROOT_DIR}/src")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${MINT_ROOT_DIR}/bin")

# TODO: 命名格式 Configuration-Platform-Project
# add_compile_options(-Wall -Wextra -Wpedantic -Werror)


# file(GLOB_RECURSE sources CONFIGURE_DEPENDS
#     "${SOURCE_DIR}/ *.h"
#     "${SOURCE_DIR}/ *.cpp"
# )

add_subdirectory("${SOURCE_DIR}")

add_executable(Mint sandbox.cpp)


# 添加源文件到可执行文件
# target_sources(Mint PUBLIC ${sources})

target_include_directories(Mint 
    PUBLIC 
        "${SOURCE_DIR}"
        "${SOURCE_DIR}/platform/windows"
        "${SOURCE_DIR}/render"
        "${SOURCE_DIR}/event"
)
target_link_libraries(Mint PUBLIC MintRuntime)


