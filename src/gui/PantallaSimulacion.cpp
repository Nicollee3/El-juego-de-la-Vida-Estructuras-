#include "gui/PantallaSimulacion.h"
#include "gui/Aplicacion.h"
#include "gui/Config.h"

PantallaSimulacion::PantallaSimulacion(Aplicacion& app)
    : Pantalla(app),
      poblacion_(),
      motor_(poblacion_, sf::Vector2f(Config::ANCHO_MUNDO, Config::ALTO_MUNDO)),
      barra_(app.recursos(), motor_, [this] { app_.irA(EstadoApp::Menu); }),
      etPausa_(app.recursos(), "PAUSA", 32, sf::Vector2f(Config::ANCHO_MUNDO / 2.0f, 40.0f),
               Etiqueta::Alineacion::Centro)
{
    // Aplicamos lo que el usuario eligió en Opciones y arrancamos con el Escenario 1.
    motor_.setAgentesCompetencia(app.ajustes().agentesCompetencia);
    motor_.setDuracionEstacion(app.ajustes().duracionEstacion);
    motor_.cargarEscenarioCompetencia();
}

void PantallaSimulacion::procesarEvento(const sf::Event& evento)
{
    if (evento.type == sf::Event::KeyPressed)
    {
        if (evento.key.code == sf::Keyboard::Escape)
        {
            app_.irA(EstadoApp::Menu);
            return;
        }
        if (evento.key.code == sf::Keyboard::Space)
            motor_.setPausa(!motor_.estaPausado());
    }

    barra_.procesarEvento(evento);  // botones de la barra
    motor_.procesarEvento(evento);  // teclas 1/2/R y clic para colocar comida
}

void PantallaSimulacion::actualizar(float dt)
{
    motor_.actualizar(dt);
    barra_.actualizar();
}

void PantallaSimulacion::dibujar(sf::RenderTarget& destino)
{
    motor_.dibujar(destino, app_.recursos());
    if (motor_.estaPausado())
        etPausa_.dibujar(destino);
    barra_.dibujar(destino);
}
