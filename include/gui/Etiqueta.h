/*
 * @file Etiqueta.h
 * @brief Texto en pantalla con sombra (estilo Minecraft) y alineación sencilla.
 */

#ifndef GUI_ETIQUETA_H
#define GUI_ETIQUETA_H

#include <SFML/Graphics.hpp>
#include <string>

class GestorRecursos;

class Etiqueta
{
public:
    // Izquierda: el punto "posicion" es el borde izquierdo (centrado en vertical).
    // Centro   : el punto "posicion" es el centro del texto.
    enum class Alineacion { Izquierda, Centro };

    Etiqueta(GestorRecursos& recursos, const std::string& texto, unsigned int tamano,
             sf::Vector2f posicion, Alineacion alineacion = Alineacion::Izquierda);

    void setTexto(const std::string& texto);
    void setTamano(unsigned int tamano);
    void setPosicion(sf::Vector2f posicion);
    void setColor(sf::Color color);
    float getAncho() const;
    void dibujar(sf::RenderTarget& destino) const;

private:
    void asignarCadena();
    void actualizarGeometria();

    sf::Text texto_;   // el texto de verdad
    sf::Text sombra_;  // copia oscura desplazada unos píxeles: da el efecto de sombra
    std::string contenido_;
    unsigned int tamano_;
    sf::Vector2f posicion_;
    Alineacion alineacion_;
};

#endif
