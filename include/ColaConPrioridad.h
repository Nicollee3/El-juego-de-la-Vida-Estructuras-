
/*
 * @file ColaPrioridad.h
 * @brief Cola de prioridad de Bichitos con 4 niveles fijos.
 * @details Es un arreglo FIJO de 4 objetos Cola; cada Cola es un nivel de prioridad.
 *          El nivel de un bichito depende de su VIDA en el momento de insertarlo
 *          (el nivel 0 es el de mayor prioridad):
 *              0: vida <= 0   (muerto)
 *              1: vida <  7   (crítico)
 *              2: vida <  30  (hambriento)
 *              3: vida >= 30  (sano, hasta 100)
 *          Dentro de un mismo nivel sale primero el que llegó primero (FIFO).
 *              insertar           O(1)
 *              eliminar_primero   O(1)  
 *              ver_primero        O(1)
 * @author Karoll Ramírez
 * @date Octubre 2026
 */

#ifndef COLA_PRIORIDAD_H
#define COLA_PRIORIDAD_H

#include "Cola.h"

    /*
     * @class ColaPrioridad
     * @brief Cola con prioridad de 4 niveles. Cada nivel es una Cola de Bichitos.
     */
    class ColaPrioridad
{
private:
    Cola niveles[4]; // Arreglo FIJO de 4 colas (una por nivel)
    int N;

public:
    /*
     * @brief Construye una cola de prioridad vacía.
     */
    ColaPrioridad()
    {
        N = 0;
    }

    /*
     * @brief Dice en qué nivel (0 a 3) cae una vida dada.
     * @param vida Vida del bichito.
     * @return int El número del nivel.
     */
    static int nivelDe(float vida)
    {
        if (vida <= 0)
            return 0;
        if (vida < 7)
            return 1;
        if (vida < 30)
            return 2;
        return 3;
    }

    /*
     * @brief Verifica si la cola de prioridad está vacía.
     */
    bool esvacia() const
    {
        return N == 0;
    }

    /*
     * @brief Cantidad total de bichitos en todos los niveles. O(1).
     */
    int longitud() const
    {
        return N;
    }

    /*
     * @brief Cantidad de bichitos que hay en un nivel (sirve para estadísticas).
     * @param nivel Número del nivel (0 a 3). Si es inválido devuelve 0.
     */
    int longitudNivel(int nivel) const
    {
        if (nivel < 0 || nivel >= 4)
            return 0;
        return niveles[nivel].longitud();
    }

    /*
     * @brief Inserta una copia del bichito al final de la cola del nivel que le corresponde según su vida. O(1).
     * @param b Bichito a insertar.
     */
    void insertar(const Bichito &b)
    {
        niveles[nivelDe(b.getVida())].insertar_al_ultimo(b);
        N++;
    }

    /*
     * @brief Consulta el bichito de mayor prioridad SIN sacarlo.
     * @param salida Aquí se guarda una copia del bichito.
     * @return false si la cola de prioridad estaba vacía.
     */
    bool ver_primero(Bichito &salida) const
    {
        for (int i = 0; i < 4; i++)
        {
            if (niveles[i].ver_primero(salida))
                return true;
        }
        return false;
    }

    /*
     * @brief Saca el bichito de mayor prioridad: el primero del nivel más bajo que tenga bichitos.
     * @param salida Aquí se guarda el bichito extraído.
     * @return false si la cola de prioridad estaba vacía.
     */
    bool eliminar_primero(Bichito &salida)
    {
        for (int i = 0; i < 4; i++)
        {
            if (niveles[i].eliminar_primero(salida))
            {
                N--;
                return true;
            }
        }
        return false;
    }
};

#endif
