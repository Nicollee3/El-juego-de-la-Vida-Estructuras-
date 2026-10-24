/*
 * @file ColaCircularIndices.h
 * @brief Cola circular estática para reciclar los índices de las posiciones libres.
 * @author Karoll Ramírez
 * @date Octubre 2026
 */

#ifndef COLA_CIRCULAR_INDICES_H
#define COLA_CIRCULAR_INDICES_H

#include <iostream>

class ColaCircularIndices
{
private:
    int *arr;          
    int capacidad;
    int frenteIdx;
    int finalIdx;
    int cantidad;

public:
    ColaCircularIndices(int cap)
    {
        capacidad = cap;
        arr = new int[capacidad];
        frenteIdx = 0;
        finalIdx = -1;
        cantidad = 0;

        // Llenamos la cola inicialmente con todos los índices disponibles (0, 1, 2, ..., capacidad-1)
        for (int i = 0; i < capacidad; ++i)
        {
            enqueue(i);
        }
    }

    ~ColaCircularIndices()
    {
        delete[] arr;
    }

    bool estaVacia() const
    {
        return cantidad == 0;
    }

    bool estaLlena() const
    {
        return cantidad == capacidad;
    }

    // Inserta un índice libre al final de la cola (Enqueue) - O(1)
    void enqueue(int index)
    {
        if (estaLlena()) return;
        finalIdx = (finalIdx + 1) % capacidad;
        arr[finalIdx] = index;
        cantidad++;
    }

    // Saca y retorna el próximo índice libre del frente de la cola (Dequeue) - O(1)
    int dequeue()
    {
        if (estaVacia()) return -1; // Cola vacía (mapa de comida lleno)
        int index = arr[frenteIdx];
        frenteIdx = (frenteIdx + 1) % capacidad;
        cantidad--;
        return index;
    }

    int getCantidad() const { return cantidad; }
    int getCapacidad() const { return capacidad; }
};

#endif