#include <cmath>          
#include <cstdlib>        
#include <algorithm>      
#include <vector>
#include "Bichito.h"
#include "PilaDinamica.h" 

// --- CONSTRUCTORES ---
Bichito::Bichito() 
    : id(0), vida(30.0f), x(0.0f), y(0.0f), targetX(0.0f), targetY(0.0f),
      velocidad(120.0f), estado(EstadoBichito::BuscandoComida),
      tiempoCercaPareja(0.0f), idParejaObjetivo(-1), cooldownReproduccion(0.0f) {}

Bichito::Bichito(int _id, float _vida, float _x, float _y) 
    : id(_id), vida(_vida), x(_x), y(_y), targetX(_x), targetY(_y),
      velocidad(120.0f), estado(EstadoBichito::BuscandoComida),
      tiempoCercaPareja(0.0f), idParejaObjetivo(-1), cooldownReproduccion(0.0f) {}

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
    if (vida < 50.0f) {
        estado = EstadoBichito::BuscandoComida;
    } else if (vida > 70.0f) {
        estado = EstadoBichito::BuscandoPareja;
    } else {
        estado = EstadoBichito::DePaseo;
    }
}

void Bichito::buscarComida(std::vector<Comida>& comidas) {
    if (comidas.empty()) return;

    int idxCercano = -1;
    float minDist = 999999.0f;

    for (size_t j = 0; j < comidas.size(); ++j) {
        if (!comidas[j].isActiva()) continue;
        float dx = comidas[j].getX() - x;
        float dy = comidas[j].getY() - y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist < minDist) {
            minDist = dist;
            idxCercano = static_cast<int>(j);
        }
    }

    if (idxCercano != -1) {
        setTarget(comidas[idxCercano].getX(), comidas[idxCercano].getY());

        // Comer si está al alcance
        if (minDist <= 15.0f) {
            cambiarVida(static_cast<float>(comidas[idxCercano].getPuntosDeVida()));
            comidas[idxCercano].setActiva(false);
        }
    }
}

void Bichito::buscarPareja(PilaDinamica& poblacion, float dt) {
    int idxPareja = -1;
    float minDist = 999999.0f;

    // Buscar al bichito disponible más cercano usando la PilaDinamica
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

void Bichito::reproducirseCon(Bichito& pareja, PilaDinamica& poblacion) {
    // Calcular punto medio para el bebé
    float hijoX = (this->x + pareja.getX()) / 2.0f;
    float hijoY = (this->y + pareja.getY()) / 2.0f;

    int nuevoId = rand() % 9000 + 1000;
    Bichito hijo(nuevoId, 40.0f, hijoX, hijoY);
    hijo.setTarget(hijoX + (rand() % 100 - 50), hijoY + (rand() % 100 - 50));
    
    poblacion.generar(hijo);

    // Desgaste de energía (al bajar la vida de 70, volverán a cambiar de estado)
    cambiarVida(-30.0f);
    pareja.cambiarVida(-30.0f);

    // Reiniciar contadores y cooldowns
    this->tiempoCercaPareja = 0.0f;
    pareja.setTiempoCercaPareja(0.0f);

    this->cooldownReproduccion = 10.0f; // 10 segundos sin reproducirse
    pareja.setCooldownReproduccion(10.0f);
}

void Bichito::pasear() {
    
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

void Bichito::actualizar(float dt, std::vector<Comida>& comidas, PilaDinamica& poblacion) {
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