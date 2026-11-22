#include "gui/PanelGenes.h"
#include "MotorGrafico.h"
#include "gui/Config.h"
#include "gui/GestorRecursos.h"
#include <array>
#include <string>

namespace
{
    constexpr std::array<GenEstatico, 6> GENES = {
        GenEstatico::INMUNIDAD,
        GenEstatico::CANIBALISMO,
        GenEstatico::VAMPIRISMO,
        GenEstatico::TELETRANSPORTARSE,
        GenEstatico::INMORTAL,
        GenEstatico::MITOSIS
    };

    constexpr float RADIO_LOGO = 16.0f;
    constexpr float SEPARACION_LOGOS = 36.0f;
    constexpr float X_MAS = Config::ANCHO_MUNDO - 30.0f;
    constexpr float Y_LOGOS = 34.0f;
    constexpr float ANCHO_SELECTOR = 274.0f;
    constexpr float ALTO_SELECTOR = 108.0f;
    constexpr float X_SELECTOR = X_MAS - ANCHO_SELECTOR;
    constexpr float Y_SELECTOR = 56.0f;

    const char* marcaGen(GenEstatico gen)
    {
        switch (gen)
        {
        case GenEstatico::INMUNIDAD: return "I";
        case GenEstatico::CANIBALISMO: return "C";
        case GenEstatico::VAMPIRISMO: return "V";
        case GenEstatico::TELETRANSPORTARSE: return "T";
        case GenEstatico::INMORTAL: return "M";
        case GenEstatico::MITOSIS: return "D";
        case GenEstatico::NINGUNO: default: return "+";
        }
    }

    const char* nombreGen(GenEstatico gen)
    {
        switch (gen)
        {
        case GenEstatico::INMUNIDAD: return "Inmunidad";
        case GenEstatico::CANIBALISMO: return "Canibalismo";
        case GenEstatico::VAMPIRISMO: return "Vampirismo";
        case GenEstatico::TELETRANSPORTARSE: return "Teletransporte";
        case GenEstatico::INMORTAL: return "Inmortal";
        case GenEstatico::MITOSIS: return "Mitosis";
        case GenEstatico::NINGUNO: default: return "Ninguno";
        }
    }

    sf::Color colorGen(GenEstatico gen)
    {
        switch (gen)
        {
        case GenEstatico::INMUNIDAD: return sf::Color(120, 205, 135);
        case GenEstatico::CANIBALISMO: return sf::Color(32, 110, 165);
        case GenEstatico::VAMPIRISMO: return sf::Color(175, 65, 110);
        case GenEstatico::TELETRANSPORTARSE: return sf::Color(70, 195, 205);
        case GenEstatico::INMORTAL: return sf::Color(185, 140, 235);
        case GenEstatico::MITOSIS: return sf::Color(235, 165, 75);
        case GenEstatico::NINGUNO: default: return sf::Color(90, 100, 115);
        }
    }

    sf::Text crearTexto(const std::string& contenido, const sf::Font& fuente,
                        unsigned int tamano, sf::Color color)
    {
        sf::Text texto(sf::String::fromUtf8(contenido.begin(), contenido.end()), fuente, tamano);
        texto.setFillColor(color);
        return texto;
    }
}

PanelGenes::PanelGenes(GestorRecursos& recursos, MotorGrafico& motor)
    : selectorAbierto_(false),
      posicionMouseConocida_(false),
      posicionMouse_(0.0f, 0.0f),
      recursos_(recursos),
      motor_(motor)
{
}

float PanelGenes::xLogoVisible(GenEstatico gen) const
{
    int totalVisibles = 0;
    int indice = -1;
    for (GenEstatico candidato : GENES)
    {
        if (!motor_.tieneGenRegistrado(candidato))
            continue;
        if (candidato == gen)
            indice = totalVisibles;
        ++totalVisibles;
    }
    return X_MAS - SEPARACION_LOGOS * static_cast<float>(totalVisibles - indice);
}

sf::Vector2f PanelGenes::posicionOpcion(int indice) const
{
    const int columna = indice % 3;
    const int fila = indice / 3;
    return sf::Vector2f(X_SELECTOR + 45.0f + columna * 90.0f,
                        Y_SELECTOR + 24.0f + fila * 48.0f);
}

bool PanelGenes::contieneLogo(float x, float y, sf::Vector2f centro, float radio) const
{
    const float dx = x - centro.x;
    const float dy = y - centro.y;
    return dx * dx + dy * dy <= radio * radio;
}

