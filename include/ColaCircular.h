/*
 * @file ColaCircular.h
 * @brief Implementación de una Cola Circular de tamaño fijo para gestionar las Estaciones del Año.
 * @details Utiliza un arreglo estático de 4 elementos y el operador de residuo (%) para 
 *          garantizar que el ciclo de estaciones rote infinitamente (Primavera -> Verano -> Otoño -> Invierno).
 * 
 * @Nicolle Ruiz
 * @OCTUBRE 2026
 */

#ifndef COLA_CIRCULAR_H
#define COLA_CIRCULAR_H

#include <iostream>
#include <string>
using namespace std;

/*
 * @brief Estructura que define las propiedades ambientales de cada estación del año.
 */
struct Estacion {
    string nombre;         // Nombre de la estación 
    float tasaComida;          // Multiplicador de probabilidad de aparición de comida
    int desgasteVida;          // Desgaste de vida que sufren los bichitos por ciclo
};

class ColaCircular
{
public:
    /*
     * @brief Constructor por defecto.
     * @details Inicializa las 4 estaciones del año en orden natural y configura 
     *          los punteros de frente y final de la cola circular.
     */
    ColaCircular()
    {
        capacidad = 4;
        frente = 0;
        final = 3;
        cantidad = 4; // Inicialmente cargamos las 4 estaciones fijas

        // Configuración de las 4 estaciones del año
        estaciones[0] = {"Primavera", 1.5f, 1}; // Mucha comida, desgaste normal (-1)
        estaciones[1] = {"Verano",    1.0f, 1}; // Comida normal, desgaste normal (-1)
        estaciones[2] = {"Otono",     0.7f, 2}; // Poca comida, desgaste moderado (-2)
        estaciones[3] = {"Invierno",  0.3f, 3}; // Escasa comida, desgaste alto (-3)
    }

    /*
     * @brief Avanza circularmente a la siguiente estación del año.
     * @details Aplica la fórmula del operador módulo: frente = (frente + 1) % capacidad
     *          Haciendo que al llegar al índice 3 (Invierno), el siguiente sea 0 (Primavera).
     */
    void siguiente_estación()
    {
        if (esVacia()) return;
        
        // Avance circular del puntero frente
        frente = (frente + 1) % capacidad;
        final = (final + 1) % capacidad;
    }

    /*
     * @brief Obtiene la estación del año activa actualmente sin eliminarla.
     * @return Estacion Datos de la estación en el frente de la cola circular.
     */
    Estacion obtener_actual()
    {
        if (esVacia())
        {
            return {"Desconocida", 0.0f, 0};
        }
        return estaciones[frente];
    }

    /*
     * @brief Muestra por pantalla el orden del ciclo empezando por la estación actual.
     */
    void mostrar_ciclo()
    {
        cout << "Ciclo de Estaciones: [ ";
        for (int i = 0; i < cantidad; i++)
        {
            int idx = (frente + i) % capacidad;
            cout << estaciones[idx].nombre << " ";
        }
        cout << "]\n";
    }

    /*
     * @brief Verifica si la cola circular carece de elementos.
     */
    bool esVacia()
    {
        return cantidad == 0;
    }

    /*
     * @brief Verifica si la cola circular ha alcanzado su capacidad máxima.
     */
    bool esLlena()
    {
        return cantidad == capacidad;
    }

    /*
     * @brief Retorna la cantidad total de estaciones registradas en la cola circular.
     */
    int longitud()
    {
        return cantidad;
    }

protected:
    Estacion estaciones[4]; // Arreglo estático con capacidad para las 4 estaciones
    int frente;             // Índice de la estación actual
    int final;              // Índice de la última estación del ciclo
    int capacidad;          // Tamaño máximo del arreglo (4)
    int cantidad;           // Cantidad actual de elementos en la cola
};

#endif