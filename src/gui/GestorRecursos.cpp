#include "gui/GestorRecursos.h"
#include "gui/Config.h"
#include <cmath>
#include <filesystem>
#include <iostream>

namespace
{
    std::string rutaDe(const std::string& clave)
    {
        return Config::RUTA_TEXTURAS + clave + ".png";
    }

    sf::Color oscurecer(sf::Color c, float factor)
    {
        return sf::Color(static_cast<sf::Uint8>(c.r * factor),
                         static_cast<sf::Uint8>(c.g * factor),
                         static_cast<sf::Uint8>(c.b * factor), c.a);
    }

    // Crea la imagen de reemplazo en memoria (sf::Image = matriz de píxeles en la RAM).
    sf::Image crearImagenReemplazo(sf::Vector2u tam, sf::Color color, Forma forma)
    {
        if (tam.x == 0) tam.x = 1;
        if (tam.y == 0) tam.y = 1;

        sf::Image img;
        img.create(tam.x, tam.y, forma == Forma::Circulo ? sf::Color::Transparent : color);
        const sf::Color borde = oscurecer(color, 0.55f);

        if (forma == Forma::Rectangulo)
        {
            for (unsigned x = 0; x < tam.x; ++x)
                for (unsigned y = 0; y < tam.y; ++y)
                    if (x == 0 || y == 0 || x == tam.x - 1 || y == tam.y - 1)
                        img.setPixel(x, y, borde);
        }
        else if (forma == Forma::Circulo)
        {
            const float cx = tam.x / 2.0f;
            const float cy = tam.y / 2.0f;
            const float radio = std::min(tam.x, tam.y) / 2.0f;
            for (unsigned x = 0; x < tam.x; ++x)
            {
                for (unsigned y = 0; y < tam.y; ++y)
                {
                    const float dx = x + 0.5f - cx;
                    const float dy = y + 0.5f - cy;
                    const float d = std::sqrt(dx * dx + dy * dy);
                    if (d <= radio)
                        img.setPixel(x, y, d > radio - 1.2f ? borde : color);
                }
            }
        }
        return img;
    }
}

GestorRecursos::GestorRecursos()
{
    if (!fuente_.loadFromFile(Config::RUTA_FUENTE))
    {
        std::cerr << "[GestorRecursos] No se encontro la fuente '" << Config::RUTA_FUENTE
                  << "'. Probando con la de respaldo...\n";
        if (!fuente_.loadFromFile(Config::RUTA_FUENTE_RESPALDO))
            std::cerr << "[GestorRecursos] Tampoco hay fuente de respaldo: no se vera texto.\n";
    }
    // Sin suavizado: las letras de pixel art se ven nítidas.
    fuente_.setSmooth(false);
}

const sf::Texture& GestorRecursos::textura(const std::string& clave, sf::Vector2u tamPorDefecto,
                                           sf::Color colorPorDefecto, Forma formaPorDefecto)
{
    auto it = texturas_.find(clave);
    if (it != texturas_.end())
        return it->second;

    sf::Texture& tex = texturas_[clave]; // se crea vacía y luego la rellenamos
    const std::string ruta = rutaDe(clave);

    if (std::filesystem::exists(ruta) && tex.loadFromFile(ruta))
    {
        // PNG del usuario cargado correctamente.
    }
    else
    {
        tex.loadFromImage(crearImagenReemplazo(tamPorDefecto, colorPorDefecto, formaPorDefecto));
    }

    tex.setSmooth(false); // al ampliar, se ven los píxeles cuadrados (nearest neighbour)
    return tex;
}

void GestorRecursos::dibujarFondo(sf::RenderTarget& destino, const std::string& clave,
                                  sf::Vector2u tamPorDefecto, sf::Color colorPorDefecto,
                                  sf::Vector2f posicion, sf::Vector2f area)
{
    const sf::Texture& tex = textura(clave, tamPorDefecto, colorPorDefecto, Forma::Plano);
    const sf::Vector2u t = tex.getSize();

    sf::Sprite sprite(tex);
    sprite.setScale(area.x / static_cast<float>(t.x), area.y / static_cast<float>(t.y));
    sprite.setPosition(posicion);
    destino.draw(sprite);
}

void GestorRecursos::recargar()
{
    for (auto& par : texturas_)
    {
        const std::string ruta = rutaDe(par.first);
        if (std::filesystem::exists(ruta) && par.second.loadFromFile(ruta))
            par.second.setSmooth(false);
    }
    std::cout << "[GestorRecursos] Texturas recargadas.\n";
}
