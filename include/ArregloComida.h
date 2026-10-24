/*
 * @file ArregloComida.h
 * @brief Arreglo dinámico de Comida que crece y se encoge, con generación por ruleta de probabilidades.
 * @details Cada vez que se genera comida se "gira una ruleta" de 100 casillas (tirada de 0 a 99):
 *              0  a 69  -> comida NORMAL   (70%)  prioridad 3,  15 puntos de vida
 *              70 a 89  -> comida MEDIANA  (20%)  prioridad 2,  30 puntos de vida
 *              90 a 99  -> comida ESPECIAL (10%)  prioridad 1,  60 puntos de vida
 *          El arreglo crece multiplicando su tamaño por la razón áurea cuando se llena,
 *          y se encoge cuando queda muy vacío (nunca baja de 10 casillas).
 *              agregar / generar   O(1) amortizado
 *              obtener             O(1)
 *              eliminar(i)         O(n)  (corre los elementos para no dejar huecos y mantener el orden)
 * @author Karoll Ramírez
 * @date Octubre 2026
 */
 
#ifndef ARREGLO_COMIDA_H
#define ARREGLO_COMIDA_H
 
#include <cstdlib>
#include "Comida.h"
 
class ArregloComida
{
public:
    // Límites de la ruleta (tirada de 0 a 99)
    static const int LIMITE_NORMAL = 70;  // tiradas 0..69  -> 70%
    static const int LIMITE_MEDIANA = 90; // tiradas 70..89 -> 20%  (90..99 -> 10% especial)
 
    /*
     * @brief Construye un arreglo de comida vacío con capacidad inicial de 10.
     */
    ArregloComida()
    {
        N = 0;
        tamano = 10;
        siguienteId = 1;
        datos = new Comida[tamano];
    }
 
    /*
     * @brief Destructor: libera la memoria del arreglo.
     */
    ~ArregloComida()
    {
        delete[] datos;
    }
 
    ArregloComida(const ArregloComida &) = delete;
    ArregloComida &operator=(const ArregloComida &) = delete;
 
    /*
     * @brief Traduce una tirada de la ruleta (0 a 99) en la prioridad de la comida.
     * @return 3 (normal, 70%), 2 (mediana, 20%) o 1 (especial, 10%).
     */
    static int prioridadPorTirada(int tirada)
    {
        if (tirada < LIMITE_NORMAL)
            return 3;
        if (tirada < LIMITE_MEDIANA)
            return 2;
        return 1;
    }
 
    /*
     * @brief Puntos de vida que da cada tipo de comida según su prioridad.
     */
    static int puntosPorPrioridad(int prioridad)
    {
        if (prioridad == 1)
            return 60;
        if (prioridad == 2)
            return 30;
        return 15;
    }
 
    /*
     * @brief Gira la ruleta, crea una comida del tipo que salió y la inserta en el arreglo.
     * @param x, y Posición donde aparece la comida.
     * @param origen De dónde viene la comida (Pool por defecto).
     * @return Copia de la comida que se insertó.
     */
    Comida generar(float x, float y, OrigenComida origen = OrigenComida::Pool)
    {
        return generarConTirada(std::rand() % 100, x, y, origen);
    }
 
    /*
     * @brief Igual que generar(), pero con la tirada ya decidida (sirve para hacer pruebas exactas).
     * @param tirada Número de 0 a 99.
     */
    Comida generarConTirada(int tirada, float x, float y, OrigenComida origen = OrigenComida::Pool)
    {
        int prioridad = prioridadPorTirada(tirada);
        Comida c(siguienteId, prioridad, puntosPorPrioridad(prioridad), x, y, origen);
        siguienteId++;
        agregar(c);
        return c;
    }
 
    /*
     * @brief Agrega una comida al final del arreglo. Si está lleno, crece con la razón áurea.
     */
    void agregar(const Comida &c)
    {
        if (N == tamano)
            cambiarTamano((int)(tamano * aureo));
        datos[N] = c;
        N++;
    }
 
    /*
     * @brief Acceso a la comida de la posición i.
     * @return Puntero a la comida (se puede modificar), o nullptr si i es inválido.
     * @warning El puntero deja de ser válido si luego agregas o eliminas (el arreglo puede moverse en memoria).
     */
    Comida *obtener(int i)
    {
        if (i < 0 || i >= N)
            return nullptr;
        return &datos[i];
    }
 
    const Comida *obtener(int i) const
    {
        if (i < 0 || i >= N)
            return nullptr;
        return &datos[i];
    }
 
    /*
     * @brief Elimina la comida de la posición i, corriendo las siguientes una casilla a la izquierda.
     * @details Si el arreglo queda muy vacío, se encoge.
     * @return false si el índice es inválido.
     */
    bool eliminar(int i)
    {
        if (i < 0 || i >= N)
            return false;
        for (int k = i; k < N - 1; k++)
            datos[k] = datos[k + 1];
        N--;
 
        if (N * aureo * aureo < tamano && tamano > 10)
            cambiarTamano((int)(N * aureo));
        return true;
    }
 
    int longitud() const { return N; }
    int capacidad() const { return tamano; }
    bool esvacio() const { return N == 0; }
 
private:
    Comida *datos;
    int N;
    int tamano;
    int siguienteId; // Cada comida generada recibe un id nuevo
    double aureo = 1.618;
 
    /*
     * @brief Cambia la capacidad del arreglo (mínimo 10, y siempre mayor que la cantidad de elementos).
     */
    void cambiarTamano(int ntamano)
    {
        if (ntamano < 10)
            ntamano = 10;
        if (ntamano <= N)
            ntamano = N + 1;
        Comida *ndatos = new Comida[ntamano];
        for (int i = 0; i < N; i++)
            ndatos[i] = datos[i];
        delete[] datos;
        datos = ndatos;
        tamano = ntamano;
    }
};
 
#endif
 