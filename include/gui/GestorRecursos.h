/*
 * @file GestorRecursos.h
 * @brief Carga y guarda la fuente y TODAS las texturas de la aplicación.
 * @details Regla de oro: nadie más en el programa llama a loadFromFile. Todo lo visible
 *          se pide con textura("clave"). Si el archivo assets/textures/<clave>.png existe se usa;
 *          si no, se genera una imagen de reemplazo (color sólido) para que la app funcione igual.
 *          Así, para cambiar cualquier textura solo tienes que guardar tu PNG con el nombre correcto.
 */

#ifndef GUI_GESTOR_RECURSOS_H
#define GUI_GESTOR_RECURSOS_H

#include <SFML/Graphics.hpp>
#include <map>
#include <string>

// Forma de la imagen de reemplazo que se genera cuando falta el PNG.
enum class Forma { Plano, Rectangulo, Circulo };

class GestorRecursos
{
public:
    GestorRecursos();
    GestorRecursos(const GestorRecursos&) = delete;
    GestorRecursos& operator=(const GestorRecursos&) = delete;

    const sf::Font& fuente() const { return fuente_; }

    /*
     * @brief Devuelve la textura asociada a una clave (por ejemplo "ui/boton_normal").
     * @param tamPorDefecto  Tamaño (en píxeles de la imagen, no de pantalla) del reemplazo.
     * @param colorPorDefecto Color del reemplazo.
     * @param formaPorDefecto Forma del reemplazo (plano, rectángulo con borde o círculo).
     * @details Los parámetros "PorDefecto" solo se usan si el PNG no existe.
     */
    const sf::Texture& textura(const std::string& clave, sf::Vector2u tamPorDefecto,
                               sf::Color colorPorDefecto, Forma formaPorDefecto = Forma::Rectangulo);

    /*
     * @brief Dibuja una textura estirada para cubrir un área rectangular (fondos y paneles).
     */
    void dibujarFondo(sf::RenderTarget& destino, const std::string& clave, sf::Vector2u tamPorDefecto,
                      sf::Color colorPorDefecto, sf::Vector2f posicion, sf::Vector2f area);

    /*
     * @brief Vuelve a leer del disco todas las texturas cuyo PNG exista (tecla F5 en el juego).
     */
    void recargar();

private:
    sf::Font fuente_;
    // std::map no invalida las referencias a sus elementos al insertar: es seguro devolver referencias.
    std::map<std::string, sf::Texture> texturas_;
};

#endif
