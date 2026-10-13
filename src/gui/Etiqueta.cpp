#include "gui/Etiqueta.h"
#include "gui/GestorRecursos.h"
#include <algorithm>
#include <cmath>

Etiqueta::Etiqueta(GestorRecursos& recursos, const std::string& texto, unsigned int tamano,
                   sf::Vector2f posicion, Alineacion alineacion)
    : contenido_(texto), tamano_(tamano), posicion_(posicion), alineacion_(alineacion)
{
    texto_.setFont(recursos.fuente());
    sombra_.setFont(recursos.fuente());
    texto_.setFillColor(sf::Color::White);
    sombra_.setFillColor(sf::Color(55, 55, 55));
    asignarCadena();
    actualizarGeometria();
}

void Etiqueta::asignarCadena()
{
    // fromUtf8 hace que las tildes y la ñ se vean bien (el código fuente está en UTF-8).
    const sf::String cadena = sf::String::fromUtf8(contenido_.begin(), contenido_.end());
    texto_.setString(cadena);
    sombra_.setString(cadena);
}

void Etiqueta::actualizarGeometria()
{
    texto_.setCharacterSize(tamano_);
    sombra_.setCharacterSize(tamano_);

    // getLocalBounds = rectángulo que ocupa el texto. El "origen" es el punto del texto que se
    // coloca exactamente sobre setPosition(); lo movemos para poder alinear a la izquierda o al centro.
    const sf::FloatRect lim = texto_.getLocalBounds();
    const float ox = (alineacion_ == Alineacion::Centro) ? lim.left + lim.width / 2.0f : lim.left;
    const float oy = lim.top + lim.height / 2.0f;

    const sf::Vector2f origen(std::round(ox), std::round(oy));
    texto_.setOrigin(origen);
    sombra_.setOrigin(origen);

    // Redondear la posición evita que el texto quede entre dos píxeles (se vería borroso).
    const sf::Vector2f p(std::round(posicion_.x), std::round(posicion_.y));
    const float desplazamiento = std::max(1.0f, std::floor(tamano_ / 8.0f));
    texto_.setPosition(p);
    sombra_.setPosition(p.x + desplazamiento, p.y + desplazamiento);
}

void Etiqueta::setTexto(const std::string& texto)
{
    if (texto == contenido_) return; // evita recalcular el texto 60 veces por segundo sin necesidad
    contenido_ = texto;
    asignarCadena();
    actualizarGeometria();
}

void Etiqueta::setTamano(unsigned int tamano)
{
    tamano_ = tamano;
    actualizarGeometria();
}

void Etiqueta::setPosicion(sf::Vector2f posicion)
{
    posicion_ = posicion;
    actualizarGeometria();
}

void Etiqueta::setColor(sf::Color color)
{
    texto_.setFillColor(color);
}

float Etiqueta::getAncho() const
{
    return texto_.getLocalBounds().width;
}

void Etiqueta::dibujar(sf::RenderTarget& destino) const
{
    destino.draw(sombra_); // primero la sombra...
    destino.draw(texto_);  // ...y encima el texto
}
