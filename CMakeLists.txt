cmake_minimum_required(VERSION 3.21)

project(InventoryApi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Drogon CONFIG REQUIRED)

add_executable(inventory_api
    src/main.cpp
    src/api/AppConfig.cpp
    src/api/ItemTypeProfile.cpp
    src/api/InventoryService.cpp
    src/http/InventoryController.cpp
)

target_include_directories(inventory_api PRIVATE include)
target_link_libraries(inventory_api PRIVATE Drogon::Drogon)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(inventory_api PRIVATE -Wall -Wextra -pedantic)
endif()

if (WIN32)
    add_custom_command(TARGET inventory_api POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:inventory_api>
            $<TARGET_FILE_DIR:inventory_api>
        COMMAND_EXPAND_LISTS
    )

    if (MINGW)
        get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        add_custom_command(TARGET inventory_api POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${MINGW_BIN_DIR}/libstdc++-6.dll"
                "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
                "${MINGW_BIN_DIR}/libwinpthread-1.dll"
                $<TARGET_FILE_DIR:inventory_api>
        )
    endif()
endif()
