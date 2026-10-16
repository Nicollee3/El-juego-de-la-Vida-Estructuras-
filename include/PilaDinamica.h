/*
 * @file PilaDinamica.h
 * @brief Implementación de estructura de datos lineal de tipo PILA utilizando arreglo dinámico
 * @details Aquí se gestiona un arreglo dinámico de BICHITOS. Podemos agregar y eliminar objetos.
 *          El tamaño del arreglo se ajusta de manera dinámica de acuerdo a la cantidad de elementos apoyándose en la proporción áurea.
 *          Se implementa la técnica "Swap and Pop" para eliminaciones en O(1) desde cualquier posición.
 *
 * @author Karoll Ramírez
 * @OCTUBRE 2026
 */

#ifndef PILA_DINAMICA_H
#define PILA_DINAMICA_H

#include <iostream>
#include "BICHITO.H"

class PilaDinamica
{
public:
    /*
     * @brief Construye una nueva Pila Dinámica vacía.
     * @details Inicializa el contador de elementos (N) en 0, asigna una capacidad inicial (10)
     * y reserva memoria para el arreglo de Bichitos llamado poblacion.
     */
    PilaDinamica()
    {
        N = 0;
        tamano = 10;
        poblacion = new Bichito[tamano];
    };
    /*
     * @brief Destructor de la Pila Dinámica.
     * @details Libera la memoria asignada dinámicamente para evitar fugas de memoria.
     */
    ~PilaDinamica()
    {
        delete[] poblacion;
    };
    /*
     * @brief Agrega un nuevo bichito "en la parte de arriba" de la pila.
     * @details Si la pila alcanza su capacidad máxima, duplica/expande el tamaño del arreglo
     *          multiplicando la capacidad actual por la razón áurea (~1.618).
     * @param d El objeto Bichito que se desea "generar" en el juego de la vida, es decir agregarlo a la pila.
     */
    void generar(Bichito d)
    {
        if (N == tamano)
        {
            int ntamano = (int)(tamano * aureo);
            Bichito *ndatos = new Bichito[ntamano];
            for (int i = 0; i < tamano; i++)
            {
                ndatos[i] = poblacion[i];
            };
            delete[] poblacion;
            poblacion = ndatos;
            tamano = ntamano;
        };
        poblacion[N] = d;
        N++;
    };
    /*
     * @brief Elimina el elemento en la partde de arriba de la pila, es decir, el último elemento agregado.
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
            Bichito *ndatos = new Bichito[ntamano];
            for (int i = 0; i < N; i++)
            {
                ndatos[i] = poblacion[i];
            };
            delete[] poblacion;
            poblacion = ndatos;
            tamano = ntamano;
        };
        N--;
    };
    void morir(int i)
    /*
     * @brief Elimina un bichito de cualquier posición en tiempo O(1) usando "Swap and Pop".
     * @details Intercambia el elemento ubicado en el índice i con el último elemento válido.
     *        Luego, llama a borrar() para eliminar el último elemento, reduciendo así la cantidad de elementos en la pila.
     * @param i El índice del bichito que se desea eliminar.
     */
    {
        if (i < 0 || i >= N)
        {
            std::cerr << "Indice " << i << " fuera de rango para morir()\n";
            return;
        }
        Bichito temp = poblacion[i];
        poblacion[i] = poblacion[N - 1];
        poblacion[N - 1] = temp;
        borrar();
    }
    int longitud()
    /*
     * @brief Obtiene la cantidad actual de bichitos almacenados en la pila.
     * @return int Número de elementos activos (N).
     */
    {
        return N;
    };
    /*
     * @brief Obtiene el objeto Bichito ubicado en una posición i-ésima sin eliminarlo. Nos sirve para consultar.
     * @param i Índice del elemento a consultar.
     * @return Bichito Copia del objeto encontrado o un bichito de control con valores -1 si el índice no es válido.
     */
    Bichito obtener(int i)
    {
        if (i < 0 || i >= N)
        {
            std::cerr << "Error: Indice fuera de rango\n";
            return Bichito(-1, -1);
        }
        return poblacion[i];
    };

    Bichito& obtenerRef(int i)
    {
        if (i < 0 || i >= N)
        {
            static Bichito centinela(-1, -1);
            return centinela;
        }
        return poblacion[i];
    }
    
protected:
    Bichito *poblacion; // Poblacion es un apuntador, que esta apuntando a la direccion de memoria donde estan guardados los objetos bichito
    int N;
    int tamano;
    double aureo = 1.618;
};

#endif
