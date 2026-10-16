/*
 * @file BarraLateral.h
 * @brief Panel derecho de la simulación: información en vivo y controles.
 * @details Ocupa el rectángulo x = 1000..1280. Se comunica con la simulación únicamente
 *          mediante los métodos públicos de MotorGrafico.
 */

#ifndef GUI_BARRA_LATERAL_H
#define GUI_BARRA_LATERAL_H

#include <SFML/Graphics.hpp>
#include <functional>
#include <memory>
#include <vector>
#include "gui/Boton.h"
#include "gui/Etiqueta.h"

class GestorRecursos;
class MotorGrafico;

class BarraLateral
{
public:
    BarraLateral(GestorRecursos& recursos, MotorGrafico& motor, std::function<void()> alVolverAlMenu);

    void procesarEvento(const sf::Event& evento);
    void actualizar();                              // refresca textos y estados de botones
    void dibujar(sf::RenderTarget& destino) const;

private:
    Boton* crearBoton(const std::string& texto, sf::Vector2f centro, std::function<void()> alClic,
                      const std::string& estilo = "ui/boton",
                      sf::Vector2u tamNativo = sf::Vector2u(64, 16));
    void cambiarVelocidad(int delta);

    GestorRecursos& recursos_;
    MotorGrafico& motor_;
    std::function<void()> alVolverAlMenu_;

    std::vector<std::unique_ptr<Boton>> botones_;
    Boton* btnPausa_;
    Boton* btnPrioridad_[3];

    Etiqueta titulo_;
    Etiqueta lineaEstacion_;
    Etiqueta lineaBichitos_;
    Etiqueta lineaComida_;
    Etiqueta lineaArreglo_;
    Etiqueta etVelocidad_;
    Etiqueta etBichitos_;
    Etiqueta etPrioridad_;
    Etiqueta etAyuda_;

    int indiceVelocidad_;
};

#endif
