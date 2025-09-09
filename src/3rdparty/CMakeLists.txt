set(third_party_dir "ThirdParty")

#set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)

if(NOT TARGET spdlog::spdlog)
    add_subdirectory(spdlog)
    set_target_properties(spdlog PROPERTIES FOLDER ${third_party_dir}/spdlog)
endif()