void PanelGenes::dibujarLogo(sf::RenderTarget& destino, GenEstatico gen,
                             sf::Vector2f centro, float radio, bool resaltado) const
{
    sf::CircleShape circulo(radio);
    circulo.setOrigin(radio, radio);
    circulo.setPosition(centro);
    circulo.setFillColor(colorGen(gen));
    circulo.setOutlineThickness(resaltado ? 3.0f : 2.0f);
    circulo.setOutlineColor(resaltado ? sf::Color(255, 235, 150) :
                                       sf::Color(230, 238, 250));
    destino.draw(circulo);

    sf::Text marca = crearTexto(marcaGen(gen), recursos_.fuente(), 14, sf::Color::White);
    const sf::FloatRect limites = marca.getLocalBounds();
    marca.setPosition(centro.x - (limites.left + limites.width * 0.5f),
                      centro.y - (limites.top + limites.height * 0.5f));
    destino.draw(marca);
}

bool PanelGenes::procesarEvento(const sf::Event& evento)
{
    if (evento.type == sf::Event::MouseMoved)
    {
        posicionMouse_ = sf::Vector2f(static_cast<float>(evento.mouseMove.x),
                                      static_cast<float>(evento.mouseMove.y));
        posicionMouseConocida_ = true;
        return false;
    }

    if (evento.type != sf::Event::MouseButtonPressed ||
        evento.mouseButton.button != sf::Mouse::Left)
        return false;

    const float x = static_cast<float>(evento.mouseButton.x);
    const float y = static_cast<float>(evento.mouseButton.y);
    const float radioClic = RADIO_LOGO + 3.0f;

    if (selectorAbierto_)
    {
        for (std::size_t i = 0; i < GENES.size(); ++i)
        {
            if (contieneLogo(x, y, posicionOpcion(static_cast<int>(i)), radioClic))
            {
                motor_.agregarBichito(GENES[i]);
                selectorAbierto_ = false;
                return true;
            }
        }
    }

    if (contieneLogo(x, y, sf::Vector2f(X_MAS, Y_LOGOS), radioClic))
    {
        selectorAbierto_ = !selectorAbierto_;
        return true;
    }

    for (GenEstatico gen : GENES)
    {
        if (motor_.tieneGenRegistrado(gen) &&
            contieneLogo(x, y, sf::Vector2f(xLogoVisible(gen), Y_LOGOS), radioClic))
        {
            selectorAbierto_ = false;
            motor_.abrirArbolGenetico(gen);
            return true;
        }
    }

    if (selectorAbierto_)
    {
        selectorAbierto_ = false;
        return x >= 0.0f && x < Config::ANCHO_MUNDO &&
               y >= 0.0f && y < Config::ALTO_MUNDO;
    }

    return false;
}

bool PanelGenes::cerrarSelector()
{
    if (!selectorAbierto_)
        return false;
    selectorAbierto_ = false;
    return true;
}

void PanelGenes::dibujar(sf::RenderTarget& destino) const
{
    if (motor_.arbolGeneticoAbierto())
        return;

    for (GenEstatico gen : GENES)
    {
        if (!motor_.tieneGenRegistrado(gen))
            continue;
        const sf::Vector2f centro(xLogoVisible(gen), Y_LOGOS);
        const bool hover = posicionMouseConocida_ &&
                           contieneLogo(posicionMouse_.x, posicionMouse_.y, centro, RADIO_LOGO);
        dibujarLogo(destino, gen, centro, RADIO_LOGO, hover);
    }

    const sf::Vector2f centroMas(X_MAS, Y_LOGOS);
    const bool hoverMas = posicionMouseConocida_ &&
                          contieneLogo(posicionMouse_.x, posicionMouse_.y,
                                       centroMas, RADIO_LOGO);
    dibujarLogo(destino, GenEstatico::NINGUNO, centroMas, RADIO_LOGO, hoverMas);

    if (!selectorAbierto_)
        return;

    sf::RectangleShape fondo(sf::Vector2f(ANCHO_SELECTOR, ALTO_SELECTOR));
    fondo.setPosition(X_SELECTOR, Y_SELECTOR);
    fondo.setFillColor(sf::Color(12, 18, 30, 245));
    fondo.setOutlineThickness(1.5f);
    fondo.setOutlineColor(sf::Color(100, 155, 185));
    destino.draw(fondo);

    for (std::size_t i = 0; i < GENES.size(); ++i)
    {
        const sf::Vector2f centro = posicionOpcion(static_cast<int>(i));
        const bool hover = posicionMouseConocida_ &&
                           contieneLogo(posicionMouse_.x, posicionMouse_.y, centro, RADIO_LOGO + 2.0f);
        dibujarLogo(destino, GENES[i], centro, 13.0f, hover);

        sf::Text nombre = crearTexto(nombreGen(GENES[i]), recursos_.fuente(), 8,
                                     sf::Color(220, 230, 242));
        const sf::FloatRect limites = nombre.getLocalBounds();
        nombre.setPosition(centro.x - (limites.left + limites.width * 0.5f),
                           centro.y + 15.0f);
        destino.draw(nombre);
    }
}
