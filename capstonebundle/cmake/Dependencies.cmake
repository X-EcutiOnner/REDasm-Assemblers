include(cmake/CPM.cmake)

function(setup_dependencies)
    find_package(capstone 6.0 QUIET)

    if(capstone_FOUND)
        if(NOT TARGET capstone)
            add_library(capstone ALIAS capstone::capstone_shared)
        endif()
        message(STATUS "Using system capstone")
    else()
        CPMAddPackage(
            NAME capstone
            GIT_TAG "6.0.0-Alpha7"
            GITHUB_REPOSITORY "capstone-engine/capstone"
            GIT_SHALLOW ON

            OPTIONS 
                "CAPSTONE_ARCHITECTURE_DEFAULT OFF"
                "CAPSTONE_ARM_SUPPORT ON"
                "CAPSTONE_AARCH64_SUPPORT ON"
        )
    endif()
endfunction()
