/**
 * @file TablaHashBichitos.h
 * @brief Tabla hash dinámica con Rehashing y marcas de borrado (Tombstones).
 * @author Karoll Ramírez
 * @date Octubre 2026
 */

#ifndef TABLA_HASH_BICHITOS_H
#define TABLA_HASH_BICHITOS_H

#include <iostream>
#include "Bichito.h"

class TablaHashBichitos
{
private:
    enum Estado { LIBRE, OCUPADO, BORRADO };

    struct Entrada
    {
        int id;
        Bichito *bichito;
        Estado estado;

        Entrada() : id(-1), bichito(nullptr), estado(LIBRE) {}
    };

    Entrada *tabla;
    int capacidad;
    int elementosActivos;
    int tombstones; // Contador de espacios borrados para saber cuándo limpiar en el rehash

    // Función hash con mezcla de bits
    int funcionHash(int id, int cap) const
    {
        unsigned int hash = static_cast<unsigned int>(id);
        hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
        hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
        hash = (hash >> 16) ^ hash;
        return hash % cap;
    }

    /**
     * @brief Redimensiona la tabla cuando el factor de carga supera el 70%.
     */
    void rehash()
    {
        int antiguaCapacidad = capacidad;
        Entrada *antiguaTabla = tabla;

        // Duplicamos capacidad (idealmente buscar un primo, pero * 2 + 1 funciona bien)
        capacidad = antiguaCapacidad * 2 + 1;
        tabla = new Entrada[capacidad];
        elementosActivos = 0;
        tombstones = 0;

        // Reinsertamos solo los elementos realmente OCUPADOS (limpiando los tombstones)
        for (int i = 0; i < antiguaCapacidad; i++)
        {
            if (antiguaTabla[i].estado == OCUPADO)
            {
                insertarInterno(antiguaTabla[i].id, antiguaTabla[i].bichito);
            }
        }

        delete[] antiguaTabla;
    }

    // Inserción interna auxiliar para el proceso de rehash (sin disparar otro rehash)
    void insertarInterno(int id, Bichito *b)
    {
        int indice = funcionHash(id, capacidad);
        while (tabla[indice].estado == OCUPADO)
        {
            if (tabla[indice].id == id) {
                tabla[indice].bichito = b;
                return;
            }
            indice = (indice + 1) % capacidad;
        }

        tabla[indice].id = id;
        tabla[indice].bichito = b;
        tabla[indice].estado = OCUPADO;
        elementosActivos++;
    }

public:
    TablaHashBichitos(int cap = 1009)
    {
        capacidad = cap;
        elementosActivos = 0;
        tombstones = 0;
        tabla = new Entrada[capacidad];
    }

    ~TablaHashBichitos()
    {
        delete[] tabla;
    }

    TablaHashBichitos(const TablaHashBichitos&) = delete;
    TablaHashBichitos& operator=(const TablaHashBichitos&) = delete;

    /**
     * @brief Inserta o actualiza el bichito. Aplica rehash si pasa el 70% de ocupación.
     */
    void insertar(int id, Bichito *b)
    {
        // Factor de carga basado en activos + tombstones para no saturar el sondeo
        if ((float)(elementosActivos + tombstones + 1) / capacidad > 0.7f)
        {
            rehash();
        }

        int indice = funcionHash(id, capacidad);
        int inicio = indice;
        int primerTombstone = -1;

        while (tabla[indice].estado != LIBRE)
        {
            if (tabla[indice].estado == OCUPADO && tabla[indice].id == id)
            {
                tabla[indice].bichito = b;
                return;
            }

            if (tabla[indice].estado == BORRADO && primerTombstone == -1)
            {
                primerTombstone = indice;
            }

            indice = (indice + 1) % capacidad;
            if (indice == inicio) break;
        }

        int indiceInsercion = (primerTombstone != -1) ? primerTombstone : indice;

        if (tabla[indiceInsercion].estado == BORRADO)
        {
            tombstones--;
        }

        if (tabla[indiceInsercion].estado != OCUPADO)
        {
            elementosActivos++;
        }

        tabla[indiceInsercion].id = id;
        tabla[indiceInsercion].bichito = b;
        tabla[indiceInsercion].estado = OCUPADO;
    }

    /**
     * @brief Elimina marcando como BORRADO (Tombstone) para no romper las cadenas de colisión.
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
                tabla[indice].bichito = nullptr;
                elementosActivos--;
                tombstones++;
                return;
            }
            indice = (indice + 1) % capacidad;
            if (indice == inicio) break;
        }
    }

    /**
     * @brief Busca el bichito por su ID en O(1) promedio.
     */
    Bichito *buscar(int id) const
    {
        int indice = funcionHash(id, capacidad);
        int inicio = indice;

        while (tabla[indice].estado != LIBRE)
        {
            if (tabla[indice].estado == OCUPADO && tabla[indice].id == id)
            {
                return tabla[indice].bichito;
            }
            indice = (indice + 1) % capacidad;
            if (indice == inicio) break;
        }

        return nullptr;
    }

    int getCapacidad() const { return capacidad; }
    int getElementos() const { return elementosActivos; }
};

#endif