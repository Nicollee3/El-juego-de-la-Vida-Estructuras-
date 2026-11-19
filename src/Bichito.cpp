#include <cmath>          
#include <cstdlib>        
#include <algorithm>      
#include "Bichito.h"
#include "ArregloDinamico.h"
#include "Diccionario.h"
#include "gui/Config.h"
#include "RTree.h"

namespace
{
    constexpr float RANGO_DETECCION_COMIDA = 400.0f;
    constexpr float EDAD_MAXIMA_MIN = 20.0f;
    constexpr float EDAD_MAXIMA_MAX = 400.0f;

    float generarEdadMaximaAleatoria()
    {
        const float u = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        return EDAD_MAXIMA_MIN + u * (EDAD_MAXIMA_MAX - EDAD_MAXIMA_MIN);
    }
}


Bichito::Bichito() 
    : id(0), vida(30.0f), edadActual(0.0f), edadMaxima(generarEdadMaximaAleatoria()), inmortal(false),
    x(0.0f), y(0.0f), targetX(0.0f), targetY(0.0f),
    velocidad(120.0f), estado(EstadoBichito::BuscandoComida),
    tiempoCercaPareja(0.0f), idParejaObjetivo(-1), cooldownReproduccion(0.0f), 
    siguiendoComida(false), condenado(false)
{
    // NUEVO: Codigo genetico
    genetica = Genoma(120.0f); // Inicializa genética con la velocidad base de tus compañeros
}

// Nota: Los valores por defecto se definen en el .h, aquí solo recibimos los parámetros
Bichito::Bichito(int _id, float _vida, float _x, float _y, Genoma _genes) 
    : id(_id), vida(_vida), edadActual(0.0f), edadMaxima(generarEdadMaximaAleatoria()), inmortal(false),
    x(_x), y(_y), targetX(_x), targetY(_y),
    velocidad(120.0f), estado(EstadoBichito::BuscandoComida),
    tiempoCercaPareja(0.0f), idParejaObjetivo(-1), cooldownReproduccion(0.0f), 
    siguiendoComida(false), condenado(false), 
    genetica(_genes) // --- NUEVO CÓDIGO GENÉTICO ---
{
    // Aseguramos que la velocidad del bichito empate con su genoma inicial
    setVelocidad(_genes.velocidad > 0.0f ? _genes.velocidad : 120.0f); 
}


void Bichito::setEdadMaxima(float _edadMax) {
    edadMaxima = std::max(EDAD_MAXIMA_MIN, std::min(_edadMax, EDAD_MAXIMA_MAX));
}
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

bool Bichito::condenar()
{
    if (condenado || vida >= 20.0f)
    {
        return false;
    }

    condenado = true;
    estado = EstadoBichito::DePaseo;
    siguiendoComida = false;
    setVelocidad(60.0f);
    destinoAleatorio();
    return true;
}

//  MÉTODOS PRIVADOS 
void Bichito::evaluarEstado() {
    // --- NUEVO CÓDIGO GENÉTICO: INTERCEPCIÓN CANÍBAL ---
    // Si la vida baja y el bicho tiene el gen caníbal, ignora la comida y se pone a cazar
    if (vida < 100.0f && (genetica.genEstatico1 == GenEstatico::CANIBALISMO || genetica.genEstatico2 == GenEstatico::CANIBALISMO)) {
        if (estado != EstadoBichito::Cazando) {
            estado = EstadoBichito::Cazando;
            setVelocidad(120.0f);
            siguiendoComida = false;
        }
        return; // Salimos para no ejecutar los estados normales de comida o paseo
    }
    

    EstadoBichito nuevoEstado;

    if(estado == EstadoBichito::BuscandoComida && vida < 100){
        return;
    }

    if (vida > 90){
        nuevoEstado = EstadoBichito::DePaseo;
    } else if (vida > 70){
        nuevoEstado = EstadoBichito::BuscandoPareja;
    } else if (vida > 50){
        nuevoEstado = EstadoBichito::DePaseo;
    }else{
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

void Bichito::buscarComida(ArregloDinamico<Comida>& comidas, Diccionario<int>& indicesComida, const RTree& rtreeComida) {
    int idxCercano = -1;
    float minDist = 999999.0f;

    // 1. Consulta espacial en el R-Tree (solo devuelve IDs cercanos, no todo el mapa)
    std::vector<int> idsCercanos = rtreeComida.buscarCercanas(x, y, RANGO_DETECCION_COMIDA);

    // 2. Buscamos en O(1) usando el Diccionario de comidas
    for (int idComida : idsCercanos) {
        int idx = -1;
        if (!indicesComida.buscar(idComida, idx)) continue;

        Comida& c = comidas.obtenerRef(idx);
        if (!c.isActiva()) continue;

        float dx = c.getX() - x;
        float dy = c.getY() - y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist <= RANGO_DETECCION_COMIDA && dist < minDist) {
            minDist = dist;
            idxCercano = idx;
        }
    }

    if (idxCercano != -1) {
        Comida& masCercana = comidas.obtenerRef(idxCercano);
        setTarget(masCercana.getX(), masCercana.getY());
        siguiendoComida = true;

        if (minDist <= 15.0f) {
            cambiarVida(static_cast<float>(masCercana.getPuntosDeVida()));
            masCercana.setActiva(false);
            siguiendoComida = false;
            destinoAleatorio();
        }
    } else {
        if (siguiendoComida) {
            siguiendoComida = false;
            destinoAleatorio();
        }
        pasear();
    }
}

void Bichito::buscarPareja(ArregloDinamico<Bichito>& poblacion,
                        Diccionario<int>& indices, float dt) {
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

        if (minDist <= 35.0f) {
            tiempoCercaPareja += dt;

            // Al cumplir 2 segundos juntos, se reproducen
            if (tiempoCercaPareja >= 2.0f) {
                reproducirseCon(pareja, poblacion, indices);
            }
        } else {
            tiempoCercaPareja = 0.0f; // Se alejaron, se reinicia la cuenta
        }
    } else {
        tiempoCercaPareja = 0.0f;
    }
}

