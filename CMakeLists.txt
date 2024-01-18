cmake_minimum_required(VERSION 3.21)
project(save-manager VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(DEFINED ENV{SKYRIM_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_FOLDER}/Data")
    set(OUTPUT_FOLDER "$ENV{SKYRIM_FOLDER}/Data")
endif()

if(DEFINED ENV{SKYRIM_MODS_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_MODS_FOLDER}")
    set(OUTPUT_FOLDER "$ENV{SKYRIM_MODS_FOLDER}/${PROJECT_NAME}")
endif()

set(
    SRC
    src/PCH.h
    src/plugin.cpp
    src/Conf.cpp
    src/Save.cpp
    src/savetools.cpp
    src/tools.cpp
    src/wintools.cpp
    src/ziptools.cpp
)

add_subdirectory(external/CommonLibSSE)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(libzip CONFIG REQUIRED)

add_library(${PROJECT_NAME} SHARED ${SRC})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_precompile_headers(${PROJECT_NAME} PRIVATE src/PCH.h)

add_compile_definitions(SKYRIM_AE)
add_compile_definitions(SKYRIM_SUPPORT_AE)

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE src
    PRIVATE external/CommonLibSSE/include
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE nlohmann_json::nlohmann_json
    PRIVATE spdlog::spdlog
    PRIVATE Shlwapi
    PRIVATE libzip::zip
    PRIVATE CommonLibSSE::CommonLibSSE
)

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
