cmake_minimum_required(VERSION 3.16)

project(ModuloEstoquePOO LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(estoque
    src/main.cpp
    src/ConsoleUtils.cpp
    src/Estoque.cpp
    src/ItemEstoque.cpp
    src/PersistenciaArquivo.cpp
    src/Produto.cpp
)

target_include_directories(estoque PRIVATE include)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(estoque PRIVATE -Wall -Wextra -pedantic)
endif()
