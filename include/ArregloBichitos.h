/*
 * @file ArregloDinamico.h
 * @brief Implementación de estructura de datos lineal de tipo ARREGLO 
 * @details Es una PLANTILLA (template): ArregloDinamico<T> puede almacenar cualquier tipo de objeto T (Bichito, Comida, etc.).
 *          La pila crece y se encoge automáticamente según la cantidad de elementos.
 *          El tamaño del arreglo se ajusta de manera dinámica de acuerdo a la cantidad de elementos apoyándose en la proporción áurea.
 *          Se implementa la técnica "Swap and Pop" para eliminaciones en O(1) desde cualquier posición.
 *
 * @author Karoll Ramírez
 * @OCTUBRE 2026
 */

#ifndef ARREGLO_DINAMICO_H
#define ARREGLO_DINAMICO_H

#include <iostream>
#include "Bichito.h"
#include "Comida.h"

/*
 * Elemento "Entidad": es lo que devuelven obtener() y obtenerRef() cuando el índice no es válido.
 * Por defecto es T(); para Bichito y Comida se conserva el valor de control con -1.
 */
namespace detalle
{
    template <typename T>
    T crearEntidad() { return T(); }

    template <>
    inline Bichito crearEntidad<Bichito>() { return Bichito(-1, -1); }

    template <>
    inline Comida crearEntidad<Comida>() { return Comida(-1, -1, -1); }
}

template <typename T>
class ArregloDinamico
{
public:
    /*
     * @brief Construye una nueva Pila Dinámica vacía.
     * @details Inicializa el contador de elementos (N) en 0, asigna una capacidad inicial (10)
     * y reserva memoria para el arreglo de elementos llamado datos.
     */
    ArregloDinamico()
    {
        N = 0;
        tamano = 10;
        datos = new T[tamano];
    };
    /*
     * @brief Destructor de la Pila Dinámica.
     * @details Libera la memoria asignada dinámicamente para evitar fugas de memoria.
     */
    ~ArregloDinamico()
    {
        delete[] datos;
    };

    // Copiar una pila duplicaría el puntero y la memoria se liberaría dos veces: se prohíbe copiarla.
    ArregloDinamico(const ArregloDinamico &) = delete;
    ArregloDinamico &operator=(const ArregloDinamico &) = delete;

    /*
     * @brief Agrega un nuevo elemento "en la parte de arriba" de la pila.
     * @details Si la pila alcanza su capacidad máxima, duplica/expande el tamaño del arreglo
     *          multiplicando la capacidad actual por la razón áurea (~1.618).
     * @param d El objeto que se desea "generar" en el juego de la vida, es decir agregarlo a la pila.
     */
    void generar(T d)
    {
        if (N == tamano)
        {
            int ntamano = (int)(tamano * aureo);
            T *ndatos = new T[ntamano];
            for (int i = 0; i < tamano; i++)
            {
                ndatos[i] = datos[i];
            };
            delete[] datos;
            datos = ndatos;
            tamano = ntamano;
        };
        datos[N] = d;
        N++;
    };
    /*
     * @brief Elimina el elemento en la parte de arriba de la pila, es decir, el último elemento agregado.
     * @details Reduce el contador de elementos y evalúa si la memoria debe contraerse
     *          según las restricciones de la razón áurea para optimizar espacio.
     */
    void borrar()
    {
        if (N <= 0)
            return;
        if (N * aureo * aureo < tamano && tamano > 10)
        {
            int ntamano = (int)(N * aureo);
            T *ndatos = new T[ntamano];
            for (int i = 0; i < N; i++)
            {
                ndatos[i] = datos[i];
            };
            delete[] datos;
            datos = ndatos;
            tamano = ntamano;
        };
        N--;
    };
    void morir(int i)
    /*
     * @brief Elimina un elemento de cualquier posición en tiempo O(1) usando "Swap and Pop".
     * @details Intercambia el elemento ubicado en el índice i con el último elemento válido.
     *        Luego, llama a borrar() para eliminar el último elemento, reduciendo así la cantidad de elementos en la pila.
     *        Nota: el orden de los elementos restantes puede cambiar.
     * @param i El índice del elemento que se desea eliminar.
     */
    {
        if (i < 0 || i >= N)
        {
            std::cerr << "Indice " << i << " fuera de rango para morir()\n";
            return;
        }
        T temp = datos[i];
        datos[i] = datos[N - 1];
        datos[N - 1] = temp;
        borrar();
    }
    /*
     * @brief Elimina TODOS los elementos y vuelve a la capacidad inicial (10).
     * @details Reemplaza al antiguo clear() de std::vector.
     */
    void vaciar()
    {
        delete[] datos;
        tamano = 10;
        N = 0;
        datos = new T[tamano];
    }
    int longitud() const
    /*
     * @brief Obtiene la cantidad actual de elementos almacenados en la pila.
     * @return int Número de elementos activos (N).
     */
    {
        return N;
    };
    /*
     * @brief Obtiene el objeto ubicado en una posición i-ésima sin eliminarlo. Nos sirve para consultar.
     * @param i Índice del elemento a consultar.
     * @return T Copia del objeto encontrado o un objeto de control con valores -1 si el índice no es válido.
     */
    T obtener(int i) const
    {
        if (i < 0 || i >= N)
        {
            std::cerr << "Error: Indice fuera de rango\n";
            return detalle::crearEntidad<T>();
        }
        return datos[i];
    };

    /*
     * @brief Devuelve la capacidad actual del arreglo dinámico (no la cantidad de elementos).
     * @return int Tamaño reservado en memoria (tamano). Se usa para mostrarlo en la barra lateral.
     */
    int capacidad() const
    {
        return tamano;
    }

    /*
     * @brief Devuelve una REFERENCIA al elemento i-ésimo, para poder consultarlo y modificarlo sin copiarlo.
     * @details La referencia deja de ser válida si luego se llama a generar() (el arreglo puede moverse en memoria).
     */
    T &obtenerRef(int i)
    {
        if (i < 0 || i >= N)
        {
            static T Entidad = detalle::crearEntidad<T>();
            return Entidad;
        }
        return datos[i];
    }

protected:
    T *datos; // datos es un apuntador a la dirección de memoria donde están guardados los objetos de tipo T
    int N;
    int tamano;
    double aureo = 1.618;
};

#endif