void Bichito::reproducirseCon(Bichito& pareja, ArregloDinamico<Bichito>& poblacion,
                            Diccionario<int>& indices) {
    float hijoX = (this->x + pareja.getX()) / 2.0f;
    float hijoY = (this->y + pareja.getY()) / 2.0f;

    int nuevoId = 1;
    while (indices.contiene(nuevoId))
    {
        ++nuevoId;
    }
    
    //  NUEVO CÓDIGO GENÉTICO: Herencia al reproducirse
    // (Por ahora heredamos genes básicos, luego perfeccionamos la mutación)
    Genoma genHijo((this->genetica.velocidad + pareja.getGenetica().velocidad) / 2.0f, 
                    this->genetica.genEstatico1, pareja.getGenetica().genEstatico2);

    Bichito hijo(nuevoId, 40.0f, hijoX, hijoY, genHijo);
    

    const float promedioEdadMax = (this->edadMaxima + pareja.getEdadMaxima()) * 0.5f;
    const float factorMutacion = 0.85f + 0.30f * (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX));
    hijo.setEdadActual(0.0f);
    hijo.setEdadMaxima(promedioEdadMax * factorMutacion);
    hijo.setInmortal(this->inmortal || pareja.esInmortal());
    hijo.setTarget(hijoX + (rand() % 100 - 50), hijoY + (rand() % 100 - 50));

    this->cambiarVida(-30.0f);
    pareja.cambiarVida(-30.0f);

    this->tiempoCercaPareja = 0.0f;
    pareja.setTiempoCercaPareja(0.0f);

    this->cooldownReproduccion = 10.0f; 
    pareja.setCooldownReproduccion(10.0f);

    poblacion.generar(hijo);
    indices.insertar(nuevoId, poblacion.longitud() - 1);
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

void Bichito::actualizarSoloPaseo(float dt)
{
    pasear();
    moverHaciaObjetivo(dt);
}

void Bichito::actualizar(float dt, ArregloDinamico<Comida>& comidas, Diccionario<int>& indicesComida,
                        const RTree& rtreeComida, ArregloDinamico<Bichito>& poblacion, Diccionario<int>& indices) {
    if (condenado) {
        actualizarSoloPaseo(dt);
        return;
    }

    // NUEVO: Desgaste metabolico de acuerdo a la velocidad 
    // En vez de bajar vida de golpe cada cierto tiempo, lo hace fluido
    vida -= (genetica.metabolismo * dt * 0.05f); // 0.05f es un factor para que no mueran en un segundo
    
    // Validar inmortalidad
    if (vida <= 0.0f && genetica.genEstatico1 == GenEstatico::INMORTAL) {
        vida = 1.0f; // Los inmortales no mueren por metabolismo
    }


    evaluarEstado();

    switch (estado) {
        case EstadoBichito::BuscandoComida:
            buscarComida(comidas, indicesComida, rtreeComida);
            break;
        case EstadoBichito::BuscandoPareja:
            buscarPareja(poblacion, indices, dt);
            break;
        case EstadoBichito::DePaseo:
            pasear();
            break;
        // --- NUEVO CÓDIGO GENÉTICO: ESTADO CAZANDO ---
        case EstadoBichito::Cazando:
            buscarPresa(poblacion, indices);
            break;

    }

    moverHaciaObjetivo(dt);
}

// NUEVO: Implementacion del canibalismo
void Bichito::buscarPresa(ArregloDinamico<Bichito>& poblacion, Diccionario<int>& indices) {
    if (poblacion.longitud() <= 1) {
        pasear(); // Si está solo en el mundo, mejor pasea
        return;
    }

    float distanciaMinima = 99999.0f;
    int indexPresa = -1;

    // 1. Busca al bichito vivo más cercano (Sin importar parentesco, según diseño)
    for (int i = 0; i < poblacion.longitud(); ++i) {
        Bichito& posiblePresa = poblacion.obtenerRef(i);
        
        if (posiblePresa.getId() == this->id || posiblePresa.getVida() <= 0.0f) continue;

        float dx = posiblePresa.getX() - this->x;
        float dy = posiblePresa.getY() - this->y;
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist < distanciaMinima) {
            distanciaMinima = dist;
            indexPresa = i;
        }
    }

    // 2. Persigue o ataca a la presa
    if (indexPresa != -1) {
        Bichito& presa = poblacion.obtenerRef(indexPresa);
        setTarget(presa.getX(), presa.getY()); // Lo sigue

        if (distanciaMinima < 15.0f) { // Colisión para atacar
            bool presaInmune = (presa.getGenetica().genEstatico1 == GenEstatico::INMUNIDAD || 
                                presa.getGenetica().genEstatico2 == GenEstatico::INMUNIDAD);
            
            if (!presaInmune) {
                // Drena la vida y mata a la presa
                this->cambiarVida(presa.getVida());
                presa.setVida(0); 
            } else {
                // Si la presa es inmune, el bicho huye o rebota un poco
                setTarget(x + (rand() % 100 - 50), y + (rand() % 100 - 50));
            }
        }
    } else {
        pasear();
    }
}
