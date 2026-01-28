project(MintEditor)

file(GLOB_RECURSE SOURCES "*.cpp" "*.h")

add_executable(MintEditor ${SOURCES})

target_include_directories(MintEditor PRIVATE 
    ${MINT_ROOT_DIR}/src
    ${MINT_ROOT_DIR}/src/3rdparty
    ${MINT_ROOT_DIR}/src/3rdparty/spdlog/include
    ${MINT_ROOT_DIR}/src/3rdparty/imgui
    ${MINT_ROOT_DIR}/src/3rdparty/glm
    ${MINT_EDITOR_DIR}/src
)

target_link_libraries(MintEditor PRIVATE MintRuntime)

target_compile_features(MintEditor PRIVATE cxx_std_20)