#ifndef GUI_PANEL_GENETICA_H
#define GUI_PANEL_GENETICA_H

#include "ArbolGenealogico.h"
#include "Bichito.h"
#include <SFML/Graphics.hpp>

class GestorRecursos;

class PanelGenetica
{
public:
    PanelGenetica();
    void seleccionar(const Bichito& bichito);
    void actualizarSeleccion(const Bichito& bichito);
    void marcarFallecido(int id);
    void cerrar();
    bool cerrarArbol();
    bool arbolAbierto() const { return arbolActivo; }
    bool procesarEvento(const sf::Event& evento, RegistroGenetico& registro);
    void dibujar(sf::RenderTarget& destino, GestorRecursos& recursos,
                 RegistroGenetico& registro);

private:
    bool visible;
    bool arbolActivo;
    int idSeleccionado;
    int generacion;
    bool vivo;
    float vida;
    float edad;
    Genoma genoma;
    EstadoBichito estado;
    GenEstatico genMostrado;
    int indiceFundador;
    float zoom;
    sf::Vector2f desplazamiento;
    bool arrastrando;
    sf::Vector2f ultimoMouse;

    void dibujarTarjeta(sf::RenderTarget& destino, GestorRecursos& recursos);
    void dibujarArbol(sf::RenderTarget& destino, GestorRecursos& recursos,
                      RegistroGenetico& registro);
};

#endif
