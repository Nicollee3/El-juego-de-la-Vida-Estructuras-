#ifndef MOTOR_GRAFICO_H
#define MOTOR_GRAFICO_H

#include <array>
#include <string>
#include <SFML/Graphics.hpp>
#include "ArregloDinamico.h"
#include "Cola.h"
#include "ColaCircular.h"
#include "Diccionario.h"

class GestorRecursos; // declaración adelantada

enum class ModoSimulacion { Normal, EscenarioCompetencia, EscenarioReproduccion };

class MotorGrafico
{
private:
    // CAMBIO: ya no existe "sf::RenderWindow ventana". La ventana ahora es de la clase Aplicacion.
    // El motor solo necesita saber qué tan grande es el "mundo" donde viven los bichitos.
    sf::Vector2f tamanoMundo;
    ArregloDinamico<Bichito>& poblacion;
    ArregloDinamico<Comida> comidasActivas;
    Diccionario<int> indicesBichitos;
    Diccionario<int> indicesComida;
    Cola<int> colaEliminacion;
    ColaCircular estaciones;
    std::array<int, 10> bolsaComida;
    int indiceBolsaComida;

    float tiempoCambioEstacion;
    float temporizadorEstacion;
    ModoSimulacion modoActual;

    float temporizadorComida;
    float intervaloComida;

    // Estado que controla la barra lateral
    bool pausado;
    float velocidadSim;     // multiplicador del tiempo (0.5x ... 4x)
    int prioridadComida;    // prioridad (1..3) de la comida que se coloca con clic
    int agentesCompetencia; // bichitos del Escenario 1

    // Métodos de actualización de física y lógica
    void actualizarFisica(float dt);
    void procesarEscenarioCompetencia(float dt);
    void procesarEscenarioReproduccion(float dt);
    void procesarSimulacionGeneral(float dt);
    void encolarCondenados();
    void procesarColaEliminacion(int elementosPendientes);
    void agregarBichitoAlFinal(Bichito bichito);
    void eliminarBichitoEn(int indice, bool retirarDeCola = true);
    void vaciarPoblacion();
    void eliminarComidaEn(int indice);
    void vaciarComidas();

    void mezclarBolsaComida();
    int obtenerRarezaSiguiente();
    float calcularIntervaloComida(const std::string& nombreEstacion);

public:
    // CAMBIO: recibe el tamaño del mundo en vez de crear la ventana.
    MotorGrafico(ArregloDinamico<Bichito>& _poblacion, sf::Vector2f _tamanoMundo);

    // CAMBIO: desaparece ejecutar(); su trabajo se reparte en estos tres métodos
    // que llama PantallaSimulacion en cada frame.
    void procesarEvento(const sf::Event& evento);
    void actualizar(float dt);
    void dibujar(sf::RenderTarget& destino, GestorRecursos& recursos);

    void agregarComida(float x, float y, int prioridad = 1, int puntosVida = 30,
                       OrigenComida origen = OrigenComida::Pool);
    // CAMBIO: si no se indica nAgentes (o es negativo) se usa el valor configurado.
    void cargarEscenarioCompetencia(int nAgentes = -1);
    void cargarEscenarioReproduccion();
    void cargarSimulacionGeneral(); // Inicia el modo normal pausado con bichitos aleatorios

    // ---------- NUEVO: control desde la barra lateral ----------
    void setPausa(bool p) { pausado = p; }
    bool estaPausado() const { return pausado; }

    void setVelocidad(float v);
    float getVelocidad() const { return velocidadSim; }

    void setPrioridadComida(int p);
    int getPrioridadComida() const { return prioridadComida; }

    void setAgentesCompetencia(int n) { if (n >= 1) agentesCompetencia = n; }
    void setDuracionEstacion(float segundos) { if (segundos > 0.0f) tiempoCambioEstacion = segundos; }

    void agregarBichito();
    void quitarBichito();

    // ---------- NUEVO: información para mostrar ----------
    ModoSimulacion getModo() const { return modoActual; }
    Estacion getEstacionActual() { return estaciones.obtener_actual(); } // no es const: obtener_actual() no lo es
    int getNumBichitos() { return poblacion.longitud(); }
    int getNumComidas() const { return comidasActivas.longitud(); }
    int getCapacidadPoblacion() { return poblacion.capacidad(); }
};

#endif
