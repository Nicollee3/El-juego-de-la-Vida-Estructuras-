/*
 * @file Bichito.h
 * @brief Representa a cada bichito dentro del juego de la vida.
 * @details Tiene atributos privados de identificación y de cuánta vida tiene. Cuenta con setters y getters con validaciones.
 * @author Karoll Ramírez
 * @OCTUBRE 2026
 */

#ifndef BICHITO_H
#define BICHITO_H

#include <cmath>

enum class EstadoBichito { BuscandoComida, BuscandoPareja, DePaseo };

class Bichito
{
private:
    int id;
    float vida;
    float x, y;
    float targetX, targetY;
    float velocidad;
    EstadoBichito estado;
    
    // Control de reproducción
    float tiempoCercaPareja;
    int idParejaObjetivo;
    float cooldownReproduccion;

public:
    Bichito() 
        : id(0), vida(30.0f), x(0.0f), y(0.0f), targetX(0.0f), targetY(0.0f),
          velocidad(120.0f), estado(EstadoBichito::BuscandoComida),
          tiempoCercaPareja(0.0f), idParejaObjetivo(-1), cooldownReproduccion(0.0f) {}

    Bichito(int _id, float _vida, float _x = 0.0f, float _y = 0.0f) 
        : id(_id), vida(_vida), x(_x), y(_y), targetX(_x), targetY(_y),
          velocidad(120.0f), estado(EstadoBichito::BuscandoComida),
          tiempoCercaPareja(0.0f), idParejaObjetivo(-1), cooldownReproduccion(0.0f) {}

    // Getters
    int getId() const { return id; }
    float getVida() const { return vida; }
    float getX() const { return x; }
    float getY() const { return y; }
    float getTargetX() const { return targetX; }
    float getTargetY() const { return targetY; }
    float getVelocidad() const { return velocidad; }
    EstadoBichito getEstado() const { return estado; }
    float getTiempoCercaPareja() const { return tiempoCercaPareja; }
    int getIdParejaObjetivo() const { return idParejaObjetivo; }
    float getCooldownReproduccion() const { return cooldownReproduccion; }

    // Setters
    void setId(int _id) { id = _id; }
    bool setVida(float _vida) {
        if (_vida >= 0.0f && _vida <= 200.0f) {
            vida = _vida;
            return true;
        }
        return false;
    }
    void setPosicion(float _x, float _y) { x = _x; y = _y; }
    void setTarget(float _tx, float _ty) { targetX = _tx; targetY = _ty; }
    void setVelocidad(float _v) { velocidad = _v; }
    void setEstado(EstadoBichito _e) { estado = _e; }
    void setTiempoCercaPareja(float _t) { tiempoCercaPareja = _t; }
    void setIdParejaObjetivo(int _idP) { idParejaObjetivo = _idP; }
    void setCooldownReproduccion(float _cd) { cooldownReproduccion = _cd; }

    // Movimiento continuo en línea recta hacia el objetivo
    void moverHaciaObjetivo(float dt) {
        float dx = targetX - x;
        float dy = targetY - y;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist > 2.0f) {
            float dirX = dx / dist;
            float dirY = dy / dist;
            x += dirX * velocidad * dt;
            y += dirY * velocidad * dt;
        }
    }
};

#endif