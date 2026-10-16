/*
 * @file Pantalla.h
 * @brief Clase base de todas las pantallas (menú, simulación, opciones, créditos).
 * @details Cada pantalla sabe: reaccionar a eventos, actualizarse con el tiempo (dt) y dibujarse.
 *          La Aplicacion solo tiene una pantalla activa a la vez.
 */

#ifndef GUI_PANTALLA_H
#define GUI_PANTALLA_H

#include <SFML/Graphics.hpp>

enum class EstadoApp { Menu, Simulacion, Opciones, Creditos, Salir };

class Aplicacion;

class Pantalla
{
public:
    explicit Pantalla(Aplicacion& app) : app_(app) {}
    virtual ~Pantalla() = default;

    virtual void procesarEvento(const sf::Event& evento) = 0; // mouse, teclado...
    virtual void actualizar(float dt) = 0;                     // dt = segundos desde el frame anterior
    virtual void dibujar(sf::RenderTarget& destino) = 0;       // pintar en la ventana

protected:
    Aplicacion& app_; // para pedir cambios de pantalla: app_.irA(EstadoApp::Menu)
};

#endif
