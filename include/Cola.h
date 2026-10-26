/*
 * @file Cola.h
 * @brief Cola FIFO genérica implementada con una lista enlazada.
 */

#ifndef COLA_H
#define COLA_H

template <typename T>
class Cola
{
private:
    struct NodoLista
    {
        T dato;
        NodoLista *sig;

        explicit NodoLista(const T &valor) : dato(valor), sig(nullptr) {}
    };

    NodoLista *cabeza;
    NodoLista *ultimo;
    int N;

public:
    Cola() : cabeza(nullptr), ultimo(nullptr), N(0) {}

    ~Cola()
    {
        vaciar();
    }

    Cola(const Cola &) = delete;
    Cola &operator=(const Cola &) = delete;

    bool esvacia() const { return cabeza == nullptr; }
    int longitud() const { return N; }

    void insertar_al_ultimo(const T &dato)
    {
        NodoLista *nuevo = new NodoLista(dato);
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
        ++N;
    }

    bool ver_primero(T &salida) const
    {
        if (esvacia())
        {
            return false;
        }
        salida = cabeza->dato;
        return true;
    }

    bool eliminar_primero(T &salida)
    {
        if (esvacia())
        {
            return false;
        }

        NodoLista *aux = cabeza;
        salida = aux->dato;
        cabeza = cabeza->sig;
        if (cabeza == nullptr)
        {
            ultimo = nullptr;
        }
        delete aux;
        --N;
        return true;
    }

    bool eliminar_primero()
    {
        if (esvacia())
        {
            return false;
        }

        NodoLista *aux = cabeza;
        cabeza = cabeza->sig;
        if (cabeza == nullptr)
        {
            ultimo = nullptr;
        }
        delete aux;
        --N;
        return true;
    }

    bool eliminar(const T &dato)
    {
        NodoLista *anterior = nullptr;
        NodoLista *actual = cabeza;
        while (actual != nullptr)
        {
            if (actual->dato == dato)
            {
                if (anterior == nullptr)
                {
                    cabeza = actual->sig;
                }
                else
                {
                    anterior->sig = actual->sig;
                }
                if (actual == ultimo)
                {
                    ultimo = anterior;
                }
                delete actual;
                --N;
                return true;
            }
            anterior = actual;
            actual = actual->sig;
        }
        return false;
    }

    void vaciar()
    {
        while (eliminar_primero())
        {
        }
    }
};

#endif
