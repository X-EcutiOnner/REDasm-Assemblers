include(cmake/CPM.cmake)

function(setup_dependencies)
    find_package(zydis QUIET)

    if(zydis_FOUND)
        if(NOT TARGET Zydis)
            add_library(Zydis ALIAS Zydis::Zydis)
        endif()
        message(STATUS "Using system zydis")
    else()
        CPMAddPackage(
            NAME zydis
            VERSION "4.1.1"
            GITHUB_REPOSITORY "zyantific/zydis"
            GIT_SHALLOW ON
            EXCLUDE_FROM_ALL YES

            OPTIONS 
                "CMAKE_POSITION_INDEPENDENT_CODE ON"
                "ZYDIS_BUILD_TOOLS OFF"
                "ZYDIS_BUILD_EXAMPLES OFF"
        )
    endif()
endfunction()
