/*
 * @file ColaConPrioridad.h
 * @brief Colas de prioridad para el ciclo de mortandad:
 *        1) ColaPrioridad (3 niveles O(1)): usada por La Parca en desastres naturales.
 *           Nivel 0 (Crítico):    poca vida, enfermos o viejitos (>= 75% de su edad máxima).
 *           Nivel 1 (Intermedio): jóvenes y adultos sanos.
 *           Nivel 2 (Protegido):  inmunes y recién nacidos.
 *           Los inmortales no entran a esta cola.
 *        2) ColaPrioridadVejez (Min-Heap O(log n)): ordena a los bichitos mortales
 *           según el instante en que su edadActual alcanza su edadMaxima.
 */

#ifndef COLA_PRIORIDAD_H
#define COLA_PRIORIDAD_H

#include <utility>
#include <vector>
#include "Cola.h"

/*
 * @struct PerfilReaper
 * @brief Vista de los atributos vitales de un Bichito usados por La Parca.
 */
struct PerfilReaper
{
    int id = -1;
    float vida = 0.0f;
    float edad = 0.0f;
    float edadMaxima = 100.0f;
    bool enfermo = false;
    bool inmune = false;
    bool inmortal = false;
};

/*
 * @class ColaPrioridad
 * @brief Cola con prioridad de 3 niveles fijos para desastres naturales (O(1)).
 */
class ColaPrioridad
{
public:
    static constexpr int NUM_NIVELES = 3;

    static constexpr float UMBRAL_POCA_VIDA = 25.0f;
    static constexpr float EDAD_RECIEN_NACIDO = 10.0f;
    static constexpr float FRACCION_VIEJO = 0.75f;

private:
    Cola<int> niveles[NUM_NIVELES]; // 0: mueren primero, 1: intermedio, 2: mueren al final
    int N;

public:
    ColaPrioridad() : N(0) {}

    static int nivelDe(const PerfilReaper &perfil)
    {
        if (perfil.vida <= 0.0f)
            return 0;

        if (perfil.inmune)
            return 2;

        const bool esViejo = (perfil.edadMaxima > 0.0f) &&
                             (perfil.edad >= perfil.edadMaxima * FRACCION_VIEJO);

        // Nivel 1 (índice 0): poca vida, enfermos o viejitos.
        if (perfil.vida < UMBRAL_POCA_VIDA || perfil.enfermo || esViejo)
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

    bool insertar(const PerfilReaper &perfil)
    {
        if (perfil.inmortal)
            return false;

        const int nivel = nivelDe(perfil);
        niveles[nivel].insertar_al_ultimo(perfil.id);
        ++N;
        return true;
    }

    bool ver_primero(int &idSalida) const
    {
        for (int i = 0; i < NUM_NIVELES; ++i)
        {
            if (niveles[i].ver_primero(idSalida))
                return true;
        }
        return false;
    }

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

    void vaciar()
    {
        for (int i = 0; i < NUM_NIVELES; ++i)
        {
            niveles[i].vaciar();
        }
        N = 0;
    }
};

/*
 * @struct EntradaVejez
 * @brief Registro de un bichito mortal en la cola de prioridad por esperanza de vida.
 */
struct EntradaVejez
{
    int id = -1;
    float tiempoMuerte = 0.0f; // instante global en que edadActual == edadMaxima
    float edadMaxima = 0.0f;
};

/*
 * @class ColaPrioridadVejez
 * @brief Min-Heap ordenado por tiempoMuerte (esperanza de vida restante).
 * @details Permite consultar en O(1) qué bichito está próximo a cumplir su edad máxima
 *          e insertar/extraer en O(log n). Los inmortales no se insertan aquí.
 */
class ColaPrioridadVejez
{
private:
    std::vector<EntradaVejez> heap;

    void subir(std::size_t idx)
    {
        while (idx > 0)
        {
            const std::size_t padre = (idx - 1) / 2;
            if (heap[idx].tiempoMuerte < heap[padre].tiempoMuerte)
            {
                std::swap(heap[idx], heap[padre]);
                idx = padre;
            }
            else
            {
                break;
            }
        }
    }

    void bajar(std::size_t idx)
    {
        const std::size_t n = heap.size();
        while (true)
        {
            std::size_t menor = idx;
            const std::size_t izq = 2 * idx + 1;
            const std::size_t der = 2 * idx + 2;

            if (izq < n && heap[izq].tiempoMuerte < heap[menor].tiempoMuerte)
                menor = izq;
            if (der < n && heap[der].tiempoMuerte < heap[menor].tiempoMuerte)
                menor = der;

            if (menor != idx)
            {
                std::swap(heap[idx], heap[menor]);
                idx = menor;
            }
            else
            {
                break;
            }
        }
    }

public:
    bool esvacia() const { return heap.empty(); }

    int longitud() const { return static_cast<int>(heap.size()); }

    void insertar(int id, float tiempoMuerte, float edadMaxima)
    {
        heap.push_back(EntradaVejez{id, tiempoMuerte, edadMaxima});
        subir(heap.size() - 1);
    }

    bool ver_primero(EntradaVejez &salida) const
    {
        if (heap.empty())
            return false;
        salida = heap[0];
        return true;
    }

    bool eliminar_primero(EntradaVejez &salida)
    {
        if (heap.empty())
            return false;
        salida = heap[0];
        heap[0] = heap.back();
        heap.pop_back();
        if (!heap.empty())
            bajar(0);
        return true;
    }

    void vaciar()
    {
        heap.clear();
    }
};

#endif
