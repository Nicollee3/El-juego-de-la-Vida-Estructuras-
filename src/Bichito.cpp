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

    // Evalúa si un gen heredado sufre una mutación espontánea según niveles de rareza
    GenEstatico evaluarMutacionGen(GenEstatico genHeredado)
    {
        int roll = std::rand() % 100;

        // 1. Mutaciones Raras (2% de probabilidad total -> 0 y 1)
        // Evita que un gen fuerte se extinga para siempre sin romper el equilibrio
        if (roll < 2) {
            int raro = std::rand() % 4;
            switch (raro) {
                case 0: return GenEstatico::CANIBALISMO;
                case 1: return GenEstatico::VAMPIRISMO;
                case 2: return GenEstatico::TELETRANSPORTARSE;
                case 3: return GenEstatico::INMORTAL;
            }
        }
        // 2. Mutaciones Comunes (15% de probabilidad -> del 2 al 16)
        else if (roll < 17) {
            int comun = std::rand() % 3;
            switch (comun) {
                case 0: return GenEstatico::INMUNIDAD;
                case 1: return GenEstatico::MITOSIS;
                case 2: return GenEstatico::ENFERMEDAD;
            }
        }

        // 3. Sin mutación (83% de probabilidad): conserva el gen de sus padres
        return genHeredado;
    }

    // Función auxiliar para verificar rápidamente si el bichito posee un gen estático en cualquiera de sus 2 ranuras
    bool tieneGen(const Genoma& g, GenEstatico buscado)
    {
        return (g.genEstatico1 == buscado || g.genEstatico2 == buscado);
    }
}

// --- CONSTRUCTORES ---
Bichito::Bichito() 
    : id(0), vida(30.0f), edadActual(0.0f), edadMaxima(generarEdadMaximaAleatoria()), inmortal(false),
      x(0.0f), y(0.0f), targetX(0.0f), targetY(0.0f),
      velocidad(120.0f), estado(EstadoBichito::BuscandoComida),
      tiempoCercaPareja(0.0f), idParejaObjetivo(-1), cooldownReproduccion(0.0f), 
      siguiendoComida(false), condenado(false)
{
    genetica = Genoma(120.0f);
}

Bichito::Bichito(int _id, float _vida, float _x, float _y, Genoma _genes) 
    : id(_id), vida(_vida), edadActual(0.0f), edadMaxima(generarEdadMaximaAleatoria()), inmortal(false),
      x(_x), y(_y), targetX(_x), targetY(_y),
      velocidad(120.0f), estado(EstadoBichito::BuscandoComida),
      tiempoCercaPareja(0.0f), idParejaObjetivo(-1), cooldownReproduccion(0.0f), 
      siguiendoComida(false), condenado(false), 
      genetica(_genes)
{
    // Sincronizamos velocidad base e inmortalidad desde el genoma recibido
    float velInicial = (_genes.velocidad > 0.0f) ? _genes.velocidad : 120.0f;
    setVelocidad(velInicial);
    inmortal = tieneGen(_genes, GenEstatico::INMORTAL);
}

// --- SETTERS CON LÓGICA ---
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
    velocidad = genetica.velocidad * 0.5f; // Baja velocidad sin borrar su genoma
    destinoAleatorio();
    return true;
}

// --- MÉTODOS PRIVADOS ---
void Bichito::evaluarEstado() {
    // 1. INTERCEPCIÓN CANÍBAL: Si tiene hambre y es caníbal, caza en vez de buscar comida normal
    if (vida < 100.0f && tieneGen(genetica, GenEstatico::CANIBALISMO)) {
        if (estado != EstadoBichito::Cazando) {
            estado = EstadoBichito::Cazando;
            velocidad = genetica.velocidad; // Corre a su velocidad genética máxima
            siguiendoComida = false;
        }
        return;
    }

    EstadoBichito nuevoEstado;

    if (estado == EstadoBichito::BuscandoComida && vida < 100.0f) {
        return;
    }

    if (vida > 90.0f) {
        nuevoEstado = EstadoBichito::DePaseo;
    } else if (vida > 70.0f) {
        nuevoEstado = EstadoBichito::BuscandoPareja;
    } else if (vida > 50.0f) {
        nuevoEstado = EstadoBichito::DePaseo;
    } else {
        nuevoEstado = EstadoBichito::BuscandoComida;
    }

    // Al cambiar de estado, ajustamos la velocidad actual proporcionalmente sin sobrescribir genetica.velocidad
    if (nuevoEstado != estado) {
        estado = nuevoEstado;
        velocidad = (estado == EstadoBichito::DePaseo) ? (genetica.velocidad * 0.5f) : genetica.velocidad;
        siguiendoComida = false;
        if (estado == EstadoBichito::DePaseo) {
            destinoAleatorio();
        }
    }
}

