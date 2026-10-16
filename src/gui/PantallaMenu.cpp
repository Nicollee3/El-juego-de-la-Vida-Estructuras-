#include "gui/PantallaMenu.h"
#include "gui/Aplicacion.h"
#include "gui/Config.h"

PantallaMenu::PantallaMenu(Aplicacion& app)
    : Pantalla(app),
      titulo_(app.recursos(), "El Juego de la Vida", 64,
              sf::Vector2f(Config::ANCHO_VENTANA / 2.0f, 150.0f), Etiqueta::Alineacion::Centro),
      ayuda_(app.recursos(), "Usa el mouse o las flechas y Enter", 16,
             sf::Vector2f(Config::ANCHO_VENTANA / 2.0f, 690.0f), Etiqueta::Alineacion::Centro),
      enfocado_(-1)
{
    const float cx = Config::ANCHO_VENTANA / 2.0f;
    const float y0 = 300.0f;
    const float paso = 80.0f; // 64 px de alto del botón + 16 px de separación

    // Cada botón recibe una "lambda": una función pequeña que se ejecuta al hacer clic.
    botones_.push_back(std::make_unique<Boton>(app.recursos(), "Iniciar simulación",
        sf::Vector2f(cx, y0), [this] { app_.irA(EstadoApp::Simulacion); }));
    botones_.push_back(std::make_unique<Boton>(app.recursos(), "Opciones",
        sf::Vector2f(cx, y0 + paso), [this] { app_.irA(EstadoApp::Opciones); }));
    botones_.push_back(std::make_unique<Boton>(app.recursos(), "Créditos",
        sf::Vector2f(cx, y0 + 2 * paso), [this] { app_.irA(EstadoApp::Creditos); }));
    botones_.push_back(std::make_unique<Boton>(app.recursos(), "Salir",
        sf::Vector2f(cx, y0 + 3 * paso), [this] { app_.irA(EstadoApp::Salir); }));
}

void PantallaMenu::moverFoco(int delta)
{
    const int n = static_cast<int>(botones_.size());
    if (enfocado_ < 0)
        enfocado_ = (delta > 0) ? 0 : n - 1;
    else
        enfocado_ = (enfocado_ + delta + n) % n;
    actualizarFoco();
}

void PantallaMenu::actualizarFoco()
{
    for (int i = 0; i < static_cast<int>(botones_.size()); ++i)
        botones_[i]->setEnfocado(i == enfocado_);
}

void PantallaMenu::procesarEvento(const sf::Event& evento)
{
    // Si el usuario usa el mouse, el foco del teclado desaparece (para no ver dos botones iluminados).
    if (evento.type == sf::Event::MouseMoved && enfocado_ != -1)
    {
        enfocado_ = -1;
        actualizarFoco();
    }

    for (auto& b : botones_)
        b->procesarEvento(evento);

    if (evento.type == sf::Event::KeyPressed)
    {
        if (evento.key.code == sf::Keyboard::Down)
            moverFoco(+1);
        else if (evento.key.code == sf::Keyboard::Up)
            moverFoco(-1);
        else if ((evento.key.code == sf::Keyboard::Return || evento.key.code == sf::Keyboard::Space) &&
                 enfocado_ >= 0)
            botones_[enfocado_]->activar();
    }
}

void PantallaMenu::actualizar(float) {}

void PantallaMenu::dibujar(sf::RenderTarget& destino)
{
    app_.recursos().dibujarFondo(destino, "fondos/fondo_menu", sf::Vector2u(320, 180),
                                 sf::Color(24, 28, 48), sf::Vector2f(0, 0),
                                 sf::Vector2f(static_cast<float>(Config::ANCHO_VENTANA),
                                              static_cast<float>(Config::ALTO_VENTANA)));
    titulo_.dibujar(destino);
    for (const auto& b : botones_)
        b->dibujar(destino);
    ayuda_.dibujar(destino);
}
