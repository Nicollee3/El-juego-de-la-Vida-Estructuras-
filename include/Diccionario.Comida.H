/**
 * @file TablaHashComidas.h
 * @brief Tabla hash estática para mapear coordenadas (o IDs) de comida a índices fijos.
 * @author Karoll Ramírez
 * @date Octubre 2026
 */

#ifndef TABLA_HASH_COMIDAS_H
#define TABLA_HASH_COMIDAS_H

#include <iostream>

class TablaHashComidas
{
private:
    struct Entrada
    {
        int id;
        int indiceArreglo; // El índice en el arreglo fijo de objetos Comida
        bool ocupado;

        Entrada() : id(-1), indiceArreglo(-1), ocupado(false) {}
    };

    Entrada *tabla;
    int capacidad;

    // Función hash basada en el id de la comida
    int funcionHash(int id, int cap) const
    {
        return id % cap;
    }

public:
    // Constructor de tamaño fijo (ej. 503, un número primo mayor que la capacidad máxima de comida)
    TablaHashComidas(int cap = 503)
    {
        capacidad = cap;
        tabla = new Entrada[capacidad];
    }

    ~TablaHashComidas()
    {
        delete[] tabla;
    }

    /**
     * @brief Inserta o actualiza la relación ID -> Índice del arreglo en O(1) promedio.
     */
    void insertar(int id, int indiceArreglo)
    {
        int indice = funcionHash(id, capacidad);
        int inicio = indice;

        // Sondeo lineal en caso de colisión
        while (tabla[indice].ocupado && tabla[indice].id != id)
        {
            indice = (indice + 1) % capacidad;
            if (indice == inicio) break; // Tabla llena (si se dimensionó bien, no pasa)
        }

        tabla[indice].id = id;
        tabla[indice].indiceArreglo = indiceArreglo;
        tabla[indice].ocupado = true;
    }

    /**
     * @brief Busca el índice en el arreglo fijo usando el ID de la comida en O(1).
     */
    int buscarIndice(int id) const
    {
        int indice = funcionHash(id, capacidad);
        int inicio = indice;

        while (tabla[indice].ocupado)
        {
            if (tabla[indice].id == id)
            {
                return tabla[indice].indiceArreglo;
            }
            indice = (indice + 1) % capacidad;
            if (indice == inicio) break;
        }

        return -1; // No encontrada
    }

    /**
     * @brief Elimina la entrada de la tabla hash cuando la comida es consumida.
     */
    void eliminar(int id)
    {
        int indice = funcionHash(id, capacidad);
        int inicio = indice;

        while (tabla[indice].ocupado)
        {
            if (tabla[indice].id == id)
            {
                tabla[indice].ocupado = false;
                tabla[indice].id = -1;
                tabla[indice].indiceArreglo = -1;
                return;
            }
            indice = (indice + 1) % capacidad;
            if (indice == inicio) break;
        }
    }

    int getCapacidad() const { return capacidad; }
};

#endif