/*
 * @file PantallaSimulacion.h
 * @brief Pantalla de la simulación: une el motor (mundo) con la barra lateral.
 * @details Es la dueña de la población (ArregloDinamico) y del MotorGrafico. Al volver al menú se destruye
 *          y la simulación se reinicia la próxima vez que se entra.
 */

#ifndef GUI_PANTALLA_SIMULACION_H
#define GUI_PANTALLA_SIMULACION_H

#include "MotorGrafico.h"
#include "ArregloDinamico.h"
#include "gui/BarraLateral.h"
#include "gui/Etiqueta.h"
#include "gui/Pantalla.h"

class PantallaSimulacion : public Pantalla
{
public:
    explicit PantallaSimulacion(Aplicacion& app);

    void procesarEvento(const sf::Event& evento) override;
    void actualizar(float dt) override;
    void dibujar(sf::RenderTarget& destino) override;

private:
    // El orden importa: motor_ guarda una referencia a poblacion_, así que poblacion_ va primero.
    ArregloDinamico<Bichito> poblacion_;
    MotorGrafico motor_;
    BarraLateral barra_;
    Etiqueta etPausa_;
};

#endif
