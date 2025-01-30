cmake_minimum_required(VERSION 3.15)

#至少要使用C++20，因为使用到了约束来简化代码
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_INSTALL_PREFIX ./install)

set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)

set(VCPKG_MANIFEST_MODE ON)

project(design_patterns)

include_directories(./include)

file(
    GLOB HDR_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*/*.hpp"
)

add_library(patterns INTERFACE ${HDR_FILES})

set_target_properties(patterns PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(patterns INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

install(
    TARGETS patterns
    EXPORT PatternsTargets
    LIBRARY DESTINATION ${INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${INSTALL_LIBDIR}
    RUNTIME DESTINATION ${INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT PatternsTargets
    FILE PatternsTargets.cmake
    DESTINATION cmake
)

option(BUILD_TEST OFF)

# if(BUILD_TEST)
file (GLOB SRC_FILES
"${PROJECT_SOURCE_DIR}/test/*.cpp"
)

add_executable(
    ${PROJECT_NAME} ${SRC_FILES}
)

find_package(Catch2 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
Patterns
Catch2::Catch2 Catch2::Catch2WithMain
)
# endif()

option(BUILD_DOCS ON)

# if (BUILD_DOCS)
if (True)
    find_package(Doxygen REQUIRED dot)
    if (DOXYGEN_FOUND)
        include(FetchContent)

        FetchContent_Declare(
            doxygen-awesome-css
            GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css
            GIT_TAG main
            GIT_SHALLOW 1
        )

        FetchContent_GetProperties(doxygen-awesome-css)

        if(NOT doxygen-awesome-css_POPULATED)
            FetchContent_Populate(doxygen-awesome-css)
            set(doxygen-awesome-css_INCLUDE_DIR ${doxygen-awesome-css_SOURCE_DIR})
        endif()

        set(DOXY_SOURCE_DIRECTORY ${design_patterns_SOURCE_DIR}/include)
        set(DOXY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs)
        set(DOXY_CSS_DIRECTORY ${doxygen-awesome-css_INCLUDE_DIR})

        configure_file(doxy.in doxy.cfg @ONLY)

        add_custom_target(
            documents
            COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/doxy.cfg
            WORKING_DIRECTORY ${design_patterns_SOURCE_DIR}
            VERBATIM
        )
    else()
        message(FATAL_ERROR "Required to build documents but doxygen is not found!")
    endif()
endif()