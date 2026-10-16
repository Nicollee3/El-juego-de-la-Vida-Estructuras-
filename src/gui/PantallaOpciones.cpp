#include "gui/PantallaOpciones.h"
#include "gui/Aplicacion.h"
#include "gui/Config.h"
#include <string>

PantallaOpciones::PantallaOpciones(Aplicacion& app)
    : Pantalla(app),
      titulo_(app.recursos(), "Opciones", 64,
              sf::Vector2f(Config::ANCHO_VENTANA / 2.0f, 110.0f), Etiqueta::Alineacion::Centro),
      etAgentes_(app.recursos(), "", 24,
                 sf::Vector2f(Config::ANCHO_VENTANA / 2.0f, 260.0f), Etiqueta::Alineacion::Centro),
      etEstacion_(app.recursos(), "", 24,
                  sf::Vector2f(Config::ANCHO_VENTANA / 2.0f, 380.0f), Etiqueta::Alineacion::Centro),
      nota_(app.recursos(), "Los cambios se aplican al iniciar la simulación", 16,
            sf::Vector2f(Config::ANCHO_VENTANA / 2.0f, 470.0f), Etiqueta::Alineacion::Centro)
{
    const float cx = Config::ANCHO_VENTANA / 2.0f;
    const sf::Vector2u mini(16, 16); // botones cuadrados pequeños: imagen de 16x16 -> 64x64 en pantalla

    // Fila 1: agentes del escenario 1  [-]  texto  [+]
    botones_.push_back(std::make_unique<Boton>(app.recursos(), "-", sf::Vector2f(cx - 280, 260),
        [this] {
            Ajustes& a = app_.ajustes();
            if (a.agentesCompetencia > 1) --a.agentesCompetencia;
            refrescar();
        }, "ui/boton_mini", mini));
    botones_.push_back(std::make_unique<Boton>(app.recursos(), "+", sf::Vector2f(cx + 280, 260),
        [this] {
            Ajustes& a = app_.ajustes();
            if (a.agentesCompetencia < 20) ++a.agentesCompetencia;
            refrescar();
        }, "ui/boton_mini", mini));

    // Fila 2: duración de la estación (de 5 en 5 segundos)
    botones_.push_back(std::make_unique<Boton>(app.recursos(), "-", sf::Vector2f(cx - 280, 380),
        [this] {
            Ajustes& a = app_.ajustes();
            if (a.duracionEstacion > 5.0f) a.duracionEstacion -= 5.0f;
            refrescar();
        }, "ui/boton_mini", mini));
    botones_.push_back(std::make_unique<Boton>(app.recursos(), "+", sf::Vector2f(cx + 280, 380),
        [this] {
            Ajustes& a = app_.ajustes();
            if (a.duracionEstacion < 60.0f) a.duracionEstacion += 5.0f;
            refrescar();
        }, "ui/boton_mini", mini));

    botones_.push_back(std::make_unique<Boton>(app.recursos(), "Volver", sf::Vector2f(cx, 580),
        [this] { app_.irA(EstadoApp::Menu); }));

    refrescar();
}

void PantallaOpciones::refrescar()
{
    const Ajustes& a = app_.ajustes();
    etAgentes_.setTexto("Agentes en escenario 1: " + std::to_string(a.agentesCompetencia));
    etEstacion_.setTexto("Duración de estación: " + std::to_string(static_cast<int>(a.duracionEstacion)) + " s");
}

void PantallaOpciones::procesarEvento(const sf::Event& evento)
{
    for (auto& b : botones_)
        b->procesarEvento(evento);

    if (evento.type == sf::Event::KeyPressed && evento.key.code == sf::Keyboard::Escape)
        app_.irA(EstadoApp::Menu);
}

void PantallaOpciones::actualizar(float) {}

void PantallaOpciones::dibujar(sf::RenderTarget& destino)
{
    app_.recursos().dibujarFondo(destino, "fondos/fondo_menu", sf::Vector2u(320, 180),
                                 sf::Color(24, 28, 48), sf::Vector2f(0, 0),
                                 sf::Vector2f(static_cast<float>(Config::ANCHO_VENTANA),
                                              static_cast<float>(Config::ALTO_VENTANA)));
    titulo_.dibujar(destino);
    etAgentes_.dibujar(destino);
    etEstacion_.dibujar(destino);
    nota_.dibujar(destino);
    for (const auto& b : botones_)
        b->dibujar(destino);
}
