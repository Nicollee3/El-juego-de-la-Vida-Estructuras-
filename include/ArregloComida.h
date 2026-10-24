/*
 * @file AdministradorComidaFijo.h
 * @brief Administrador de comida con tamaño fijo, ruleta de probabilidades y reciclaje O(1).
 * @author Karoll Ramírez
 * @date Octubre 2026
 */

#ifndef ADMINISTRADOR_COMIDA_FIJO_H
#define ADMINISTRADOR_COMIDA_FIJO_H

#include <cstdlib>
#include "Comida.h"
#include "ColaCircularIndices.h" // La cola circular que hicimos antes
#include "TablaHashComidas.h"    // Tu tabla hash estática que hicimos antes

class AdministradorComidaFijo
{
private:
    Comida *arregloComida;           // Arreglo estático de tamaño fijo
    int capacidadMax;                // Tamaño fijo máximo del mapa
    ColaCircularIndices colaLibres;  // Cola circular de índices disponibles O(1)
    TablaHashComidas tablaHash;      // Diccionario ID -> Índice en O(1)
    int siguienteId;

public:
    // Límites de la ruleta (0 a 99)
    static const int LIMITE_NORMAL = 70;  // 70% normal (prio 3, 15 hp)
    static const int LIMITE_MEDIANA = 90; // 20% mediana (prio 2, 30 hp)
                                      // 10% especial (prio 1, 60 hp)

    AdministradorComidaFijo(int capMax = 300) 
        : capacidadMax(capMax), colaLibres(capMax), tablaHash(503), siguienteId(1)
    {
        // Reservamos memoria una sola vez (Tamaño Fijo Estático)
        arregloComida = new Comida[capacidadMax];
    }

    ~AdministradorComidaFijo()
    {
        delete[] arregloComida;
    }

    // Traduce la ruleta (Conservamos tu lógica exacta)
    static int prioridadPorTirada(int tirada)
    {
        if (tirada < LIMITE_NORMAL) return 3;
        if (tirada < LIMITE_MEDIANA) return 2;
        return 1;
    }

    static int puntosPorPrioridad(int prioridad)
    {
        if (prioridad == 1) return 60;
        if (prioridad == 2) return 30;
        return 15;
    }

    // GENERAR COMIDA POR PROBABILIDAD (Todo en O(1))
    bool generar(float x, float y, OrigenComida origen = OrigenComida::Pool)
    {
        return generarConTirada(std::rand() % 100, x, y, origen);
    }

    bool generarConTirada(int tirada, float x, float y, OrigenComida origen)
    {
        // 1. Preguntamos a la cola circular si hay un índice libre (O(1))
        if (colaLibres.estaVacia())
        {
            return false; // El mapa está lleno de comida, no se puede insertar más
        }

        int indiceLibre = colaLibres.dequeue(); // O(1)

        // 2. Creamos la comida usando tu ruleta
        int prioridad = prioridadPorTirada(tirada);
        int vida = puntosPorPrioridad(prioridad);

        // Configuramos la comida en la ranura fija del arreglo
        arregloComida[indiceLibre] = Comida(siguienteId, prioridad, vida, x, y, origen);
        
        // 3. Registramos en la Tabla Hash en O(1) promedio
        tablaHash.insertar(siguienteId, indiceLibre);

        siguienteId++;
        return true;
    }

    // ELIMINAR CUANDO SE LA COMEN (En O(1) estricto, sin corrimientos)
    bool consumirComidaPorId(int idComida)
    {
        // 1. Buscamos el índice en la tabla hash en O(1)
        int indice = tablaHash.buscarIndice(idComida);
        if (indice == -1) return false; // No existe

        // 2. Desactivamos la comida
        arregloComida[indice].setActiva(false);

        // 3. Borramos de la tabla hash en O(1)
        tablaHash.eliminar(idComida);

        // 4. Reciclamos el índice devolviéndolo a la cola circular en O(1)
        colaLibres.enqueue(indice);

        return true;
    }

    // Método para dibujar en SFML (recorre el arreglo fijo de manera ultra rápida)
    const Comida* getArregloComida() const { return arregloComida; }
    int getCapacidadMax() const { return capacidadMax; }
};

#endif