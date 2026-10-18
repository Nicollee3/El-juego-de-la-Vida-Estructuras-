/*
 * @file Aplicacion.h
 * @brief Dueña de la ventana y del bucle principal; decide qué pantalla está activa.
 */

#ifndef GUI_APLICACION_H
#define GUI_APLICACION_H

#include <SFML/Graphics.hpp>
#include <memory>
#include "gui/Ajustes.h"
#include "gui/GestorRecursos.h"
#include "gui/Pantalla.h"

class Aplicacion
{
public:
    Aplicacion();

    void ejecutar();            // bucle principal: eventos -> actualizar -> dibujar
    void irA(EstadoApp nuevo);  // solo ANOTA el cambio; se aplica al terminar de procesar eventos

    GestorRecursos& recursos() { return recursos_; }
    Ajustes& ajustes() { return ajustes_; }

private:
    void crearPantalla(EstadoApp estado);

    //La pantalla debe morir antes que los recursos,
    // y los recursos (texturas) antes que la ventana.
    sf::RenderWindow ventana_;
    GestorRecursos recursos_;
    Ajustes ajustes_;
    std::unique_ptr<Pantalla> actual_;
    EstadoApp estado_;
    EstadoApp pendiente_;
};

#endif
