/*
 * @file Comida.h
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
    float x, y;
    bool activa;

public:
    Comida() : id(0), prioridad(3), puntos_de_vida(20), x(0.0f), y(0.0f), activa(true) {}

    Comida(int _id, int _prioridad, int _puntos_de_vida, float _x = 0.0f, float _y = 0.0f)
        : id(_id), prioridad(_prioridad), puntos_de_vida(_puntos_de_vida), x(_x), y(_y), activa(true) {}

    int getId() const { return id; }
    int getPrioridad() const { return prioridad; }
    int getPuntosDeVida() const { return puntos_de_vida; }
    float getX() const { return x; }
    float getY() const { return y; }
    bool isActiva() const { return activa; }

    void setId(int _id) { id = _id; }
    void setPosicion(float _x, float _y) { x = _x; y = _y; }
    void setActiva(bool a) { activa = a; }

    bool setPrioridad(int _prioridad) {
        if (_prioridad >= 1 && _prioridad <= 3) {
            prioridad = _prioridad;
            return true;
        }
        return false;
    }

    bool setPuntos_De_Vida(int _puntos_de_vida) {
        if (_puntos_de_vida >= 1 && _puntos_de_vida <= 100) {
            puntos_de_vida = _puntos_de_vida;
            return true;
        }
        return false;
    }
};

#endif