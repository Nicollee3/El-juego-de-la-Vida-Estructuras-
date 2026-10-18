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
/*
 * @enum EstadoBichito
 * @brief Define los posibles estados de comportamiento de un bichito en la simulación.
 */
enum class EstadoBichito { BuscandoComida, BuscandoPareja, DePaseo };
/*
 * @class Bichito
 * @brief Modela una entidad autónoma (bichito) con ciclos de vida, movimiento y reproducción.
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

    // Declaración de métodos privados (sus cuerpos se mueven al .cpp)
    void evaluarEstado();
    /*
     * @brief Busca la comida más cercana dentro del vector de elementos disponibles.
     * @param comidas Vector de objetos Comida presentes en el entorno.
     */
    void buscarComida(std::vector<Comida>& comidas);
    /*
     * @brief Gestiona la búsqueda y aproximación hacia un compañero compatible para la reproducción.
     * @param poblacion Referencia a la estructura que almacena la población actual de bichitos.
     * @param dt Incremento de tiempo (delta time) transcurrido desde el último fotograma.
     */
    void buscarPareja(PilaDinamica& poblacion, float dt);
    /*
     * @brief Ejecuta la lógica de reproducción con una pareja seleccionada.
     * @param pareja Referencia al bichito con el cual se va a reproducir.
     * @param poblacion Referencia a la población para registrar al nuevo descendiente.
     */
    void reproducirseCon(Bichito& pareja, PilaDinamica& poblacion);
    /*
     * @brief Controla el movimiento de patrullaje o paseo cuando no hay estímulos cercanos.
     */
    void pasear();

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
    void setPosicion(float _x, float _y) { x = _x; y = _y; }
    void setTarget(float _tx, float _ty) { targetX = _tx; targetY = _ty; }
    void setVelocidad(float _v) { velocidad = _v; }
    void setEstado(EstadoBichito _e) { estado = _e; }
    void setTiempoCercaPareja(float _t) { tiempoCercaPareja = _t; }
    void setIdParejaObjetivo(int _idP) { idParejaObjetivo = _idP; }
    void setCooldownReproduccion(float _cd) { cooldownReproduccion = _cd; }

    // Declaración de métodos con lógica pesada (sus cuerpos se mueven al .cpp)
    /*
     * @brief Desplaza al bichito de manera fluida hacia su posición objetivo (targetX, targetY).
     * @param dt Incremento de tiempo (delta time) transcurrido en el fotograma.
     */
    void moverHaciaObjetivo(float dt);
    /*
     * @brief Actualiza la lógica del bichito (comportamiento, movimiento, vida) en cada ciclo del juego.
     * @param dt Incremento de tiempo transcurrido.
     * @param comidas Vector con las fuentes de comida disponibles en la simulación.
     * @param poblacion Referencia a la estructura dinámica que almacena la población general.
     */
    void actualizar(float dt, std::vector<Comida>& comidas, PilaDinamica& poblacion);
    /*
     * @brief Incrementa o decrementa la vida actual del bichito de forma segura.
     * @param delta Cantidad de vida a sumar (positiva) o restar (negativa).
     */
    void cambiarVida(float delta);
};

#endif