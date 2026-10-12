#ifndef MOTOR_GRAFICO_H
#define MOTOR_GRAFICO_H

#include <SFML/Graphics.hpp>
#include <vector>
#include "PilaDinamica.h"
#include "ColaConPrioridad.h"
#include "ColaCircular.h"

enum class ModoSimulacion { Normal, EscenarioCompetencia, EscenarioReproduccion };

class MotorGrafico
{
private:
    sf::RenderWindow ventana;
    PilaDinamica& poblacion;
    ColaConPrioridad colaComida;
    std::vector<Comida> comidasActivas;
    ColaCircular estaciones;

    float tiempoCambioEstacion;
    float temporizadorEstacion;
    ModoSimulacion modoActual;

    // Métodos de actualización de física y lógica
    void actualizarFisica(float dt);
    void procesarEscenarioCompetencia(float dt);
    void procesarEscenarioReproduccion(float dt);
    void procesarSimulacionGeneral(float dt);

    void renderizar();

public:
    MotorGrafico(PilaDinamica& _poblacion, unsigned int ancho = 1000, unsigned int alto = 700);
    
    void ejecutar();
    void agregarComida(float x, float y, int prioridad = 1, int puntosVida = 30);
    void cargarEscenarioCompetencia(int nAgentes);
    void cargarEscenarioReproduccion();
};

#endif