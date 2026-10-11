
/*
 * @file COLAS_CON_PRIORIDAD.H
 * @brief Gestiona la comida del juego mediante 3 niveles de prioridad. Por medio de colas con prioridad.
 * @details Aquí se gestiona un arreglo estático de 3 colas de COMIDA de acuerdo a la prioridad de cada elemento. (3 niveles)
 *
 * @author Karoll Ramírez
 * @author Nicolle Ruíz
 * @OCTUBRE 2026
 */

#ifndef COLA_CON_PRIORIDAD_H
#define COLA_CON_PRIORIDAD_H

#include <iostream>
#include "COMIDA.H"
#include "COLA.H"

class ColaConPrioridad
{
private:
    Cola arreglo_colas[3];
    int total_niveles;

public:
    /*
     * @brief Construye una nueva cola con prioridad de 3 niveles.
     */
    ColaConPrioridad()
    {
        total_niveles = 3;
    }
    /*
     * @brief Verifica si todas las colas de prioridad se encuentran completamente vacías.
     * @return true Si no existe comida en ninguno de los 3 niveles.
     * @return false Si al menos una cola contiene elementos de comida.
     */
    bool esta_vacia() const
    {
        for (int i = 0; i < total_niveles; i++)
        {
            if (!arreglo_colas[i].esvacia())
            {
                return false;
            }
        }
        return true;
    }

    /*
     * @brief Inserta un objeto Comida en la cola correspondiente según su nivel de prioridad.
     * @details Mapea el rango de prioridad (1-3) al índice del arreglo interno (0-2).
     * @param c Objeto Comida que se desea insertar en el sistema.
     */
bool insertar(Comida c)
{
    int prioridad = c.getPrioridad();

    if (prioridad >= 1 && prioridad <= 3)
    {
        arreglo_colas[prioridad - 1].insertar_al_ultimo(c);
        return true; 
    }
    
    return false; 
}

    Comida extraer()
    {
        for (int i = 0; i < total_niveles; i++)
            // ALEATORIEDAD, QUE SALGAN MAS DE LAS COMIDAS DE BAJA PRIORIDAD OSEA 3, MAS O MENOS DE PRIORIDAD 2 Y MUCHAS DE PRIORIDAD 1
            else
            {
                return Comida(-1, -1, -1);
            }
    }
};

#endif