/*
 * @file BICHITO.H
 * @brief Representa a cada bichito dentro del juego de la vida.
 * @details Tiene atributos privados de identificación y de cuánta vida tiene. Cuenta con setters y getters con validaciones.
 * @author Karoll Ramírez
 * @OCTUBRE 2026
 */

#ifndef BICHITO_H
#define BICHITO_H

class Bichito
{
private:
    int id;
    int vida;
public:
    Bichito()
    /*
     * @brief Constructor por defecto.
     */
    {
        id = 0;
        vida = 30;
    }
    /*
     * @brief Constructor parametrizado.
     * @param _id Identificador único.
     * @param _vida Vida inicial del bichito.
     */
    Bichito(int _id, int _vida)
    {
        id = _id;
        vida = _vida;
    }
    // GETTERS
    /*
     * @brief Obtiene el ID del bichito.
     * @return int El ID.
     */
    int getId() const
    {
        return id;
    }

    /*
     * @brief Obtiene la vida actual del bichito.
     * @return int Los puntos de vida.
     */
    int getVida() const
    {
        return vida;
    }
    // SETTERS

    /*
     * @brief Modifica el ID del bichito.
     * @param _id Nuevo ID.
     */
    void setId(int _id)
    {
        id = _id;
    }
    /*
     * @brief Modifica la vida del bichito validando que esté estrictamente entre 0 y 100.
     * @param _vida Nueva cantidad de vida.
     * @return true Si la vida está en el rango permitido y se actualizó.
     * @return false Si el valor está fuera del rango de 0 a 100.
     */
    bool setVida(int _vida)
    {
        if (_vida >= 0 && _vida <= 100)
        {
            vida = _vida;
            return true;
        }
        return false; 
    }
};

#endif