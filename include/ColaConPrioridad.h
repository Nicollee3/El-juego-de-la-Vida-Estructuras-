/*
 * @file ColaConPrioridad.h
 * @brief Cola de prioridad de "The Reaper" con 3 niveles fijos de mortandad.
 * @details Arreglo FIJO de 3 objetos Cola<int> (almacena IDs de bichitos).
 *          El nivel de prioridad se determina a partir de PerfilReaper
 *          (nivel 0 = mayor prioridad, muere primero):
 *              0 (Nivel 1 - Crítico):    poca vida, enfermos o viejitos.
 *              1 (Nivel 2 - Intermedio): jóvenes y adultos sanos.
 *              2 (Nivel 3 - Protegido):  inmunes y recién nacidos.
 *          Los bichitos inmortales no se encolan.
 *          Dentro de un mismo nivel sale primero el que llegó primero (FIFO).
 *              insertar           O(1)
 *              eliminar_primero   O(1)
 *              ver_primero        O(1)
 */

#ifndef COLA_PRIORIDAD_H
#define COLA_PRIORIDAD_H

#include "Cola.h"

/*
 * @struct PerfilReaper
 * @brief Vista desacoplada de los atributos vitales de un Bichito usados por The Reaper.
 * @details Permite clasificar a los bichitos sin acoplar directamente la cola a los
 *          atributos de genética (Parte 1) mientras se integran las ramas.
 */
struct PerfilReaper
{
    int id = -1;
    float vida = 0.0f;
    float edad = 0.0f;
    bool enfermo = false;
    bool inmune = false;
    bool inmortal = false;
};

/*
 * @class ColaPrioridad
 * @brief Cola con prioridad de 3 niveles para The Reaper (Gestor de Mortandad).
 */
class ColaPrioridad
{
public:
    static constexpr int NUM_NIVELES = 3;

    // Umbrales de clasificación de The Reaper
    static constexpr float UMBRAL_POCA_VIDA = 25.0f;
    static constexpr float EDAD_RECIEN_NACIDO = 10.0f;
    static constexpr float EDAD_VIEJO = 60.0f;

private:
    Cola<int> niveles[NUM_NIVELES]; // 0: mueren primero, 1: intermedio, 2: mueren al final
    int N;

public:
    ColaPrioridad() : N(0) {}

    /*
     * @brief Calcula el nivel de prioridad (0 a 2) para un perfil de bichito.
     * @param perfil Datos vitales del bichito.
     * @return int 0 (máxima prioridad), 1 (intermedio) o 2 (mínima prioridad).
     */
    static int nivelDe(const PerfilReaper &perfil)
    {
        // Si ya no tiene vida, siempre es prioridad máxima.
        if (perfil.vida <= 0.0f)
            return 0;

        // Los inmunes van en el último nivel de prioridad.
        if (perfil.inmune)
            return 2;

        // Nivel 1 (índice 0): poca vida, enfermos o viejitos.
        if (perfil.vida < UMBRAL_POCA_VIDA || perfil.enfermo || perfil.edad >= EDAD_VIEJO)
            return 0;

        // Nivel 3 (índice 2): recién nacidos sanos.
        if (perfil.edad <= EDAD_RECIEN_NACIDO)
            return 2;

        // Nivel 2 (índice 1): jóvenes y adultos sanos.
        return 1;
    }

    bool esvacia() const { return N == 0; }

    int longitud() const { return N; }

    int longitudNivel(int nivel) const
    {
        if (nivel < 0 || nivel >= NUM_NIVELES)
            return 0;
        return niveles[nivel].longitud();
    }

    /*
     * @brief Inserta el ID de un bichito en el nivel que le corresponde. O(1).
     * @return true si fue encolado, false si es inmortal (exento del Reaper).
     */
    bool insertar(const PerfilReaper &perfil)
    {
        if (perfil.inmortal)
            return false;

        const int nivel = nivelDe(perfil);
        niveles[nivel].insertar_al_ultimo(perfil.id);
        ++N;
        return true;
    }

    /*
     * @brief Consulta el ID del bichito de mayor prioridad sin extraerlo. O(1).
     */
    bool ver_primero(int &idSalida) const
    {
        for (int i = 0; i < NUM_NIVELES; ++i)
        {
            if (niveles[i].ver_primero(idSalida))
                return true;
        }
        return false;
    }

    /*
     * @brief Extrae el ID del bichito de mayor prioridad y reporta de qué nivel salió. O(1).
     */
    bool eliminar_primero(int &idSalida, int *nivelSalida = nullptr)
    {
        for (int i = 0; i < NUM_NIVELES; ++i)
        {
            if (niveles[i].eliminar_primero(idSalida))
            {
                --N;
                if (nivelSalida != nullptr)
                    *nivelSalida = i;
                return true;
            }
        }
        return false;
    }

    /*
     * @brief Vacía todos los niveles de la cola.
     */
    void vaciar()
    {
        for (int i = 0; i < NUM_NIVELES; ++i)
        {
            niveles[i].vaciar();
        }
        N = 0;
    }
};

#endif
