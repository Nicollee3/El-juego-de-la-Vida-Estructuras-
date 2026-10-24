
/*
 * @file Cola.h
 * @brief Implementación de una Cola de Bichitos con lista enlazada.
 * @details Sigue el principio FIFO.
 *          Mantiene punteros a la cabeza y al último, por eso insertar y eliminar cuestan O(1).
 * @author Karoll Ramírez
 * @date Octubre 2026
 */

#ifndef COLA_H
#define COLA_H

#include "Bichito.h"

/*
 * @class Cola
 * @brief Estructura de datos lineal de tipo cola para elementos Bichito.
 * @details Permite la inserción de elementos por el final y la extracción por la cabeza.
 */
class Cola
{
private:
    /*
     * @struct NodoLista
     * @brief Nodo individual de la Cola. Es privado: nadie fuera de la Cola lo necesita.
     */
    struct NodoLista
    {
        Bichito dato;
        NodoLista *sig;
        /*
         * @brief Construye un nodo con una copia del bichito.
         */
        NodoLista(const Bichito &_dato) : dato(_dato), sig(nullptr) {}
    };

    NodoLista *cabeza;
    NodoLista *ultimo;
    int N;

public:
    /*
     * @brief Construye una nueva cola vacía.
     */
    Cola()
    {
        cabeza = nullptr;
        ultimo = nullptr;
        N = 0;
    }

    /*
     * @brief Destructor de la Cola. Libera todos los nodos.
     */
    ~Cola()
    {
        while (eliminar_primero())
        {
        }
    }

    /*
     * @brief Se prohíbe copiar la cola: dos colas compartiendo nodos liberarían la misma memoria dos veces.
     */
    Cola(const Cola &) = delete;
    Cola &operator=(const Cola &) = delete;

    /*
     * @brief Verifica si la cola se encuentra vacía.
     * @return true si no hay elementos, false si hay al menos uno.
     */
    bool esvacia() const
    {
        return cabeza == nullptr;
    }

    /*
     * @brief Cantidad de bichitos que hay en la cola. O(1).
     */
    int longitud() const
    {
        return N;
    }

    /*
     * @brief Inserta un bichito al final de la cola. O(1).
     * @param d Bichito que se desea encolar (se guarda una copia).
     */
    void insertar_al_ultimo(const Bichito &d)
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
        N++;
    }

    /*
     * @brief Consulta el primer bichito SIN sacarlo de la cola. O(1).
     * @param salida Aquí se guarda una copia del primer bichito.
     * @return false si la cola estaba vacía (y salida no se toca).
     */
    bool ver_primero(Bichito &salida) const
    {
        if (esvacia())
            return false;
        salida = cabeza->dato;
        return true;
    }

    /*
     * @brief Elimina el primer bichito y lo entrega en salida. O(1).
     * @param salida Aquí se guarda el bichito extraído.
     * @return true si se eliminó algo, false si la cola estaba vacía.
     */
    bool eliminar_primero(Bichito &salida)
    {
        if (esvacia())
            return false;

        NodoLista *aux = cabeza;
        salida = aux->dato;

        cabeza = cabeza->sig;
        if (cabeza == nullptr)
            ultimo = nullptr;

        delete aux;
        N--;
        return true;
    }

    /*
     * @brief Elimina el primer bichito sin devolverlo (útil para vaciar la cola).
     * @return true si se eliminó algo, false si estaba vacía.
     */
    bool eliminar_primero()
    {
        if (esvacia())
            return false;

        NodoLista *aux = cabeza;
        cabeza = cabeza->sig;
        if (cabeza == nullptr)
            ultimo = nullptr;

        delete aux;
        N--;
        return true;
    }
};

#endif