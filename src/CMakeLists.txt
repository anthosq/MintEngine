
set(TARGET_NAME MintRuntime)


# set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ENGINE_ROOT_DIR "${MINT_ROOT_DIR}/src")
set(THIRD_PARTY_DIR "${ENGINE_ROOT_DIR}/3rdparty")


file(GLOB_RECURSE HEADER_FILES "*.h")
file(GLOB_RECURSE SOURCE_FILES "*.cpp")

list(FILTER HEADER_FILES EXCLUDE REGEX "3rdparty/.*")
list(FILTER SOURCE_FILES EXCLUDE REGEX "3rdparty/.*")


source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${HEADER_FILES}
    ${SOURCE_FILES}
)


add_library(${TARGET_NAME} STATIC
    ${HEADER_FILES}
    ${SOURCE_FILES}
)

# test?
add_subdirectory(3rdparty)

target_include_directories(${TARGET_NAME}
    PUBLIC
        ${ENGINE_ROOT_DIR}
        ${ENGINE_ROOT_DIR}/3rdparty
)

# link dependencies
target_link_libraries(${TARGET_NAME}
    PUBLIC
        spdlog::spdlog
        imgui
        glfw
        glad
        glm
)