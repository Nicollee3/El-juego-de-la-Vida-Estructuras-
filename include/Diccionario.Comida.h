/**
 * @file TablaHashComidas.h
 * @brief Tabla hash estática optimizada para mapear IDs de comida a índices de arreglo con Tombstones y mezcla de bits.
 * @author Karoll Ramírez
 * @date Octubre 2026
 */

#ifndef TABLA_HASH_COMIDAS_H
#define TABLA_HASH_COMIDAS_H

#include <iostream>

class TablaHashComidas
{
private:
    enum Estado { LIBRE, OCUPADO, BORRADO };

    struct Entrada
    {
        int id;
        int indiceArreglo; // El índice en el arreglo fijo de objetos Comida
        Estado estado;

        Entrada() : id(-1), indiceArreglo(-1), estado(LIBRE) {}
    };

    Entrada *tabla;
    int capacidad;
    int elementosActivos;

    // Función hash con mezcla de bits para evitar agrupamientos por IDs secuenciales
    int funcionHash(int id, int cap) const
    {
        unsigned int hash = static_cast<unsigned int>(id);
        hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
        hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
        hash = (hash >> 16) ^ hash;
        return hash % cap;
    }

public:
    // Constructor de tamaño fijo (ej. 503, un número primo recomendado)
    TablaHashComidas(int cap = 503)
    {
        capacidad = cap;
        elementosActivos = 0;
        tabla = new Entrada[capacidad];
    }

    ~TablaHashComidas()
    {
        delete[] tabla;
    }

    // Prevenir copias superficiales para proteger la memoria dinámica
    TablaHashComidas(const TablaHashComidas&) = delete;
    TablaHashComidas& operator=(const TablaHashComidas&) = delete;

    /**
     * @brief Inserta o actualiza la relación ID -> Índice del arreglo en O(1) promedio.
     */
    void insertar(int id, int indiceArreglo)
    {
        int indice = funcionHash(id, capacidad);
        int inicio = indice;
        int primerTombstone = -1;

        while (tabla[indice].estado != LIBRE)
        {
            // Si el ID ya existe, actualizamos su índice de arreglo y salimos
            if (tabla[indice].estado == OCUPADO && tabla[indice].id == id)
            {
                tabla[indice].indiceArreglo = indiceArreglo;
                return;
            }

            // Guardamos el primer espacio BORRADO para reutilizarlo inteligentemente
            if (tabla[indice].estado == BORRADO && primerTombstone == -1)
            {
                primerTombstone = indice;
            }

            indice = (indice + 1) % capacidad;
            if (indice == inicio) break; // Tabla llena
        }

        // Elegimos dónde insertar: si hay un tombstone lo reciclamos, sino usamos el índice libre
        int indiceInsercion = (primerTombstone != -1) ? primerTombstone : indice;

        if (tabla[indiceInsercion].estado != OCUPADO)
        {
            elementosActivos++;
        }

        tabla[indiceInsercion].id = id;
        tabla[indiceInsercion].indiceArreglo = indiceArreglo;
        tabla[indiceInsercion].estado = OCUPADO;
    }

    /**
     * @brief Busca el índice en el arreglo fijo usando el ID de la comida en O(1).
     */
    int buscarIndice(int id) const
    {
        int indice = funcionHash(id, capacidad);
        int inicio = indice;

        while (tabla[indice].estado != LIBRE)
        {
            if (tabla[indice].estado == OCUPADO && tabla[indice].id == id)
            {
                return tabla[indice].indiceArreglo;
            }
            indice = (indice + 1) % capacidad;
            if (indice == inicio) break;
        }

        return -1; // No encontrada
    }

    /**
     * @brief Elimina la entrada marcándola como BORRADO (Tombstone) para preservar las cadenas de colisión.
     */
    void eliminar(int id)
    {
        int indice = funcionHash(id, capacidad);
        int inicio = indice;

        while (tabla[indice].estado != LIBRE)
        {
            if (tabla[indice].estado == OCUPADO && tabla[indice].id == id)
            {
                tabla[indice].estado = BORRADO;
                tabla[indice].id = -1;
                tabla[indice].indiceArreglo = -1;
                elementosActivos--;
                return;
            }
            indice = (indice + 1) % capacidad;
            if (indice == inicio) break;
        }
    }

    int getCapacidad() const { return capacidad; }
    int getElementosActivos() const { return elementosActivos; }
};

#endif