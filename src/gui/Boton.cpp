#include "gui/Boton.h"
#include "gui/Config.h"
#include "gui/GestorRecursos.h"

Boton::Boton(GestorRecursos& recursos, const std::string& texto, sf::Vector2f centro,
             std::function<void()> alClic, const std::string& estilo, sf::Vector2u tamNativoPorDefecto)
    : recursos_(recursos),
      estilo_(estilo),
      tamNativo_(tamNativoPorDefecto),
      centro_(centro),
      alClic_(std::move(alClic)),
      etiqueta_(recursos, texto, 24, centro, Etiqueta::Alineacion::Centro),
      estadoMouse_(Estado::Normal),
      habilitado_(true),
      enfocado_(false),
      seleccionado_(false),
      colorTinte_(sf::Color::White)
{
    ajustarTexto();
    aplicarEstado();
}

const sf::Texture& Boton::textura(Estado estado) const
{
    // Los colores solo se usan mientras no existan tus PNG: así ya se nota el efecto de "iluminar".
    switch (estado)
    {
    case Estado::Hover:
        return recursos_.textura(estilo_ + "_hover", tamNativo_, sf::Color(165, 165, 190));
    case Estado::Presionado:
        return recursos_.textura(estilo_ + "_presionado", tamNativo_, sf::Color(70, 70, 95));
    default:
        return recursos_.textura(estilo_ + "_normal", tamNativo_, sf::Color(115, 115, 140));
    }
}

Boton::Estado Boton::visual() const
{
    if (!habilitado_) return Estado::Normal;
    if (estadoMouse_ == Estado::Presionado || seleccionado_) return Estado::Presionado;
    if (estadoMouse_ == Estado::Hover || enfocado_) return Estado::Hover;
    return Estado::Normal;
}

sf::FloatRect Boton::getLimites() const
{
    // Tamaño en pantalla = tamaño de la imagen * escala de pixel art.
    const sf::Vector2u t = textura(Estado::Normal).getSize();
    const float ancho = t.x * Config::ESCALA_PIXEL;
    const float alto = t.y * Config::ESCALA_PIXEL;
    return sf::FloatRect(centro_.x - ancho / 2.0f, centro_.y - alto / 2.0f, ancho, alto);
}

void Boton::ajustarTexto()
{
    // Ajusta el tamaño suavemente (de 1 en 1) para que el texto siempre tenga buen margen
    // tanto horizontal como vertical y nunca salte bruscamente a letras diminutas.
    const sf::FloatRect lim = getLimites();
    const float maxAncho = lim.width - 16.0f;
    unsigned int tam = static_cast<unsigned int>(lim.height * 0.45f);
    if (tam > 22) tam = 22;
    if (tam < 11) tam = 11;

    etiqueta_.setTamano(tam);
    while (etiqueta_.getAncho() > maxAncho && tam > 10)
    {
        --tam;
        etiqueta_.setTamano(tam);
    }
}

void Boton::aplicarEstado()
{
    // Al presionar, el texto baja unos píxeles: da sensación de botón "hundido".
    const float baja = (visual() == Estado::Presionado) ? Config::ESCALA_PIXEL : 0.0f;
    etiqueta_.setPosicion(sf::Vector2f(centro_.x, centro_.y + baja));
}

void Boton::procesarEvento(const sf::Event& evento)
{
    if (!habilitado_) return;

    switch (evento.type)
    {
    case sf::Event::MouseMoved:
    {
        // Si el mouse está encima -> Hover; si no -> Normal (salvo que se esté presionando).
        const bool dentro = getLimites().contains(static_cast<float>(evento.mouseMove.x),
                                                  static_cast<float>(evento.mouseMove.y));
        if (estadoMouse_ != Estado::Presionado)
            estadoMouse_ = dentro ? Estado::Hover : Estado::Normal;
        break;
    }
    case sf::Event::MouseButtonPressed:
        if (evento.mouseButton.button == sf::Mouse::Left &&
            getLimites().contains(static_cast<float>(evento.mouseButton.x),
                                  static_cast<float>(evento.mouseButton.y)))
            estadoMouse_ = Estado::Presionado;
        break;

    case sf::Event::MouseButtonReleased:
        if (evento.mouseButton.button == sf::Mouse::Left && estadoMouse_ == Estado::Presionado)
        {
            const bool dentro = getLimites().contains(static_cast<float>(evento.mouseButton.x),
                                                      static_cast<float>(evento.mouseButton.y));
            estadoMouse_ = dentro ? Estado::Hover : Estado::Normal;
            aplicarEstado();
            if (dentro && alClic_) alClic_(); // se ejecuta la acción del botón
            return;
        }
        break;

    default:
        break;
    }
    aplicarEstado();
}

void Boton::dibujar(sf::RenderTarget& destino) const
{
    const sf::Texture& tex = textura(visual());

    sf::Sprite sprite(tex); // un sprite = "pegatina" que muestra una textura
    sprite.setOrigin(tex.getSize().x / 2.0f, tex.getSize().y / 2.0f); // origen al centro de la imagen
    sprite.setScale(Config::ESCALA_PIXEL, Config::ESCALA_PIXEL);
    sprite.setPosition(centro_);
    if (!habilitado_)
        sprite.setColor(sf::Color(130, 130, 130));
    else
        sprite.setColor(colorTinte_);

    destino.draw(sprite);
    etiqueta_.dibujar(destino);
}

void Boton::setTexto(const std::string& texto)
{
    etiqueta_.setTexto(texto);
    ajustarTexto();
    aplicarEstado();
}

void Boton::setPosicion(sf::Vector2f centro)
{
    centro_ = centro;
    aplicarEstado();
}

void Boton::setHabilitado(bool habilitado)
{
    habilitado_ = habilitado;
    if (!habilitado_) estadoMouse_ = Estado::Normal;
    aplicarEstado();
}

void Boton::setEnfocado(bool enfocado)
{
    enfocado_ = enfocado;
    aplicarEstado();
}

void Boton::setSeleccionado(bool seleccionado)
{
    seleccionado_ = seleccionado;
    aplicarEstado();
}

void Boton::setColorTinte(sf::Color color)
{
    colorTinte_ = color;
}

void Boton::activar()
{
    if (habilitado_ && alClic_) alClic_();
}
