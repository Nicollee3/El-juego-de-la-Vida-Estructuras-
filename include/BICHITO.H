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
#include <Comida.h>

class PilaDinamica;

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

    // Declaración de métodos privados
    void evaluarEstado();
    void buscarComida(std::vector<Comida>& comidas);
    void buscarPareja(PilaDinamica& poblacion, float dt);
    void reproducirseCon(Bichito& pareja, PilaDinamica& poblacion);
    void pasear();

public:
    // Constructores
    Bichito();
    Bichito(int _id, float _vida, float _x = 0.0f, float _y = 0.0f);

    // Getters simples (pueden quedarse en el .h por ser de 1 línea)
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

    // Setters simples
    void setId(int _id) { id = _id; }
    bool setVida(float _vida);
    void setPosicion(float _x, float _y) { x = _x; y = _y; }
    void setTarget(float _tx, float _ty) { targetX = _tx; targetY = _ty; }
    void setVelocidad(float _v) { velocidad = _v; }
    void setEstado(EstadoBichito _e) { estado = _e; }
    void setTiempoCercaPareja(float _t) { tiempoCercaPareja = _t; }
    void setIdParejaObjetivo(int _idP) { idParejaObjetivo = _idP; }
    void setCooldownReproduccion(float _cd) { cooldownReproduccion = _cd; }

    // Declaración de métodos con lógica pesada (sus cuerpos se mueven al .cpp)
    void moverHaciaObjetivo(float dt);
    void actualizar(float dt, std::vector<Comida>& comidas, PilaDinamica& poblacion);
    void cambiarVida(float delta);
};

#endif