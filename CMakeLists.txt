cmake_minimum_required(VERSION 3.21)

option(SKYRIM_AE "Build for Skyrim AE" OFF)

project(save-manager VERSION 0.0.1 LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
  tools
  GIT_REPOSITORY https://github.com/Zouizoui78/tools
  GIT_TAG 0c9cff10f046e478055caa5fa20f9c72dfd0abd8
)
FetchContent_MakeAvailable(tools)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/version.h
    @ONLY
)

set(
    SRC
    src/PCH.h
    src/plugin_init.hpp
    src/plugin.cpp
    src/Conf.cpp
    src/Save.cpp
    src/savetools.cpp
    src/tools.cpp
)

add_library(${PROJECT_NAME} SHARED ${SRC})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_precompile_headers(${PROJECT_NAME} PRIVATE src/PCH.h)

target_link_libraries(save-manager PRIVATE tools)

if(SKYRIM_AE)
    add_compile_definitions(SKYRIM_SUPPORT_AE)
endif()

add_subdirectory(external/CommonLibSSE)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(libzip CONFIG REQUIRED)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include
    PRIVATE src
    PRIVATE external/CommonLibSSE/include
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE CommonLibSSE::CommonLibSSE
    PRIVATE nlohmann_json::nlohmann_json
    PRIVATE spdlog::spdlog
    PRIVATE Shlwapi
    PRIVATE libzip::zip
)

if(DEFINED ENV{SKYRIM_MODS_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_MODS_FOLDER}")
    set(OUTPUT_FOLDER "$ENV{SKYRIM_MODS_FOLDER}/${PROJECT_NAME}")
endif()

if(DEFINED OUTPUT_FOLDER)
    set(DLL_FOLDER "${OUTPUT_FOLDER}/SKSE/Plugins")

    message(STATUS "SKSE plugin output folder: ${DLL_FOLDER}")

    add_custom_command(
        TARGET "${PROJECT_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${DLL_FOLDER}"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
        VERBATIM
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_custom_command(
            TARGET "${PROJECT_NAME}"
            POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_PDB_FILE:${PROJECT_NAME}>" "${DLL_FOLDER}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
            VERBATIM
        )
    endif()
endif()
