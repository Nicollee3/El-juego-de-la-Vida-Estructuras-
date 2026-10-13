/*
 * @file PantallaMenu.h
 * @brief Menú principal: Iniciar simulación, Opciones, Créditos y Salir.
 */

#ifndef GUI_PANTALLA_MENU_H
#define GUI_PANTALLA_MENU_H

#include <memory>
#include <vector>
#include "gui/Boton.h"
#include "gui/Etiqueta.h"
#include "gui/Pantalla.h"

class PantallaMenu : public Pantalla
{
public:
    explicit PantallaMenu(Aplicacion& app);

    void procesarEvento(const sf::Event& evento) override;
    void actualizar(float dt) override;
    void dibujar(sf::RenderTarget& destino) override;

private:
    void moverFoco(int delta);
    void actualizarFoco();

    Etiqueta titulo_;
    Etiqueta ayuda_;
    std::vector<std::unique_ptr<Boton>> botones_;
    int enfocado_; // botón resaltado con el teclado (-1 = ninguno)
};

#endif
