#include <cmath>          
#include <cstdlib>        
#include <algorithm>      
#include "Bichito.h"
#include "ArregloDinamico.h"
#include "gui/Config.h"

// --- CONSTRUCTORES ---
Bichito::Bichito() 
    : id(0), vida(30.0f), x(0.0f), y(0.0f), targetX(0.0f), targetY(0.0f),
      velocidad(120.0f), estado(EstadoBichito::BuscandoComida),
      tiempoCercaPareja(0.0f), idParejaObjetivo(-1), cooldownReproduccion(0.0f), 
      siguiendoComida(false){}

Bichito::Bichito(int _id, float _vida, float _x, float _y) 
    : id(_id), vida(_vida), x(_x), y(_y), targetX(_x), targetY(_y),
      velocidad(120.0f), estado(EstadoBichito::BuscandoComida),
      tiempoCercaPareja(0.0f), idParejaObjetivo(-1), cooldownReproduccion(0.0f), 
      siguiendoComida(false) {}

// --- SETTERS CON LÓGICA ---
bool Bichito::setVida(float _vida) {
    if (_vida >= 0.0f && _vida <= 200.0f) {
        vida = _vida;
        return true;
    }
    return false;
}

void Bichito::cambiarVida(float delta) {
    float nueva = getVida() + delta;
    if (nueva < 0.0f) nueva = 0.0f;
    if (nueva > 200.0f) nueva = 200.0f;
    setVida(nueva);
}

// --- MÉTODOS PRIVADOS ---
void Bichito::evaluarEstado() {
    EstadoBichito nuevoEstado;

    if(estado == EstadoBichito::BuscandoComida && vida < 100){
        return;
    }

    if (vida > 90){
        // setVelocidad(60.0f);
        nuevoEstado = EstadoBichito::DePaseo;
    } else if (vida > 70){
        // setVelocidad(120.0f);
        nuevoEstado = EstadoBichito::BuscandoPareja;
    } else if (vida > 50){
        // setVelocidad(120.0f);
        nuevoEstado = EstadoBichito::DePaseo;
    }else{
        // setVelocidad(120.0f);
        nuevoEstado = EstadoBichito::BuscandoComida;
    }

    // Si el bichito cambia a estar de paseo baja su velocidad
    if (nuevoEstado != estado) {
        estado = nuevoEstado;
        float nVelocidad = (estado == EstadoBichito::DePaseo) ? 60.0f : 120.0f;
        setVelocidad(nVelocidad);
        siguiendoComida = false;
        if (estado == EstadoBichito::DePaseo) {
            destinoAleatorio();
        }
    }
}

void Bichito::buscarComida(ArregloDinamico<Comida>& comidas) {
    int idxCercano = -1;
    float minDist = 999999.0f;

    for (int j = 0; j < comidas.longitud(); ++j) {
        if (!comidas.obtenerRef(j).isActiva()) continue;
        float dx = comidas.obtenerRef(j).getX() - x;
        float dy = comidas.obtenerRef(j).getY() - y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist < minDist) {
            minDist = dist;
            idxCercano = j;
        }
    }

    if (idxCercano != -1) {
        Comida& masCercana = comidas.obtenerRef(idxCercano);
        setTarget(masCercana.getX(), masCercana.getY());
        siguiendoComida = true;

        // Comer si está al alcance
        if (minDist <= 15.0f) {
            cambiarVida(static_cast<float>(masCercana.getPuntosDeVida()));
            masCercana.setActiva(false);
            siguiendoComida = false;
            destinoAleatorio();
        }
    } else {
        // Si no hay comida activa o estaba persiguiendo una que ya no existe:
        if (siguiendoComida) {
            siguiendoComida = false;
            destinoAleatorio(); // Reacciona de inmediato cambiando de destino
        }
        pasear();
    }
}

void Bichito::buscarPareja(ArregloDinamico<Bichito>& poblacion, float dt) {
    int idxPareja = -1;
    float minDist = 999999.0f;

    // Buscar al bichito disponible más cercano en la población
    for (int j = 0; j < poblacion.longitud(); ++j) {
        Bichito& otro = poblacion.obtenerRef(j);

        if (otro.getId() != this->id && 
            otro.getEstado() == EstadoBichito::BuscandoPareja &&
            otro.getCooldownReproduccion() <= 0.0f) 
        {
            float dx = otro.getX() - this->x;
            float dy = otro.getY() - this->y;
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist < minDist) {
                minDist = dist;
                idxPareja = j;
            }
        }
    }

    if (idxPareja != -1) {
        Bichito& pareja = poblacion.obtenerRef(idxPareja);
        setTarget(pareja.getX(), pareja.getY());

        // Si están a distancia 
        if (minDist <= 35.0f) {
            tiempoCercaPareja += dt;

            // Al cumplir 2 segundos juntos, se reproducen
            if (tiempoCercaPareja >= 2.0f) {
                reproducirseCon(pareja, poblacion);
            }
        } else {
            tiempoCercaPareja = 0.0f; // Se alejaron, se reinicia la cuenta
        }
    } else {
        tiempoCercaPareja = 0.0f;
    }
}

void Bichito::reproducirseCon(Bichito& pareja, ArregloDinamico<Bichito>& poblacion) {
    // 1. Calcular las coordenadas y crear la instancia del hijo
    float hijoX = (this->x + pareja.getX()) / 2.0f;
    float hijoY = (this->y + pareja.getY()) / 2.0f;

    int nuevoId = rand() % 9000 + 1000;
    Bichito hijo(nuevoId, 40.0f, hijoX, hijoY);
    hijo.setTarget(hijoX + (rand() % 100 - 50), hijoY + (rand() % 100 - 50));

    // 2. Modificar a los padres
    this->cambiarVida(-30.0f);
    pareja.cambiarVida(-30.0f);

    this->tiempoCercaPareja = 0.0f;
    pareja.setTiempoCercaPareja(0.0f);

    this->cooldownReproduccion = 10.0f; // 10 segundos sin reproducirse
    pareja.setCooldownReproduccion(10.0f);

    // 3. Insertar al hijo al final
    poblacion.generar(hijo);
}

void Bichito::pasear() {
    
    float dx = targetX - x;
    float dy = targetY - y;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist <= 10.0f) {
        destinoAleatorio();
    }
}

void Bichito::destinoAleatorio() {
    float margen = 30.0f;
    float minX = margen;
    float maxX = Config::ANCHO_MUNDO - margen;
    float minY = margen;
    float maxY = Config::ALTO_MUNDO - margen;

    float rangoX = maxX - minX;
    float rangoY = maxY - minY;

    targetX = minX + (static_cast<float>(rand()) / RAND_MAX) * rangoX;
    targetY = minY + (static_cast<float>(rand()) / RAND_MAX) * rangoY;
}

// --- MÉTODOS PÚBLICOS DE MOVIMIENTO Y ACTUALIZACIÓN ---
void Bichito::moverHaciaObjetivo(float dt) {
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

void Bichito::actualizar(float dt, ArregloDinamico<Comida>& comidas,
                         ArregloDinamico<Bichito>& poblacion) {
    evaluarEstado();

    switch (estado) {
        case EstadoBichito::BuscandoComida:
            buscarComida(comidas);
            break;
        case EstadoBichito::BuscandoPareja:
            buscarPareja(poblacion, dt);
            break;
        case EstadoBichito::DePaseo:
            pasear();
            break;
    }

    moverHaciaObjetivo(dt);
}