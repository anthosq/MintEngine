set(third_party_dir "ThirdParty")

#set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)

if(NOT TARGET spdlog::spdlog)
    option(SPDLOG_BUILD_EXAMPLE "" OFF)
    option(SPDLOG_INSTALL "" OFF)
    add_subdirectory(spdlog)
    set_target_properties(spdlog PROPERTIES FOLDER ${third_party_dir}/spdlog)
endif()

if(NOT TARGET glfw)
    option(GLFW_BUILD_EXAMPLES "" OFF)
    option(GLFW_BUILD_TESTS "" OFF)
    option(GLFW_BUILD_DOCS "" OFF)
    option(GLFW_INSTALL "" OFF)
    add_subdirectory(glfw)

    set_target_properties(glfw PROPERTIES FOLDER ${third_party_dir}/glfw)
    set_target_properties(update_mappings PROPERTIES FOLDER ${third_party_dir}/glfw)
endif()

if(NOT TARGET imgui)
    include(imgui.cmake)
    set_target_properties(imgui PROPERTIES FOLDER ${third_party_dir}/imgui)
endif()

if(NOT TARGET glad)
    include(glad.cmake)
    set_target_properties(glad PROPERTIES FOLDER ${third_party_dir}/glad)
endif()