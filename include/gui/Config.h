/*
 * @file Config.h
 * @brief Constantes globales de la interfaz gráfica (tamaños, escalas y rutas).
 * @details Todo número "mágico" de la interfaz vive aquí para poder cambiarlo desde un solo lugar.
 */

#ifndef GUI_CONFIG_H
#define GUI_CONFIG_H

#include <string>

namespace Config
{
    // ---------- Ventana ----------
    // La ventana se divide en dos zonas: el MUNDO (donde viven los bichitos) a la izquierda
    // y la BARRA LATERAL a la derecha.  1000 + 280 = 1280.
    inline constexpr unsigned int ANCHO_VENTANA = 1280;
    inline constexpr unsigned int ALTO_VENTANA = 720;
    inline constexpr float ANCHO_BARRA = 280.0f;
    inline constexpr float ANCHO_MUNDO = static_cast<float>(ANCHO_VENTANA) - ANCHO_BARRA;
    inline constexpr float ALTO_MUNDO = static_cast<float>(ALTO_VENTANA);

    // ---------- Pixel art ----------
    // Cada píxel de tus imágenes se dibuja como un cuadrado de ESCALA_PIXEL x ESCALA_PIXEL
    // píxeles de pantalla. Un botón de 64x16 se verá de 256x64.
    inline constexpr float ESCALA_PIXEL = 4.0f;
    // La comida es pequeña: una imagen de 8x8 se verá de 16x16.
    inline constexpr float ESCALA_COMIDA = 2.0f;
    // false: El bichito no crece.
    // true : El bichito crece según su vida.
    inline constexpr bool BICHITO_CRECE = false;

    // ---------- Rutas (relativas a la carpeta del .exe) ----------
    inline const std::string RUTA_FUENTE = "assets/fonts/Monocraft.ttf";
    inline const std::string RUTA_FUENTE_RESPALDO = "C:/Windows/Fonts/arial.ttf"; // solo si falta la anterior
    inline const std::string RUTA_TEXTURAS = "assets/textures/";
}

#endif
