cmake_minimum_required(VERSION 3.16)

set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)

project(ElJuegoDeLaVida VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Buscar librerías en MSYS2 UCRT64 si existe
if(EXISTS "C:/msys64/ucrt64")
    list(APPEND CMAKE_PREFIX_PATH "C:/msys64/ucrt64")
endif()

# 1. Intentar usar SFML instalado en el sistema (PC de tu compañera)
find_package(SFML 2.5 COMPONENTS graphics window system QUIET)

# 2. Si no existe en el sistema, descargarlo (Tu PC)
if(NOT SFML_FOUND)
    message(STATUS "SFML no encontrado. Descargando vía FetchContent...")
    
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)

    include(FetchContent)
    FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG        2.6.1
    )
    FetchContent_MakeAvailable(SFML)
endif()

include_directories(include)

file(GLOB_RECURSE FUENTES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(ElJuegoDeLaVida ${FUENTES})

if(MSVC)
    target_compile_options(ElJuegoDeLaVida PRIVATE /utf-8)
endif()

target_link_libraries(ElJuegoDeLaVida PRIVATE sfml-graphics sfml-window sfml-system)

file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/assets)
add_custom_command(
    TARGET ElJuegoDeLaVida POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:ElJuegoDeLaVida>/assets
    COMMENT "Copiando carpeta assets junto al ejecutable..."
)