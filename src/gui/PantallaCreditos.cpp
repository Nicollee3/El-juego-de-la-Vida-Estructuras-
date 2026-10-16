#include "gui/PantallaCreditos.h"
#include "gui/Aplicacion.h"
#include "gui/Config.h"

PantallaCreditos::PantallaCreditos(Aplicacion& app)
    : Pantalla(app),
      titulo_(app.recursos(), "Créditos", 64,
              sf::Vector2f(Config::ANCHO_VENTANA / 2.0f, 110.0f), Etiqueta::Alineacion::Centro)
{
    const float cx = Config::ANCHO_VENTANA / 2.0f;

    // EDITA ESTAS LÍNEAS con los nombres reales del equipo.
    const char* textos[] = {
        "Simulador de vida con estructuras de datos",
        "Karoll Ramírez",
        "Nicolle Ruiz",
        "Miguel Cadavid",
        "Hecho con C++ y SFML 2.6.1"};

    float y = 240.0f;
    for (const char* t : textos)
    {
        lineas_.emplace_back(app.recursos(), t, 24, sf::Vector2f(cx, y), Etiqueta::Alineacion::Centro);
        y += 56.0f;
    }

    volver_ = std::make_unique<Boton>(app.recursos(), "Volver", sf::Vector2f(cx, 580),
                                      [this] { app_.irA(EstadoApp::Menu); });
}

void PantallaCreditos::procesarEvento(const sf::Event& evento)
{
    volver_->procesarEvento(evento);
    if (evento.type == sf::Event::KeyPressed && evento.key.code == sf::Keyboard::Escape)
        app_.irA(EstadoApp::Menu);
}

void PantallaCreditos::actualizar(float) {}

void PantallaCreditos::dibujar(sf::RenderTarget& destino)
{
    app_.recursos().dibujarFondo(destino, "fondos/fondo_menu", sf::Vector2u(320, 180),
                                 sf::Color(24, 28, 48), sf::Vector2f(0, 0),
                                 sf::Vector2f(static_cast<float>(Config::ANCHO_VENTANA),
                                              static_cast<float>(Config::ALTO_VENTANA)));
    titulo_.dibujar(destino);
    for (const auto& l : lineas_)
        l.dibujar(destino);
    volver_->dibujar(destino);
}
