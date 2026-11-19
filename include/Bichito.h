/*
- @file Bichito.h
- @brief Representa a cada bichito dentro del juego de la vida.
- @details Tiene atributos privados de identificación y de cuánta vida tiene. Cuenta con setters y getters con validaciones.
-          La lógica de comida y población utiliza ArregloDinamico.
- @author Karoll Ramírez / Nicolle (Genética)
- @OCTUBRE 2026
 */

#ifndef BICHITO_H
#define BICHITO_H

#include <cmath>
// INCLUSIÓN NUEVA: Incorporamos la estructura genética
#include "Genoma.h" 

class Comida;
template <typename T>
class Diccionario;
template <typename T>
class ArregloDinamico;
class RTree;

/*
- @enum EstadoBichito
- @brief Define los posibles estados de comportamiento de un bichito en la simulación.
 */
enum class EstadoBichito
{
    BuscandoComida,
    BuscandoPareja,
    DePaseo,
    Cazando // NUEVO ESTADO: Añadido para soportar el gen Caníbal
};

/*
- @class Bichito
- @brief Modela una entidad autónoma (bichito) con ciclos de vida y movimiento.
 */
class Bichito
{
private:
    int id;
    float vida;
    float edadActual;
    float edadMaxima;
    bool inmortal;
    float x, y;
    float targetX, targetY;
    float velocidad;
    EstadoBichito estado;
    float tiempoCercaPareja;
    int idParejaObjetivo;
    float cooldownReproduccion;
    bool siguiendoComida;
    bool condenado;

    // NUEVO ATRIBUTO: El genoma que almacena los 2 genes estáticos y el metabolismo acoplado a la velocidad.
    Genoma genetica;

    // Declaración de métodos privados (sus cuerpos están en el .cpp)
    /*
- @brief Decide el estado del bichito según su vida y su cooldown.
     */
    void evaluarEstado();
    /*
- @brief Controla el movimiento de patrullaje o paseo cuando no hay estímulos cercanos.
     */
    void pasear();
    /*
- @brief Genera un x,y aleatorio para pasear()
     */
    void destinoAleatorio();
    void buscarComida(ArregloDinamico<Comida>& comidas, Diccionario<int>& indicesComida, const RTree& rtreeComida);
    
    // NUEVO MÉTODO PRIVADO: Exclusivo para bichitos con el gen Caníbal
    void buscarPresa(ArregloDinamico<Bichito>& poblacion, Diccionario<int>& indices);

    void buscarPareja(ArregloDinamico<Bichito>& poblacion, Diccionario<int>& indices, float dt);
    void reproducirseCon(Bichito& pareja, ArregloDinamico<Bichito>& poblacion,
                        Diccionario<int>& indices);

public:
    /*
- @brief Construye un nuevo objeto Bichito con valores predeterminados.
     */
    Bichito();
    
    // CONSTRUCTOR MODIFICADO: Ahora puede recibir un Genoma al nacer (con un valor por defecto para los fundadores)
    Bichito(int _id, float _vida, float _x = 0.0f, float _y = 0.0f, Genoma _genes = Genoma());

    /*
- @GETTERS Y SETTERS
- @brief Métodos de acceso y modificación para los atributos privados del bichito.
     */
    int getId() const { return id; }
    float getVida() const { return vida; }
    float getEdadActual() const { return edadActual; }
    float getEdadMaxima() const { return edadMaxima; }
    bool esInmortal() const { return inmortal; } // Nota: La lógica interna revisará genetica.genEstatico1/2
    float getX() const { return x; }
    float getY() const { return y; }
    float getTargetX() const { return targetX; }
    float getTargetY() const { return targetY; }
    float getVelocidad() const { return velocidad; }
    EstadoBichito getEstado() const { return estado; }
    float getTiempoCercaPareja() const { return tiempoCercaPareja; }
    int getIdParejaObjetivo() const { return idParejaObjetivo; }
    float getCooldownReproduccion() const { return cooldownReproduccion; }
    
    // NUEVO GETTER: Para acceder a los genes desde fuera (ej. para el Árbol Genealógico)
    Genoma getGenetica() const { return genetica; }

    void setId(int _id) { id = _id; }
    bool setVida(float _vida);
    void setEdadActual(float _edad) { edadActual = (_edad >= 0.0f) ? _edad : 0.0f; }
    void setEdadMaxima(float _edadMax);
    void setInmortal(bool _inm) { inmortal = _inm; }
    void envejecer(float dt) { if (dt > 0.0f) edadActual += dt; }
    bool murioDeHambre() const { return vida <= 0.0f; }
    bool murioPorVejez() const { return (!inmortal && edadActual >= edadMaxima); }
    
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
    // SETTER MODIFICADO: Al cambiar la velocidad, se actualiza el gen variable del metabolismo
    void setVelocidad(float _v) 
    { 
        velocidad = _v; 
        genetica.velocidad = _v;
        genetica.metabolismo = _v * 2.5f; 
    }
    void setEstado(EstadoBichito _e) { estado = _e; }
    void setTiempoCercaPareja(float _t) { tiempoCercaPareja = _t; }
    void setIdParejaObjetivo(int _idP) { idParejaObjetivo = _idP; }
    void setCooldownReproduccion(float _cd) { cooldownReproduccion = _cd; }
    void setSiguiendoComida(bool _sc) { siguiendoComida = _sc; }
    bool estaCondenado() const { return condenado; }
    bool condenar();

    /*
- @brief Desplaza al bichito de manera fluida hacia su posición objetivo (targetX, targetY).
- @param dt Incremento de tiempo (delta time) transcurrido en el fotograma.
     */
    void moverHaciaObjetivo(float dt);
    void actualizarSoloPaseo(float dt);
    /*
- @brief Actualiza la lógica del bichito (estado, movimiento) en cada ciclo del juego.
- @details La vida NO baja aquí: el desgaste por segundo lo aplica el juego con cambiarVida(-1).
- @param dt Incremento de tiempo transcurrido.
     */
    void actualizar(float dt, ArregloDinamico<Comida>& comidas, Diccionario<int>& indicesComida,
                    const RTree& rtreeComida, ArregloDinamico<Bichito>& poblacion, Diccionario<int>& indices);
    
    /*
- @brief Incrementa o decrementa la vida actual del bichito de forma segura (siempre queda entre 0 y 200).
- @param delta Cantidad de vida a sumar (positiva) o restar (negativa).
     */
    void cambiarVida(float delta);
};

#endif