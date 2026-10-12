/*
 * @file ColaConPrioridad.h
 * @brief Gestiona la comida del juego mediante 3 niveles de prioridad por medio de colas.
 * @details Aquí se gestiona un arreglo estático de 3 colas de COMIDA de acuerdo a la prioridad de cada elemento. (3 niveles)
 *
 * @author Karoll Ramírez
 * @author Nicolle Ruíz
 * @OCTUBRE 2026
 */

#ifndef COLA_CON_PRIORIDAD_H
#define COLA_CON_PRIORIDAD_H

#include <iostream>
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
        // Las colas dentro del arreglo se inicializan automáticamente con el constructor de Cola
    }

    /*
     * @brief Inserta un objeto Comida en la cola correspondiente a su nivel de prioridad.
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
     * @brief Extrae y retorna la comida de mayor prioridad disponible en la simulación.
     * @details Revisa en orden jerárquico desde la cola de Prioridad 1 (índice 0) hasta la Prioridad 3 (índice 2).
     * @return Comida El objeto extraído o Comida(-1, -1, -1) si no hay comida disponible.
     */
    Comida extraer_maxima_prioridad()
    {
        // Busca desde la prioridad más alta (0) a la más baja (2)
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
     * @brief Verifica si no existe comida de ningún nivel en el sistema.
     * @return true si las 3 colas están vacías, false de lo contrario.
     */
    bool esvacia() const
    {
        return colas[0].esvacia() && colas[1].esvacia() && colas[2].esvacia();
    }
};

#endif