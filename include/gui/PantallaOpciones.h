/*
 * @file PantallaOpciones.h
 * @brief Ajustes: cantidad inicial de bichitos (presets, +/-10 o teclado) y duración de estación.
 */

#ifndef GUI_PANTALLA_OPCIONES_H
#define GUI_PANTALLA_OPCIONES_H

#include <array>
#include <memory>
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>
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
    void refrescar();
    void establecerBichitos(int valor);
    void aplicarTextoEntrada();

    Etiqueta titulo_;
    Etiqueta etBichitosTitulo_;
    Etiqueta etValorBichitos_;
    Etiqueta etPresets_;
    Etiqueta etEstacion_;
    Etiqueta nota_;

    std::vector<std::unique_ptr<Boton>> botones_;
    std::array<Boton*, 5> btnPresets_;

    sf::FloatRect cajaEntrada_;
    std::string textoEntrada_;
    float tiempo_;
};

#endif
