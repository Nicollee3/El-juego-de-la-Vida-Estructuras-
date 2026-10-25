/**
 * @file AdministradorComida.h
 * @brief Gestión de comida mediante Arreglo Dinámico y Diccionario en O(1), integrada con Estaciones.
 * @author Nicolle Ruiz 
 * @date Octubre 2026
 */

#ifndef ADMINISTRADOR_COMIDA_H
#define ADMINISTRADOR_COMIDA_H

#include <cstdlib>
#include <string>
#include "Comida.h"
#include "ArregloBichitos.h"    // Contiene la plantilla ArregloDinamico<T>
#include "Diccionario.Comida.h" 

class AdministradorComida
{
private:
    ArregloDinamico<Comida> comidasActivas; // Arreglo dinámico de objetos Comida
    TablaHashComidas tablaHash;             // El diccionario de Karoll (ID -> Índice del arreglo)
    int siguienteId;

public:
    // Porcentajes de probabilidad (0 a 99)
    static const int LIMITE_NORMAL = 70;  // 70% Comida Normal (Prioridad 3)
    static const int LIMITE_MEDIANA = 90; // 20% Comida Mediana (Prioridad 2)
                                          // 10% Comida Especial (Prioridad 1)

    AdministradorComida() : siguienteId(1) {}

    /**
     * @brief Retorna la vida/curación que otorga la comida según su prioridad.
     */
    static int puntosPorPrioridad(int prioridad)
    {
        if (prioridad == 1) return 60;
        if (prioridad == 2) return 30;
        return 15;
    }

    /**
     * @brief Genera comida considerando las probabilidades de la ruleta y la Estación del Año.
     */
    bool generar(float x, float y, const std::string& nombreEstacion, OrigenComida origen = OrigenComida::Pool)
    {
        int tirada = std::rand() % 100;
        int prioridad = 3;

        // Selección probabilística
        if (tirada < LIMITE_NORMAL)
        {
            prioridad = 3;
        }
        else if (tirada < LIMITE_MEDIANA)
        {
            prioridad = 2;
        }
        else
        {
            prioridad = 1;
        }

        // Restricción por Estación: En Otoño e Invierno NO sale comida de Prioridad 1
        if ((nombreEstacion == "Otono" || nombreEstacion == "Invierno") && prioridad == 1)
        {
            prioridad = 2; // Se degrada a prioridad media debido a la escasez
        }

        int vida = puntosPorPrioridad(prioridad);
        Comida nuevaComida(siguienteId, prioridad, vida, x, y, origen);

        // 1. Guardar en el Arreglo Dinámico
        comidasActivas.generar(nuevaComida);

        // 2. Registrar en el Diccionario (de Karoll) el ID y el índice que ocupó en el arreglo
        int ultimoIndice = comidasActivas.longitud() - 1;
        tablaHash.insertar(siguienteId, ultimoIndice);

        siguienteId++;
        return true;
    }

    /**
     * @brief Elimina una comida en O(1) cuando un bichito se la come usando Swap & Pop.
     */
    bool consumirComidaPorId(int idComida)
    {
        // 1. Buscar en el diccionario de Karoll el índice de la comida en O(1)
        int indice = tablaHash.buscarIndice(idComida);
        if (indice == -1) return false;

        // 2. Borrar del diccionario
        tablaHash.eliminar(idComida);

        // 3. Si el elemento a eliminar no es el último del arreglo dinámico,
        // al hacer Swap & Pop la última comida pasará al índice 'indice'.
        // Debemos actualizar la nueva posición de esa comida movida en el diccionario de Karoll.
        int ultimoIndice = comidasActivas.longitud() - 1;
        if (indice != ultimoIndice)
        {
            Comida comidaUltima = comidasActivas.obtener(ultimoIndice);
            tablaHash.insertar(comidaUltima.getId(), indice);
        }

        // 4. Aplicar el borrado Swap & Pop en el Arreglo Dinámico en O(1)
        comidasActivas.morir(indice);

        return true;
    }

    // Métodos de consulta
    ArregloDinamico<Comida>& getComidasActivas() { return comidasActivas; }
    int getNumComidas() const { return comidasActivas.longitud(); }
    void vaciar() { comidasActivas.vaciar(); }
};

#endif