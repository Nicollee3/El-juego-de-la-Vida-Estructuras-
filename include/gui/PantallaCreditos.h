/*
 * @file PantallaCreditos.h
 * @brief Pantalla de créditos con un botón para volver al menú.
 */

#ifndef GUI_PANTALLA_CREDITOS_H
#define GUI_PANTALLA_CREDITOS_H

#include <memory>
#include <vector>
#include "gui/Boton.h"
#include "gui/Etiqueta.h"
#include "gui/Pantalla.h"

class PantallaCreditos : public Pantalla
{
public:
    explicit PantallaCreditos(Aplicacion& app);

    void procesarEvento(const sf::Event& evento) override;
    void actualizar(float dt) override;
    void dibujar(sf::RenderTarget& destino) override;

private:
    Etiqueta titulo_;
    std::vector<Etiqueta> lineas_;
    std::unique_ptr<Boton> volver_;
};

#endif
