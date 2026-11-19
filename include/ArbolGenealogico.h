#ifndef ARBOL_GENEALOGICO_H
#define ARBOL_GENEALOGICO_H

#include "Genoma.h"
#include "ArregloDinamico.h"

/**
 * @struct NodoGenealogico
 * @brief Representa a un individuo en la historia familiar (esté vivo o muerto).
 */
struct NodoGenealogico {
    int idBichito;
    bool estadoVida; // true = vivo, false = fallecido (persistencia post-muerte)[cite: 10, 11]
    
    // Atributos base rastreados por la GUI
    float vidaMaxima;
    float velocidad;
    
    NodoGenealogico* padre;
    NodoGenealogico* madre;
    
    // Lista de hijos para mantener la estructura N-aria
    ArregloDinamico<NodoGenealogico*> hijos;

    NodoGenealogico(int id, float vMax, float vel) 
        : idBichito(id), estadoVida(true), vidaMaxima(vMax), 
        velocidad(vel), padre(nullptr), madre(nullptr) {}
};

/**
 * @class ArbolLinajeGen
 * @brief Maneja un árbol N-ario dedicado exclusivamente a rastrear la dispersión de un Gen Estático específico.
 * Totalmente desacoplado de las físicas para no afectar el rendimiento[cite: 10, 11].
 */
class ArbolLinajeGen {
private:
    GenEstatico genRastreado;
    ArregloDinamico<NodoGenealogico*> fundadores; // Raíces (los primeros en mutar o nacer con este gen)

public:
    ArbolLinajeGen(GenEstatico gen) : genRastreado(gen) {}

    /**
     * @brief Registra un nuevo portador en O(1)[cite: 10, 11].
     */
    void registrarPortador(int idHijo, float vMax, float vel, NodoGenealogico* nodoPadre, NodoGenealogico* nodoMadre);
    
    /**
     * @brief Marca al portador como fallecido preservando la rama familiar en O(1)[cite: 10, 11].
     */
    void registrarMuerte(NodoGenealogico* nodo);
    
    /**
     * @brief Recorrido en O(N) para la interfaz gráfica. Solo se ejecuta bajo demanda[cite: 10, 11].
     */
    void obtenerDispersianParaGUI(); 
};

#endif // ARBOL_GENEALOGICO_H