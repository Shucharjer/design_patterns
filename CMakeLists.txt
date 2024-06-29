cmake_minimum_required(VERSION 3.15)

#至少要使用C++20，因为使用到了约束来简化代码
set(CMAKE_CXX_STANDARD 23)

set(CMAKE_INSTALL_PREFIX ./install)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)

project(design_patterns)

include_directories(./include)

file (GLOB SRC_FILES
"${PROJECT_SOURCE_DIR}/test/*.cpp"
"${PROJECT_SOURCE_DIR}/source/*.cpp"
"${PROJECT_SOURCE_DIR}/source/*/*.cpp"
"${PROJECT_SOURCE_DIR}/source/*/*/*.cpp"
"${PROJECT_SOURCE_DIR}/source/*/*/*/*.cpp"
)

add_executable(
    ${PROJECT_NAME} ${SRC_FILES}
)

include(CMakePaths.cmake)
include(CMakeFunctions.cmake)
getLibraryConfigPath("Catch2" VCPKG_PACKAGE_CONFIG_PATH)
find_package(Catch2 CONFIG REQUIRED HINTS ${VCPKG_PACKAGE_CONFIG_PATH})
target_link_libraries(${PROJECT_NAME} PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)
