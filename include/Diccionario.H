/**
 * @file TablaHashBichitos.h
 * @brief Tabla hash con sondeo lineal y Rehashing automático para mantener O(1).
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
    struct Entrada
    {
        int id;
        Bichito *bichito;
        bool ocupado;

        Entrada() : id(-1), bichito(nullptr), ocupado(false) {}
    };

    Entrada *tabla;
    int capacidad;
    int elementos;

    // Función hash básica basada en el operador módulo
    int funcionHash(int id, int cap) const
    {
        return id % cap;
    }

    /**
     * @brief Redimensiona la tabla hash al doble de capacidad y reubica las entradas.
     */
    void rehash()
    {
        int antiguaCapacidad = capacidad;
        Entrada *antiguaTabla = tabla;

        // Duplicamos la capacidad (o usamos un primo mayor)
        capacidad = antiguaCapacidad * 2 + 1;
        tabla = new Entrada[capacidad];
        elementos = 0;

        // Reinsertamos todas las entradas que estaban ocupadas
        for (int i = 0; i < antiguaCapacidad; i++)
        {
            if (antiguaTabla[i].ocupado)
            {
                insertar(antiguaTabla[i].id, antiguaTabla[i].bichito);
            }
        }

        delete[] antiguaTabla; // Liberamos el arreglo viejo
    }

public:
    TablaHashBichitos(int cap = 101)
    {
        capacidad = cap;
        elementos = 0;
        tabla = new Entrada[capacidad];
    }

    ~TablaHashBichitos()
    {
        delete[] tabla;
    }

    /**
     * @brief Inserta o actualiza la dirección en memoria del Bichito por su ID.
     *        Aplica rehashing automático si el factor de carga supera 0.7.
     */
    void insertar(int id, Bichito *b)
    {
        // Si la tabla supera el 70% de ocupación, redimensionamos antes de insertar
        if ((float)(elementos + 1) / capacidad > 0.7f)
        {
            rehash();
        }

        int indice = funcionHash(id, capacidad);

        // Sondeo lineal en caso de colisión
        while (tabla[indice].ocupado && tabla[indice].id != id)
        {
            indice = (indice + 1) % capacidad;
        }

        if (!tabla[indice].ocupado)
        {
            elementos++;
        }

        tabla[indice].id = id;
        tabla[indice].bichito = b;
        tabla[indice].ocupado = true;
    }

    /**
     * @brief Elimina la entrada del ID en la tabla hash cuando el bichito muere.
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
                tabla[indice].bichito = nullptr;
                tabla[indice].id = -1;
                elementos--;
                return;
            }
            indice = (indice + 1) % capacidad;
            if (indice == inicio) break;
        }
    }

    /**
     * @brief Busca la referencia directa del bichito por su ID en O(1) promedio.
     */
    Bichito *buscar(int id) const
    {
        int indice = funcionHash(id, capacidad);
        int inicio = indice;

        while (tabla[indice].ocupado)
        {
            if (tabla[indice].id == id)
            {
                return tabla[indice].bichito;
            }
            indice = (indice + 1) % capacidad;

            if (indice == inicio)
            {
                break;
            }
        }

        return nullptr;
    }

    int getCapacidad() const { return capacidad; }
    int getElementos() const { return elementos; }
};

#endif