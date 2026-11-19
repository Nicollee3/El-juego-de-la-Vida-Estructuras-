/*
 * @file PantallaMenu.h
 * @brief Menú principal: Iniciar simulación, Opciones, Créditos y Salir.
 */

#ifndef GUI_PANTALLA_MENU_H
#define GUI_PANTALLA_MENU_H

#include <memory>
#include <vector>
#include <SFML/Graphics.hpp>
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
    struct ParticulaFondo
    {
        float x = 0.0f;
        float y = 0.0f;
        float vx = 0.0f;
        float vy = 0.0f;
        float radio = 6.0f;
        float fase = 0.0f;
        sf::Color color;
        bool esComida = false;
    };

    void moverFoco(int delta);
    void actualizarFoco();
    void inicializarFondoAnimado();

    Etiqueta titulo_;
    Etiqueta subtitulo_;
    Etiqueta ayuda_;
    std::vector<std::unique_ptr<Boton>> botones_;
    std::vector<ParticulaFondo> particulas_;
    float tiempo_;
    int enfocado_; // botón resaltado con el teclado (-1 = ninguno)
};

#endif
