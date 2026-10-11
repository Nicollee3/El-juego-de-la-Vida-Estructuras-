/*
 * @file COMIDA.H
 * @brief Representa a la comida en el juego.
 * @details Tiene atributos privados de su id, prioridad y puntos de vida.
 *          Tiene métodos públicos para obtener y modificar estos atributos.
 * @author Karoll Ramírez
 * @OCTUBRE 2026
 */

#ifndef COMIDA_H
#define COMIDA_H
class Comida
{
private:
    int id;
    int prioridad;
    int puntos_de_vida;

public:
    /*
     * @brief Constructor parametrizado de la clase Comida.
     * @param _id Identificador único para la comida.
     * @param _prioridad Nivel de prioridad inicial.
     * @param _puntos_de_vida Puntos de vida iniciales que otorga.
     */
    Comida(int _id, int _prioridad, int _puntos_de_vida)
    {
        id = _id;
        prioridad = _prioridad;
        puntos_de_vida = _puntos_de_vida;
    }

    // Métodos Getters
    /*
     * @brief Obtiene el identificador de la comida.
     * @return int El ID de la comida.
     */
    int getId() const
    {
        return id;
    }
    /*
     * @brief Obtiene el nivel de prioridad de la comida.
     * @return int La prioridad actual.
     */
    int getPrioridad() const
    {
        return prioridad;
    }
    /*
     * @brief Obtiene los puntos de vida que otorga la comida.
     * @return int Los puntos de vida actuales.
     */
    int getPuntosDeVida() const
    {
        return puntos_de_vida;
    }

    // Métodos Setters
    /*
     * @brief Modifica el identificador de la comida.
     * @param _id Nuevo identificador.
     */
    void setId(int _id)
    {
        id = _id;
    }
    /*
     * @brief Modifica el nivel de prioridad validando que esté entre 1 y 3.
     * @param _prioridad Nueva prioridad a asignar.
     * @return true Si la prioridad es válida y se actualizó.
     * @return false Si la prioridad está fuera de rango.
     */
    bool setPrioridad(int _prioridad)
    {
        if (_prioridad >= 1 && _prioridad <= 3)
        {
            prioridad = _prioridad;
            return true;
        }
        return false;
    }
    /*
     * @brief Modifica los puntos de vida validando que estén entre 1 y 100.
     * @param _puntos_de_vida Nuevos puntos de vida a asignar.
     * @return true Si los puntos son válidos y se actualizaron.
     * @return false Si los puntos están fuera de rango.
     */
    bool setPuntos_De_Vida(int _puntos_de_vida)
    {
        if (_puntos_de_vida >= 1 && _puntos_de_vida <= 100)
        {
            puntos_de_vida = _puntos_de_vida;
            return true;
        }
        return false;
    }
};

#endif