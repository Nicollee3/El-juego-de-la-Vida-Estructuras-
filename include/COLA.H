/*
 * @file Cola.h
 * @brief Implementación de una Cola.
 * @details Sigue el principio FIFO.
 * @author Karoll Ramírez
 * @date Octubre 2026
 */

#ifndef COLA_H
#define COLA_H

#include <iostream>
#include "Comida.h"

/*
 * @class NodoLista
 * @brief Representa un nodo individual dentro de la estructura de la Cola.
 * @details Almacena un objeto de tipo Comida y un puntero hacia el siguiente nodo de la secuencia.
 */
class NodoLista
{
public:

    Comida dato;
    NodoLista *sig;
    /*
     * @brief Construye un nuevo NodoLista con un objeto Comida específico.
     * @param _dato Objeto Comida que se asignará al nodo.
     */

    NodoLista(Comida _dato)
    {
        dato = _dato;
        sig = NULL;
    }
};
/*
 * @class Cola
 * @brief Estructura de datos lineal de tipo cola para elementos Comida.
 * @details Permite la inserción de elementos por el final y la extracción por la cabeza.
 */
class Cola
{
private:
    NodoLista *cabeza;
    NodoLista *ultimo;

public:
    /*
     * @brief Construye una nueva cola vacía.
     * @details Inicializa los punteros cabeza y ultimo.
     */
    Cola()
    {
        cabeza = NULL;
        ultimo = NULL;
    }
    /*
     * @brief Destructor de la Cola.
     * @details Libera memoria.
     */
    ~Cola()
    {
        while (!esvacia())
        {
            eliminar_primero();
        }
    }
    /*
     * @brief Verifica si la cola se encuentra vacía.
     * @return true si no hay elementos en la cola (cabeza es null)
     * @return false si la cola contiene al menos un elemento.
     */
    bool esvacia() const
    {
        return cabeza == NULL;
    }
    /*
     * @brief Inserta un nuevo elemento de comida al final de la cola.
     * @details Crea un nuevo nodo y actualiza los punteros de la cola 
     * @param d Objeto Comida que se desea encolar.
     */
    void insertar_al_ultimo(Comida d)
    {
        NodoLista *nuevo = new NodoLista(d);

        if (esvacia())
        {
            cabeza = nuevo;
            ultimo = nuevo;
        }
        else
        {
            ultimo->sig = nuevo;
            ultimo = nuevo;
        }
    }
    /*
     * @brief Elimina y retorna el primer elemento de la cola.
     * @details Extrae el elemento ubicado en la cabeza, actualiza los punteros y libera la memoria del nodo que fue eliminado.
     * @return Comida El objeto comida extraído, o un objeto centinela Comida(-1, -1, -1) si la cola está vacía.
     */
    Comida eliminar_primero()
    {
        if (esvacia())
        {
            std::cout << "La cola esta vacia." << std::endl;
            return Comida(-1, -1, -1);
        }

        Comida d = cabeza->dato;
        NodoLista *aux = cabeza;

        cabeza = cabeza->sig;

        if (esvacia())
        {
            ultimo = NULL;
        }

        delete aux;
        return d;
    }
};

#endif