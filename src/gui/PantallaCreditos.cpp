#include "gui/PantallaCreditos.h"
#include "gui/Aplicacion.h"
#include "gui/Config.h"
#include <cmath>

PantallaCreditos::PantallaCreditos(Aplicacion& app)
    : Pantalla(app),
      titulo_(app.recursos(), "CRÉDITOS", 56,
              sf::Vector2f(Config::ANCHO_VENTANA / 2.0f, 96.0f), Etiqueta::Alineacion::Centro),
      tiempo_(0.0f)
{
    titulo_.setColor(sf::Color(235, 250, 255));

    const float cx = Config::ANCHO_VENTANA / 2.0f;
    const float cxTexto = cx - 120.0f;

    const char* textos[] = {
        "Simulador de vida con estructuras de datos",
        "Karoll Ramírez",
        "Nicolle Ruiz",
        "Miguel Cadavid",
        "Hecho con C++ y SFML 2.6.1"};

    float y = 242.0f;
    for (int i = 0; i < 5; ++i)
    {
        const unsigned int tam = (i == 0 || i == 4) ? 20u : 26u;
        lineas_.emplace_back(app.recursos(), textos[i], tam,
                             sf::Vector2f(cxTexto, y), Etiqueta::Alineacion::Centro);
        if (i == 0)
            lineas_.back().setColor(sf::Color(115, 225, 200));
        else if (i == 4)
            lineas_.back().setColor(sf::Color(170, 190, 215));
        else
            lineas_.back().setColor(sf::Color(245, 248, 255));
        y += 54.0f;
    }

    volver_ = std::make_unique<Boton>(app.recursos(), "Volver", sf::Vector2f(cx, 622.0f),
                                      [this] { app_.irA(EstadoApp::Menu); },
                                      "ui/boton_menu_principal", sf::Vector2u(72, 14));
}

void PantallaCreditos::procesarEvento(const sf::Event& evento)
{
    volver_->procesarEvento(evento);
    if (evento.type == sf::Event::KeyPressed && evento.key.code == sf::Keyboard::Escape)
        app_.irA(EstadoApp::Menu);
}

void PantallaCreditos::actualizar(float dt)
{
    tiempo_ += dt;
}

void PantallaCreditos::dibujar(sf::RenderTarget& destino)
{
    const float ancho = static_cast<float>(Config::ANCHO_VENTANA);
    const float alto = static_cast<float>(Config::ALTO_VENTANA);
    const float cx = ancho / 2.0f;

    // 1. Fondo con gradiente nocturno a juego con el menú principal
    sf::VertexArray fondo(sf::Quads, 4);
    fondo[0] = sf::Vertex(sf::Vector2f(0.0f, 0.0f), sf::Color(11, 18, 34));
    fondo[1] = sf::Vertex(sf::Vector2f(ancho, 0.0f), sf::Color(15, 24, 46));
    fondo[2] = sf::Vertex(sf::Vector2f(ancho, alto), sf::Color(16, 46, 56));
    fondo[3] = sf::Vertex(sf::Vector2f(0.0f, alto), sf::Color(12, 36, 48));
    destino.draw(fondo);

    // 2. Tarjeta central que enmarca los créditos y al perrito
    const sf::Vector2f tamTarjeta(920.0f, 360.0f);
    sf::RectangleShape tarjeta(tamTarjeta);
    tarjeta.setOrigin(tamTarjeta.x / 2.0f, tamTarjeta.y / 2.0f);
    tarjeta.setPosition(cx, 355.0f);
    tarjeta.setFillColor(sf::Color(15, 24, 40, 205));
    tarjeta.setOutlineThickness(2.5f);
    tarjeta.setOutlineColor(sf::Color(75, 205, 195, 150));
    destino.draw(tarjeta);

    // 3. Perrito pixel-art navideño saltando feliz a la derecha de los créditos
    const sf::Texture& texPerrito = app_.recursos().textura(
        "ui/perrito_creditos", sf::Vector2u(64, 64), sf::Color(255, 255, 255), Forma::Rectangulo);
    const float px = cx + 275.0f;
    const float salto = std::abs(std::sin(tiempo_ * 3.2f)) * 12.0f;
    const float py = 360.0f - salto;

    // Sombra suave debajo del perrito
    sf::CircleShape sombra(54.0f);
    sombra.setOrigin(54.0f, 54.0f);
    sombra.setScale(1.1f, 0.28f);
    sombra.setPosition(px, 462.0f);
    sombra.setFillColor(sf::Color(0, 0, 0, 110));
    destino.draw(sombra);

    sf::Sprite spritePerrito(texPerrito);
    spritePerrito.setOrigin(texPerrito.getSize().x / 2.0f, texPerrito.getSize().y / 2.0f);
    const float alturaDeseada = 200.0f;
    const float escala = alturaDeseada / static_cast<float>(std::max(1u, texPerrito.getSize().y));
    spritePerrito.setScale(escala, escala);
    spritePerrito.setPosition(std::round(px), std::round(py));
    destino.draw(spritePerrito);

    // 4. Textos y botón volver
    titulo_.dibujar(destino);
    for (const auto& l : lineas_)
        l.dibujar(destino);
    volver_->dibujar(destino);
}