void Bichito::buscarComida(ArregloDinamico<Comida>& comidas, Diccionario<int>& indicesComida, const RTree& rtreeComida) {
    int idxCercano = -1;
    float minDist = 999999.0f;

    std::vector<int> idsCercanos = rtreeComida.buscarCercanas(x, y, RANGO_DETECCION_COMIDA);

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

            if (tiempoCercaPareja >= 2.0f) {
                reproducirseCon(pareja, poblacion, indices);
            }
        } else {
            tiempoCercaPareja = 0.0f;
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
    
    // 1. Selección aleatoria de los genes de ambos padres
    GenEstatico base1 = (std::rand() % 2 == 0) ? this->genetica.genEstatico1 : pareja.getGenetica().genEstatico1;
    GenEstatico base2 = (std::rand() % 2 == 0) ? this->genetica.genEstatico2 : pareja.getGenetica().genEstatico2;

    // 2. Aplicar probabilidad de mutación escalonada
    GenEstatico genFinal1 = evaluarMutacionGen(base1);
    GenEstatico genFinal2 = evaluarMutacionGen(base2);

    if (genFinal1 != GenEstatico::NINGUNO && genFinal1 == genFinal2) {
        genFinal2 = base2;
    }

    // 3. Variación gradual en genes variables (Velocidad +/- 10% y Metabolismo acoplado)
    float velPromedio = (this->genetica.velocidad + pareja.getGenetica().velocidad) * 0.5f;
    float variacionVel = 0.90f + 0.20f * (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX));
    float velHijo = std::max(50.0f, std::min(velPromedio * variacionVel, 220.0f));

    Genoma genHijo(velHijo, genFinal1, genFinal2);
    Bichito hijo(nuevoId, 40.0f, hijoX, hijoY, genHijo);

    const float promedioEdadMax = (this->edadMaxima + pareja.getEdadMaxima()) * 0.5f;
    const float factorMutacion = 0.85f + 0.30f * (static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX));
    hijo.setEdadActual(0.0f);
    hijo.setEdadMaxima(promedioEdadMax * factorMutacion);
    
    bool esInmortalHijo = tieneGen(genHijo, GenEstatico::INMORTAL);
    hijo.setInmortal(esInmortalHijo);
    
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

    // --- GEN TELETRANSPORTACIÓN ---
    // Si tiene el gen, está lejos de su objetivo (> 180px) y no está solo paseando,
    // tiene una pequeña probabilidad por fotograma de dar un salto espacial instantáneo.
    if (tieneGen(genetica, GenEstatico::TELETRANSPORTARSE) && 
        estado != EstadoBichito::DePaseo && 
        dist > 180.0f && (std::rand() % 180 == 0)) 
    {
        float offsetX = static_cast<float>((std::rand() % 30) - 15);
        float offsetY = static_cast<float>((std::rand() % 30) - 15);
        x = std::max(30.0f, std::min(targetX + offsetX, Config::ANCHO_MUNDO - 30.0f));
        y = std::max(30.0f, std::min(targetY + offsetY, Config::ALTO_MUNDO - 30.0f));
        return;
    }

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

    // 1. DESGASTE METABÓLICO ACORDE A LA VELOCIDAD HEREDADA
    vida -= (genetica.metabolismo * dt * 0.05f);

    // 2. GEN ENFERMEDAD (Desgaste propio y contagio en área)
    if (tieneGen(genetica, GenEstatico::ENFERMEDAD)) {
        vida -= (1.5f * dt); // La enfermedad debilita lentamente al portador
        for (int i = 0; i < poblacion.longitud(); ++i) {
            Bichito& vecino = poblacion.obtenerRef(i);
            if (vecino.getId() == this->id || vecino.getVida() <= 0.0f) continue;

            float dx = vecino.getX() - x;
            float dy = vecino.getY() - y;
            if ((dx * dx + dy * dy) <= (40.0f * 40.0f)) {
                if (!tieneGen(vecino.getGenetica(), GenEstatico::INMUNIDAD)) {
                    vecino.cambiarVida(-2.5f * dt); // Daña al vecino si no es inmune
                }
            }
        }
    }

    // 3. GEN VAMPIRISMO (Robo pasivo de vida por proximidad)
    if (tieneGen(genetica, GenEstatico::VAMPIRISMO)) {
        for (int i = 0; i < poblacion.longitud(); ++i) {
            Bichito& victima = poblacion.obtenerRef(i);
            if (victima.getId() == this->id || victima.getVida() <= 0.0f) continue;

            float dx = victima.getX() - x;
            float dy = victima.getY() - y;
            if ((dx * dx + dy * dy) <= (45.0f * 45.0f)) {
                if (!tieneGen(victima.getGenetica(), GenEstatico::INMUNIDAD)) {
                    float robo = 4.0f * dt;
                    victima.cambiarVida(-robo);
                    this->cambiarVida(robo);
                }
            }
        }
    }

    // 4. VALIDACIÓN DE INMORTALIDAD (Revisa ambas ranuras genéticas)
    bool esInmortal = tieneGen(genetica, GenEstatico::INMORTAL) || inmortal;
    if (vida <= 0.0f && esInmortal) {
        vida = 1.0f;
    }

    // 5. GEN MITOSIS (Clonación asexual cuando está muy saludable y sin cooldown)
    if (tieneGen(genetica, GenEstatico::MITOSIS) && vida >= 115.0f && cooldownReproduccion <= 0.0f) {
        int nuevoId = 1;
        while (indices.contiene(nuevoId)) {
            ++nuevoId;
        }

        // El clon hereda los genes del progenitor pasando por posible mutación
        GenEstatico g1Clon = evaluarMutacionGen(genetica.genEstatico1);
        GenEstatico g2Clon = evaluarMutacionGen(genetica.genEstatico2);
        if (g1Clon != GenEstatico::NINGUNO && g1Clon == g2Clon) g2Clon = genetica.genEstatico2;

        Genoma genClon(genetica.velocidad, g1Clon, g2Clon);
        float clonX = std::max(30.0f, std::min(x + 20.0f, Config::ANCHO_MUNDO - 30.0f));
        float clonY = std::max(30.0f, std::min(y + 20.0f, Config::ALTO_MUNDO - 30.0f));

        Bichito clon(nuevoId, 45.0f, clonX, clonY, genClon);
        clon.setEdadActual(0.0f);
        clon.setEdadMaxima(this->edadMaxima);
        clon.setInmortal(tieneGen(genClon, GenEstatico::INMORTAL));
        clon.setTarget(clonX + (rand() % 80 - 40), clonY + (rand() % 80 - 40));

        this->cambiarVida(-45.0f);
        this->cooldownReproduccion = 14.0f;

        poblacion.generar(clon);
        indices.insertar(nuevoId, poblacion.longitud() - 1);
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
        case EstadoBichito::Cazando:
            buscarPresa(poblacion, indices);
            break;
    }

    moverHaciaObjetivo(dt);
}

// --- IMPLEMENTACIÓN DEL CANIBALISMO ---
void Bichito::buscarPresa(ArregloDinamico<Bichito>& poblacion, Diccionario<int>& indices) {
    if (poblacion.longitud() <= 1) {
        pasear();
        return;
    }

    float distanciaMinima = 99999.0f;
    int indexPresa = -1;

    // Busca al bichito vivo más cercano sin importar parentesco
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

    if (indexPresa != -1) {
        Bichito& presa = poblacion.obtenerRef(indexPresa);
        setTarget(presa.getX(), presa.getY());

        if (distanciaMinima < 15.0f) {
            bool presaInmune = tieneGen(presa.getGenetica(), GenEstatico::INMUNIDAD);
            
            if (!presaInmune) {
                this->cambiarVida(presa.getVida());
                presa.setVida(0.0f); 
            } else {
                // Si la presa es inmune, el caníbal rebota y cambia de rumbo
                setTarget(x + (rand() % 100 - 50), y + (rand() % 100 - 50));
            }
        }
    } else {
        pasear();
    }
}