/*
 * @file Bichito.h
 * @brief Representa a cada bichito dentro del juego de la vida.
 * @details Tiene atributos privados de identificación y de cuánta vida tiene. Cuenta con setters y getters con validaciones.
 *          Versión SIN comida y SIN pila: no depende de Comida.h ni de PilaDinamica.
 * @author Karoll Ramírez
 * @OCTUBRE 2026
 */

#ifndef BICHITO_H
#define BICHITO_H

#include <cmath>

/*
 * @enum EstadoBichito
 * @brief Define los posibles estados de comportamiento de un bichito en la simulación.
 */
enum class EstadoBichito
{
    BuscandoComida,
    BuscandoPareja,
    DePaseo
};
/*
 * @class Bichito
 * @brief Modela una entidad autónoma (bichito) con ciclos de vida y movimiento.
 */
class Bichito
{
private:
    int id;
    float vida;
    float x, y;
    float targetX, targetY;
    float velocidad;
    EstadoBichito estado;
    float tiempoCercaPareja;
    int idParejaObjetivo;
    float cooldownReproduccion;
    bool siguiendoComida;

    // Declaración de métodos privados (sus cuerpos están en el .cpp)
    /*
     * @brief Decide el estado del bichito según su vida y su cooldown.
     */
    void evaluarEstado();
    /*
     * @brief Controla el movimiento de patrullaje o paseo cuando no hay estímulos cercanos.
     */
    void pasear();
    /*
     * @brief Genera un x,y aleatorio para pasear()
     */
    void destinoAleatorio();

public:
    /*
     * @brief Construye un nuevo objeto Bichito con valores predeterminados.
     */
    Bichito();
    Bichito(int _id, float _vida, float _x = 0.0f, float _y = 0.0f);

    /*
     * @GETTERS Y SETTERS
     * @brief Métodos de acceso y modificación para los atributos privados del bichito.
     */
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

    void setId(int _id) { id = _id; }
    bool setVida(float _vida);
    void setPosicion(float _x, float _y)
    {
        x = _x;
        y = _y;
    }
    void setTarget(float _tx, float _ty)
    {
        targetX = _tx;
        targetY = _ty;
    }
    void setVelocidad(float _v) { velocidad = _v; }
    void setEstado(EstadoBichito _e) { estado = _e; }
    void setTiempoCercaPareja(float _t) { tiempoCercaPareja = _t; }
    void setIdParejaObjetivo(int _idP) { idParejaObjetivo = _idP; }
    void setCooldownReproduccion(float _cd) { cooldownReproduccion = _cd; }
    void setSiguiendoComida(bool _sc) { siguiendoComida = _sc; }

    /*
     * @brief Desplaza al bichito de manera fluida hacia su posición objetivo (targetX, targetY).
     * @param dt Incremento de tiempo (delta time) transcurrido en el fotograma.
     */
    void moverHaciaObjetivo(float dt);
    /*
     * @brief Actualiza la lógica del bichito (estado, movimiento) en cada ciclo del juego.
     * @details La vida NO baja aquí: el desgaste por segundo lo aplica el juego con cambiarVida(-1).
     * @param dt Incremento de tiempo transcurrido.
     */
    void actualizar(float dt);
    /*
     * @brief Incrementa o decrementa la vida actual del bichito de forma segura (siempre queda entre 0 y 100).
     * @param delta Cantidad de vida a sumar (positiva) o restar (negativa).
     */
    void cambiarVida(float delta);
};

#endif
