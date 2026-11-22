#ifndef GUI_PANEL_GENES_H
#define GUI_PANEL_GENES_H

#include <SFML/Graphics.hpp>
#include "Genoma.h"

class GestorRecursos;
class MotorGrafico;

class PanelGenes
{
public:
    PanelGenes(GestorRecursos& recursos, MotorGrafico& motor);

    bool procesarEvento(const sf::Event& evento);
    bool cerrarSelector();
    void dibujar(sf::RenderTarget& destino) const;

private:
    bool selectorAbierto_;
    bool posicionMouseConocida_;
    sf::Vector2f posicionMouse_;
    GestorRecursos& recursos_;
    MotorGrafico& motor_;

    float xLogoVisible(GenEstatico gen) const;
    sf::Vector2f posicionOpcion(int indice) const;
    bool contieneLogo(float x, float y, sf::Vector2f centro, float radio) const;
    void dibujarLogo(sf::RenderTarget& destino, GenEstatico gen,
                     sf::Vector2f centro, float radio, bool resaltado) const;
};

#endif
