cmake_minimum_required(VERSION 3.16)

# Silenciar advertencias de depreciación de dependencias externas
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)

project(ElJuegoDeLaVida VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Forzar SFML a compilar de forma estatica
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
# Desactivar módulos no utilizados para evitar conflictos con MinGW moderno
set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_DOC OFF CACHE BOOL "" FORCE)

# Descarga e integración automática de SFML 2.6.1
include(FetchContent)
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG        2.6.1
)
FetchContent_MakeAvailable(SFML)

# Incluir las cabeceras del proyecto
include_directories(include)

# CAMBIO: en vez de listar cada .cpp a mano, se toman todos los de src/ (incluida src/gui/).
# CONFIGURE_DEPENDS hace que CMake detecte archivos nuevos al volver a compilar.
file(GLOB_RECURSE FUENTES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)

# Definición del ejecutable
add_executable(ElJuegoDeLaVida ${FUENTES})

# Con Visual Studio (MSVC) hay que indicar que el código fuente está en UTF-8 (tildes y ñ en los textos).
if(MSVC)
    target_compile_options(ElJuegoDeLaVida PRIVATE /utf-8)
endif()

# Enlace de librerías SFML
target_link_libraries(ElJuegoDeLaVida PRIVATE sfml-graphics sfml-window sfml-system)

# NUEVO: copiar la carpeta assets/ (fuente y texturas) junto al ejecutable en cada compilación.
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/assets)
add_custom_command(
    TARGET ElJuegoDeLaVida POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:ElJuegoDeLaVida>/assets
    COMMENT "Copiando carpeta assets junto al ejecutable..."
)