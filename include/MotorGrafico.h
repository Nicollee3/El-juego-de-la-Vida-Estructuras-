#ifndef MOTOR_GRAFICO_H
#define MOTOR_GRAFICO_H

#include <array>
#include <string>
#include <vector>
#include <SFML/Graphics.hpp>
#include "ArregloDinamico.h"
#include "Cola.h"
#include "ColaCircular.h"
#include "Diccionario.h"
#include "ArbolGenealogico.h"
#include "MotorDesastres.h"
#include "RTree.h"
#include "gui/PanelGenetica.h"

class GestorRecursos; // declaración adelantada

enum class ModoSimulacion { Normal, EscenarioCompetencia, EscenarioReproduccion };

class MotorGrafico
{
private:
    // CAMBIO: ya no existe "sf::RenderWindow ventana". La ventana ahora es de la clase Aplicacion.
    // El motor solo necesita saber qué tan grande es el "mundo" donde viven los bichitos.
    sf::Vector2f tamanoMundo;
    RTree rtreeComida;
    ArregloDinamico<Bichito>& poblacion;
    ArregloDinamico<Comida> comidasActivas;
    Diccionario<int> indicesBichitos;
    Diccionario<int> indicesComida;
    Cola<int> colaEliminacion;
    ColaPrioridadVejez colaVejez;
    RegistroGenetico registroGenetico;
    PanelGenetica panelGenetica;
    ColaCircular estaciones;
    MotorDesastres motorDesastres;
    float tiempoSimulacion;
    int muertesHambre;
    int muertesVejez;
    std::array<int, 10> bolsaComida;
    int indiceBolsaComida;

    float tiempoCambioEstacion;
    float temporizadorEstacion;
    ModoSimulacion modoActual;

    float temporizadorComida;
    float intervaloComida;

    // Alerta visual central y efectos ambientales / de desastre
    struct ParticulaClima
    {
        float x = 0.0f;
        float y = 0.0f;
        float vx = 0.0f;
        float vy = 0.0f;
        float tam = 4.0f;
        float rotacion = 0.0f;
        float velRot = 0.0f;
        float fase = 0.0f;
        int variante = 0;
    };

    ResultadoDesastre alertaDesastre;
    float tiempoAlertaDesastre;
    float duracionAlertaDesastre;
    float tiempoVisual;
    std::vector<ParticulaClima> particulasClima;

    // Estado que controla la barra lateral
    bool pausado;
    float velocidadSim;     // multiplicador del tiempo (0.5x ... 4x)
    int prioridadComida;    // prioridad (1..3) de la comida que se coloca con clic
    int agentesCompetencia; // bichitos del Escenario 1
    int bichitosIniciales;  // bichitos iniciales de la simulación general
    int idBichitoSeleccionado; // ID del bichito inspeccionado con clic izquierdo (-1 = ninguno)

    // Métodos de actualización de física y lógica
    void actualizarFisica(float dt);
    void procesarEscenarioCompetencia(float dt);
    void procesarEscenarioReproduccion(float dt);
    void procesarSimulacionGeneral(float dt);
    void encolarCondenados();
    void procesarColaEliminacion(int elementosPendientes);
    void registrarEnColaVejez(const Bichito& bichito);
    void registrarEnArbolGenealogico(const Bichito& bichito,
                                     bool permitirFundadorAdicional = false);
    void intentarMutarGenEstatico(Bichito& bichito);
    void procesarMuertesPorHambre();
    void procesarMuertesPorVejez();
    void agregarBichitoAlFinal(Bichito bichito, bool permitirFundadorAdicional = false);
    void eliminarBichitoEn(int indice, bool retirarDeCola = true);
    void eliminarBichitoPorId(int id);
    void vaciarPoblacion();
    void eliminarComidaEn(int indice);
    void vaciarComidas();

    void mezclarBolsaComida();
    int obtenerRarezaSiguiente();
    float calcularIntervaloComida(const std::string& nombreEstacion);
    void mostrarAlertaDesastre(const ResultadoDesastre& resultado);
    void dibujarAlertaDesastre(sf::RenderTarget& destino, GestorRecursos& recursos) const;
    void inicializarParticulasClima();
    void actualizarEfectosClima(float dt);
    void dibujarEfectosEstacion(sf::RenderTarget& destino, const std::string& estacion) const;
    void dibujarAnimacionDesastre(sf::RenderTarget& destino) const;

public:
    // CAMBIO: recibe el tamaño del mundo en vez de crear la ventana.
    MotorGrafico(ArregloDinamico<Bichito>& _poblacion, sf::Vector2f _tamanoMundo);

    // CAMBIO: desaparece ejecutar(); su trabajo se reparte en estos tres métodos
    // que llama PantallaSimulacion en cada frame.
    void procesarEvento(const sf::Event& evento);
    bool cerrarArbolGenetico() { return panelGenetica.cerrarArbol(); }
    bool arbolGeneticoAbierto() const { return panelGenetica.arbolAbierto(); }
    bool abrirArbolGenetico(GenEstatico gen);
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
    void setBichitosIniciales(int n) { if (n >= 1 && n <= 300) bichitosIniciales = n; }
    void setDuracionEstacion(float segundos) { if (segundos > 0.0f) tiempoCambioEstacion = segundos; }

    void agregarBichito(GenEstatico gen = GenEstatico::NINGUNO);
    bool tieneGenRegistrado(GenEstatico gen) const;
    void quitarBichito();

    // ---------- Desastres Naturales y The Reaper ----------
    ResultadoDesastre activarDesastre(TipoDesastre tipo);
    MotorDesastres& getMotorDesastres() { return motorDesastres; }
    std::string getResumenUltimoDesastre() const { return motorDesastres.getResumenUltimoDesastre(); }

    // ---------- NUEVO: información para mostrar ----------
    ModoSimulacion getModo() const { return modoActual; }
    Estacion getEstacionActual() { return estaciones.obtener_actual(); } // no es const: obtener_actual() no lo es
    int getNumBichitos() { return poblacion.longitud(); }
    int getNumComidas() const { return comidasActivas.longitud(); }
    int getCapacidadPoblacion() { return poblacion.capacidad(); }
    int getMuertesHambre() const { return muertesHambre; }
    int getMuertesVejez() const { return muertesVejez; }
};

#endif