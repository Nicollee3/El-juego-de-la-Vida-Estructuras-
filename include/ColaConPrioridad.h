/*
 * @file ColaConPrioridad.h
 * @brief Gestiona la comida del juego mediante 3 niveles de prioridad por medio de colas.
 * @details Aquí se gestiona un arreglo estático de 3 colas de COMIDA de acuerdo a la prioridad de cada elemento (3 niveles).
 *          Incluye un método de extracción por pesos probabilísticos para simular abundancia en Prioridad 3 y escasez en Prioridad 1.
 *
 * @author Karoll Ramírez
 * @author Nicolle Ruíz
 * @OCTUBRE 2026
 */

#ifndef COLA_CON_PRIORIDAD_H
#define COLA_CON_PRIORIDAD_H

#include <iostream>
#include <cstdlib> // Para std::rand()
#include "Comida.h"
#include "Cola.h"

class ColaConPrioridad
{
private:
    Cola colas[3]; // Arreglo de 3 Colas: colas[0] = Prioridad 1, colas[1] = Prioridad 2, colas[2] = Prioridad 3

public:
    /*
     * @brief Constructor por defecto.
     * @details Inicializa de manera implícita las 3 colas del arreglo.
     */
    ColaConPrioridad()
    {
        // Las colas se inicializan automáticamente al ser objetos de la clase Cola.
    }

    /*
     * Inserta un objeto Comida en la cola correspondiente a su nivel de prioridad.
     * @param c Objeto Comida a encolar.
     */
    void insertar(Comida c)
    {
        int prio = c.getPrioridad(); // Obtiene la prioridad (1, 2 o 3)
        
        // Mapea la prioridad (1..3) a los índices del arreglo (0..2)
        if (prio >= 1 && prio <= 3)
        {
            colas[prio - 1].insertar_al_ultimo(c);
        }
        else
        {
            // Si por alguna razón tiene prioridad fuera de rango, se envía por defecto a la más baja (índice 2)
            colas[2].insertar_al_ultimo(c);
        }
    }

    /*
     * El siguiente metodo extrae y retorna la comida de mayor prioridad disponible en la simulación.
     * Revisa en orden estricto desde la Prioridad 1 (índice 0) hasta la Prioridad 3 (índice 2).
     * @return Comida El objeto extraído o Comida(-1, -1, -1) si no hay comida disponible.
     */
    Comida extraer_maxima_prioridad()
    {
        for (int i = 0; i < 3; i++)
        {
            if (!colas[i].esvacia())
            {
                return colas[i].eliminar_primero();
            }
        }

        // Si las 3 colas están vacías
        return Comida(-1, -1, -1);
    }

    /*
     * @brief Extrae comida aplicando ponderación probabilística (Weighted Fair Queueing).
     * @details Le asigna aproximadamente un 70% de probabilidad de salida a la Prioridad 3 (colas[2]),
     *          20% a la Prioridad 2 (colas[1]) y 10% a la Prioridad 1 (colas[0]).
     *          Si la cola seleccionada por el dado aleatorio está vacía, busca en las demás en orden secundario.
     * @return Comida El objeto extraído o Comida(-1, -1, -1) si el sistema está totalmente vacío.
     */
    Comida extraer_segun_probabilidad()
    {
        if (esvacia())
        {
            return Comida(-1, -1, -1);
        }

        int dado = (std::rand() % 100) + 1; // Genera un entero entre 1 y 100
        int ordenDeBusqueda[3];

        if (dado <= 70) 
        {
            // 70% de probabilidad: Intentar Prioridad 3 primero, luego 2, luego 1
            ordenDeBusqueda[0] = 2; 
            ordenDeBusqueda[1] = 1; 
            ordenDeBusqueda[2] = 0;
        } 
        else if (dado <= 90) 
        {
            // 20% de probabilidad: Intentar Prioridad 2 primero, luego 3, luego 1
            ordenDeBusqueda[0] = 1; 
            ordenDeBusqueda[1] = 2; 
            ordenDeBusqueda[2] = 0;
        } 
        else 
        {
            // 10% de probabilidad: Intentar Prioridad 1 primero, luego 2, luego 3
            ordenDeBusqueda[0] = 0; 
            ordenDeBusqueda[1] = 1; 
            ordenDeBusqueda[2] = 2;
        }

        // Recorrer las colas en el orden determinado por el dado aleatorio
        for (int i = 0; i < 3; i++)
        {
            int idx = ordenDeBusqueda[i];
            if (!colas[idx].esvacia())
            {
                return colas[idx].eliminar_primero();
            }
        }

        return Comida(-1, -1, -1);
    }

    /*
     * @brief Extrae comida omitiendo la Prioridad 1 (Alta) para simular escasez.
     * @details Diseñado para las estaciones de Otoño e Invierno. Asigna un 80% de 
     *          probabilidad de salida a la Prioridad 3 y un 20% a la Prioridad 2. 
     *          Si la cola seleccionada está vacía, busca en la otra cola disponible.
     * @return Comida El objeto extraído de Prioridad 2 o 3, o Comida(-1, -1, -1) si no hay disponibilidad.
     */
    Comida extraer_sin_prioridad_alta()
    {
        if (colas[1].esvacia() && colas[2].esvacia())
        {
            return Comida(-1, -1, -1);
        }

        int dado = (std::rand() % 100) + 1;

        if (dado <= 80)
        {
            if (!colas[2].esvacia()) return colas[2].eliminar_primero();
            if (!colas[1].esvacia()) return colas[1].eliminar_primero();
        }
        else
        {
            if (!colas[1].esvacia()) return colas[1].eliminar_primero();
            if (!colas[2].esvacia()) return colas[2].eliminar_primero();
        }

        return Comida(-1, -1, -1);
    }

    /*
     * @brief Verifica si no existe comida de ningún nivel en el sistema.
     * @return true si las 3 colas están vacías, false de lo contrario.
     */
    bool esvacia() const
    {
        return colas[0].esvacia() && colas[1].esvacia() && colas[2].esvacia();
    }

    /*
     * @brief Vacía todas las colas de la estructura.
     */
    void vaciar()
    {
        while (!esvacia())
        {
            extraer_maxima_prioridad();
        }
    }
};

#endif