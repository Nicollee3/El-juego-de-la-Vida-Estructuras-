#include "gui/Aplicacion.h"
#include "gui/Config.h"
#include "gui/PantallaCreditos.h"
#include "gui/PantallaMenu.h"
#include "gui/PantallaOpciones.h"
#include "gui/PantallaSimulacion.h"
#include <algorithm>

Aplicacion::Aplicacion()
    : ventana_(sf::VideoMode(Config::ANCHO_VENTANA, Config::ALTO_VENTANA),
               "Simulador de Vida 2D",
               sf::Style::Titlebar | sf::Style::Close), // sin botón de maximizar: tamaño fijo
      estado_(EstadoApp::Menu),
      pendiente_(EstadoApp::Menu)
{
    ventana_.setFramerateLimit(60); // máximo 60 imágenes por segundo
    crearPantalla(EstadoApp::Menu);
}

void Aplicacion::irA(EstadoApp nuevo)
{
    // No se cambia de pantalla aquí mismo: quien llama a irA() es un botón DENTRO de la pantalla
    // actual. Si la destruyéramos ahora, el programa intentaría seguir ejecutando código de un objeto muerto.
    pendiente_ = nuevo;
}

void Aplicacion::crearPantalla(EstadoApp estado)
{
    switch (estado)
    {
    case EstadoApp::Menu:       actual_ = std::make_unique<PantallaMenu>(*this); break;
    case EstadoApp::Simulacion: actual_ = std::make_unique<PantallaSimulacion>(*this); break;
    case EstadoApp::Opciones:   actual_ = std::make_unique<PantallaOpciones>(*this); break;
    case EstadoApp::Creditos:   actual_ = std::make_unique<PantallaCreditos>(*this); break;
    case EstadoApp::Salir:      return;
    }
    estado_ = estado;

    // Le avisamos a la pantalla nueva dónde está el mouse. Sin esto, un botón que quede justo
    // debajo del cursor no se iluminaría hasta que movieras el mouse.
    sf::Event ficticio;
    ficticio.type = sf::Event::MouseMoved;
    const sf::Vector2i pos = sf::Mouse::getPosition(ventana_);
    ficticio.mouseMove.x = pos.x;
    ficticio.mouseMove.y = pos.y;
    actual_->procesarEvento(ficticio);
}

void Aplicacion::ejecutar()
{
    sf::Clock reloj; // cronómetro: restart() devuelve el tiempo desde la última vez

    while (ventana_.isOpen())
    {
        // dt = segundos que duró el frame anterior. Se limita a 0.05 s: si arrastras la ventana en
        // Windows el programa se congela y dt sería enorme, haciendo que los bichitos "salten".
        const float dt = std::min(reloj.restart().asSeconds(), 0.05f);

        // 1) EVENTOS: SFML guarda en una cola todo lo que hace el usuario (mover mouse, clic, tecla...).
        sf::Event evento;
        while (ventana_.pollEvent(evento))
        {
            if (evento.type == sf::Event::Closed)
                ventana_.close();
            else if (evento.type == sf::Event::KeyPressed && evento.key.code == sf::Keyboard::F5)
                recursos_.recargar(); // recarga tus PNG sin cerrar el programa
            else
                actual_->procesarEvento(evento);
        }
        if (!ventana_.isOpen()) break;

        // 2) CAMBIO DE PANTALLA (si algún botón lo pidió)
        if (pendiente_ != estado_)
        {
            if (pendiente_ == EstadoApp::Salir)
            {
                ventana_.close();
                break;
            }
            crearPantalla(pendiente_);
        }

        // 3) ACTUALIZAR la lógica de la pantalla activa
        actual_->actualizar(dt);

        // 4) DIBUJAR: se borra el cuadro anterior, se pinta todo y display() lo muestra en la ventana
        ventana_.clear(sf::Color::Black);
        actual_->dibujar(ventana_);
        ventana_.display();
    }
}
