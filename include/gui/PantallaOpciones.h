/*
 * @file PantallaOpciones.h
 * @brief Ajustes: cantidad de agentes del Escenario 1 y duración de cada estación.
 */

#ifndef GUI_PANTALLA_OPCIONES_H
#define GUI_PANTALLA_OPCIONES_H

#include <memory>
#include <vector>
#include "gui/Boton.h"
#include "gui/Etiqueta.h"
#include "gui/Pantalla.h"

class PantallaOpciones : public Pantalla
{
public:
    explicit PantallaOpciones(Aplicacion& app);

    void procesarEvento(const sf::Event& evento) override;
    void actualizar(float dt) override;
    void dibujar(sf::RenderTarget& destino) override;

private:
    void refrescar(); // actualiza los textos con los valores actuales

    Etiqueta titulo_;
    Etiqueta etAgentes_;
    Etiqueta etEstacion_;
    Etiqueta nota_;
    std::vector<std::unique_ptr<Boton>> botones_;
};

#endif
