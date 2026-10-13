/*
 * @file Boton.h
 * @brief Botón con 3 estados visuales: normal, hover (mouse encima) y presionado.
 * @details Usa tres texturas con la clave "<estilo>_normal", "<estilo>_hover" y "<estilo>_presionado"
 *          (por defecto estilo = "ui/boton"). El clic se dispara al SOLTAR el botón izquierdo
 *          estando encima del botón, que es como funciona cualquier botón de un programa normal.
 */

#ifndef GUI_BOTON_H
#define GUI_BOTON_H

#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include "gui/Etiqueta.h"

class GestorRecursos;

class Boton
{
public:
    Boton(GestorRecursos& recursos, const std::string& texto, sf::Vector2f centro,
          std::function<void()> alClic,
          const std::string& estilo = "ui/boton",
          sf::Vector2u tamNativoPorDefecto = sf::Vector2u(64, 16));

    void procesarEvento(const sf::Event& evento);
    void dibujar(sf::RenderTarget& destino) const;

    void setTexto(const std::string& texto);
    void setPosicion(sf::Vector2f centro);
    void setHabilitado(bool habilitado);
    void setEnfocado(bool enfocado);         // navegación con teclado: se ve igual que hover
    void setSeleccionado(bool seleccionado); // se ve "hundido" (para opciones tipo 1/2/3)
    void activar();                          // simula un clic (teclado)
    sf::FloatRect getLimites() const;

private:
    enum class Estado { Normal, Hover, Presionado };

    Estado visual() const;
    const sf::Texture& textura(Estado estado) const;
    void aplicarEstado();
    void ajustarTexto();

    GestorRecursos& recursos_;
    std::string estilo_;
    sf::Vector2u tamNativo_;
    sf::Vector2f centro_;
    std::function<void()> alClic_;
    Etiqueta etiqueta_;

    Estado estadoMouse_;
    bool habilitado_;
    bool enfocado_;
    bool seleccionado_;
};

#endif
