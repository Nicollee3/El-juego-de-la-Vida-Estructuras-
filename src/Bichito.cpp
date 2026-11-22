#include "Bichito.h"
#include "ArregloDinamico.h"
#include "Diccionario.h"
#include "RTree.h"
#include "gui/Config.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace
{
    constexpr float RANGO_DETECCION_COMIDA = 400.0f;

    float uniforme()
    {
        return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    }

    float normal()
    {
        float suma = 0.0f;
        for (int i = 0; i < 12; ++i)
            suma += uniforme();
        return suma - 6.0f;
    }

    bool tieneGenoma(const Genoma& g, GenEstatico gen)
    {
        return g.genEstatico1 == gen || g.genEstatico2 == gen;
    }

    float mutar(float valor, float desviacion, float minimo, float maximo)
    {
        return std::max(minimo, std::min(maximo, valor + normal() * desviacion));
    }

    Genoma mezclarGenomas(const Genoma& a, const Genoma& b, float factorMutacion)
    {
        float valoresA[4] = {a.vidaMax, a.velocidad, a.metabolismo, a.edadMax};
        float valoresB[4] = {b.vidaMax, b.velocidad, b.metabolismo, b.edadMax};
        float resultado[4] = {};
        int orden[4] = {0, 1, 2, 3};
        const bool intercambiarPadres = std::rand() % 2 != 0;
        for (int i = 3; i > 0; --i)
            std::swap(orden[i], orden[std::rand() % (i + 1)]);

        for (int i = 0; i < 4; ++i)
        {
            const int estadistica = orden[i];
            const bool heredaDeA = (i < 2) != intercambiarPadres;
            const float heredado = heredaDeA ? valoresA[estadistica] : valoresB[estadistica];
            const float sigma = estadistica == 2 ? 0.08f : heredado * 0.08f;
            const float minimo = estadistica == 0 ? 50.0f :
                                 estadistica == 1 ? 40.0f :
                                 estadistica == 2 ? 0.1f : 20.0f;
            const float maximo = estadistica == 0 ? 180.0f :
                                 estadistica == 1 ? 240.0f :
                                 estadistica == 2 ? 5.0f : 300.0f;
            resultado[estadistica] = mutar(heredado, sigma * factorMutacion, minimo, maximo);
        }

        const GenEstatico genesA[2] = {a.genEstatico1, a.genEstatico2};
        const GenEstatico genesB[2] = {b.genEstatico1, b.genEstatico2};
        GenEstatico heredados[2];
        for (int i = 0; i < 2; ++i)
        {
            const GenEstatico* conjunto = std::rand() % 2 == 0 ? genesA : genesB;
            heredados[i] = conjunto[std::rand() % 2];
        }
        if (heredados[0] != GenEstatico::NINGUNO && heredados[0] == heredados[1])
            heredados[1] = GenEstatico::NINGUNO;

        return Genoma(GenesVariables(resultado[0], resultado[1], resultado[2], resultado[3]),
                      heredados[0], heredados[1]);
    }
}

Bichito::Bichito()
    : id(0), vida(30.0f), edadActual(0.0f), inmortal(false),
      x(0.0f), y(0.0f), targetX(0.0f), targetY(0.0f),
      velocidad(120.0f), estado(EstadoBichito::BuscandoComida),
      tiempoCercaPareja(0.0f), idParejaObjetivo(-1), cooldownReproduccion(0.0f),
      cooldownTeletransporte(0.0f), destelloTeletransporte(0.0f),
      siguiendoComida(false), condenado(false), generacion(1), idPadre(-1),
      idMadre(-1), nacidoPorMitosis(false), genetica()
{
    setVida(vida);
}

Bichito::Bichito(int _id, float _vida, float _x, float _y, Genoma _genes)
    : id(_id), vida(0.0f), edadActual(0.0f), inmortal(false),
      x(_x), y(_y), targetX(_x), targetY(_y),
      velocidad(_genes.velocidad), estado(EstadoBichito::BuscandoComida),
      tiempoCercaPareja(0.0f), idParejaObjetivo(-1), cooldownReproduccion(0.0f),
      cooldownTeletransporte(0.0f), destelloTeletransporte(0.0f),
      siguiendoComida(false), condenado(false), generacion(1), idPadre(-1),
      idMadre(-1), nacidoPorMitosis(false), genetica(_genes)
{
    inmortal = tieneGenoma(genetica, GenEstatico::INMORTAL);
    velocidad = genetica.velocidad * (inmortal ? 0.60f : 1.0f);
    setVida(_vida);
}

bool Bichito::tieneGen(GenEstatico gen) const
{
    return tieneGenoma(genetica, gen);
}

bool Bichito::agregarGenEstatico(GenEstatico gen)
{
    if (gen == GenEstatico::NINGUNO || tieneGen(gen))
        return false;

    if (genetica.genEstatico1 == GenEstatico::NINGUNO)
        genetica.genEstatico1 = gen;
    else if (genetica.genEstatico2 == GenEstatico::NINGUNO)
        genetica.genEstatico2 = gen;
    else
        return false;

    if (gen == GenEstatico::INMORTAL)
        setInmortal(true);
    return true;
}

void Bichito::setEdadMaxima(float edad)
{
    if (std::isfinite(edad))
        genetica.edadMax = std::max(1.0f, std::min(edad, 300.0f));
}

bool Bichito::setVida(float nuevaVida)
{
    if (!std::isfinite(nuevaVida) || nuevaVida < 0.0f)
        return false;
    vida = std::min(nuevaVida, genetica.vidaMax);
    return true;
}

void Bichito::cambiarVida(float delta)
{
    if (std::isfinite(delta))
        setVida(std::max(0.0f, std::min(genetica.vidaMax, vida + delta)));
}

bool Bichito::condenar()
{
    if (condenado || vida >= 20.0f)
        return false;
    condenado = true;
    estado = EstadoBichito::DePaseo;
    siguiendoComida = false;
    velocidad = genetica.velocidad * 0.5f * (inmortal ? 0.60f : 1.0f);
    destinoAleatorio();
    return true;
}

void Bichito::evaluarEstado()
{
    if (tieneGen(GenEstatico::CANIBALISMO) && vida < genetica.vidaMax * 0.75f)
    {
        estado = EstadoBichito::Cazando;
        siguiendoComida = false;
        velocidad = genetica.velocidad * (inmortal ? 0.60f : 1.0f);
        return;
    }
    if (estado == EstadoBichito::BuscandoComida && vida < genetica.vidaMax * 0.90f)
        return;

    EstadoBichito nuevoEstado = EstadoBichito::BuscandoComida;
    if (vida >= genetica.vidaMax * 0.90f)
        nuevoEstado = EstadoBichito::DePaseo;
    else if (vida >= genetica.vidaMax * 0.70f)
        nuevoEstado = EstadoBichito::BuscandoPareja;
    else if (vida >= genetica.vidaMax * 0.50f)
        nuevoEstado = EstadoBichito::DePaseo;

    if (tieneGen(GenEstatico::MITOSIS) &&
        nuevoEstado == EstadoBichito::BuscandoPareja)
        nuevoEstado = EstadoBichito::DePaseo;

    if (nuevoEstado != estado)
    {
        estado = nuevoEstado;
        velocidad = genetica.velocidad *
                    (estado == EstadoBichito::DePaseo ? 0.5f : 1.0f) *
                    (inmortal ? 0.60f : 1.0f);
        siguiendoComida = false;
        if (estado == EstadoBichito::DePaseo)
            destinoAleatorio();
    }
}

void Bichito::buscarComida(ArregloDinamico<Comida>& comidas,
                           Diccionario<int>& indicesComida, const RTree& rtreeComida)
{
    int idxCercano = -1;
    float minDist = RANGO_DETECCION_COMIDA;
    const std::vector<int> idsCercanos = rtreeComida.buscarCercanas(x, y, RANGO_DETECCION_COMIDA);
    for (int idComida : idsCercanos)
    {
        int idx = -1;
        if (!indicesComida.buscar(idComida, idx))
            continue;
        Comida& comida = comidas.obtenerRef(idx);
        if (!comida.isActiva())
            continue;
        const float dx = comida.getX() - x;
        const float dy = comida.getY() - y;
        const float distancia = std::sqrt(dx * dx + dy * dy);
        if (distancia < minDist)
        {
            minDist = distancia;
            idxCercano = idx;
        }
    }
    if (idxCercano < 0)
    {
        if (siguiendoComida)
        {
            siguiendoComida = false;
            destinoAleatorio();
        }
        pasear();
        return;
    }
    Comida& comida = comidas.obtenerRef(idxCercano);
    setTarget(comida.getX(), comida.getY());
    siguiendoComida = true;
    if (minDist <= 15.0f)
    {
        cambiarVida(static_cast<float>(comida.getPuntosDeVida()));
        comida.setActiva(false);
        siguiendoComida = false;
        destinoAleatorio();
    }
}

bool Bichito::buscarPareja(ArregloDinamico<Bichito>& poblacion,
                           Diccionario<int>& indices, float dt)
{
    int indicePareja = -1;
    float distanciaMinima = 999999.0f;
    for (int i = 0; i < poblacion.longitud(); ++i)
    {
        Bichito& otro = poblacion.obtenerRef(i);
        if (otro.id == id || otro.estado != EstadoBichito::BuscandoPareja ||
            otro.cooldownReproduccion > 0.0f ||
            otro.tieneGen(GenEstatico::MITOSIS))
            continue;
        const float dx = otro.x - x;
        const float dy = otro.y - y;
        const float distancia = std::sqrt(dx * dx + dy * dy);
        if (distancia < distanciaMinima)
        {
            distanciaMinima = distancia;
            indicePareja = i;
        }
    }
    if (indicePareja < 0)
        return false;
    Bichito& pareja = poblacion.obtenerRef(indicePareja);
    setTarget(pareja.x, pareja.y);
    if (distanciaMinima > 35.0f)
    {
        tiempoCercaPareja = 0.0f;
        return false;
    }
    tiempoCercaPareja += dt;
    if (tiempoCercaPareja >= 2.0f)
        return reproducirseCon(pareja, poblacion, indices);
    return false;
}

bool Bichito::reproducirseCon(Bichito& pareja, ArregloDinamico<Bichito>& poblacion,
                              Diccionario<int>& indices)
{
    const int padreId = id;
    const int madreId = pareja.id;
    const int generacionHijo = std::max(generacion, pareja.generacion) + 1;
    const float hijoX = (x + pareja.x) * 0.5f;
    const float hijoY = (y + pareja.y) * 0.5f;
    int nuevoId = 1;
    while (indices.contiene(nuevoId))
        ++nuevoId;

    Genoma genHijo = mezclarGenomas(genetica, pareja.genetica, 1.0f);
    Bichito hijo(nuevoId, 40.0f, hijoX, hijoY, genHijo);
    hijo.setGeneracion(generacionHijo);
    hijo.setPadres(padreId, madreId, false);
    hijo.setTarget(hijoX + (std::rand() % 100 - 50), hijoY + (std::rand() % 100 - 50));
    cambiarVida(-30.0f);
    pareja.cambiarVida(-30.0f);
    tiempoCercaPareja = 0.0f;
    pareja.tiempoCercaPareja = 0.0f;
    cooldownReproduccion = 10.0f;
    pareja.cooldownReproduccion = 10.0f;
    poblacion.generar(hijo);
    indices.insertar(nuevoId, poblacion.longitud() - 1);
    return true;
}

void Bichito::pasear()
{
    const float dx = targetX - x;
    const float dy = targetY - y;
    if (std::sqrt(dx * dx + dy * dy) <= 10.0f)
        destinoAleatorio();
}

void Bichito::destinoAleatorio()
{
    const float margen = 30.0f;
    targetX = margen + uniforme() * (Config::ANCHO_MUNDO - 2.0f * margen);
    targetY = margen + uniforme() * (Config::ALTO_MUNDO - 2.0f * margen);
}

void Bichito::moverHaciaObjetivo(float dt)
{
    const float dx = targetX - x;
    const float dy = targetY - y;
    const float distancia = std::sqrt(dx * dx + dy * dy);
    if (tieneGen(GenEstatico::TELETRANSPORTARSE) &&
        cooldownTeletransporte <= 0.0f && distancia > 180.0f &&
        estado != EstadoBichito::DePaseo && std::rand() % 180 == 0)
    {
        const float paso = std::min(150.0f, distancia - 25.0f);
        x += dx / distancia * paso;
        y += dy / distancia * paso;
        cambiarVida(-8.0f);
        cooldownTeletransporte = 3.0f;
        destelloTeletransporte = 0.18f;
        return;
    }
    if (distancia > 2.0f)
    {
        x += dx / distancia * velocidad * dt;
        y += dy / distancia * velocidad * dt;
    }
}

void Bichito::actualizarSoloPaseo(float dt)
{
    pasear();
    moverHaciaObjetivo(dt);
}

void Bichito::actualizar(float dt, ArregloDinamico<Comida>& comidas,
                         Diccionario<int>& indicesComida, const RTree& rtreeComida,
                         ArregloDinamico<Bichito>& poblacion, Diccionario<int>& indices,
                         bool verano, bool invierno)
{
    cooldownTeletransporte = std::max(0.0f, cooldownTeletransporte - dt);
    destelloTeletransporte = std::max(0.0f, destelloTeletransporte - dt);
    if (condenado)
    {
        actualizarSoloPaseo(dt);
        return;
    }
    cambiarVida(-genetica.metabolismo * dt * 0.05f);

    if (tieneGen(GenEstatico::VAMPIRISMO))
    {
        if (verano)
            cambiarVida(-1.5f * dt);
        const float roboPorSegundo = invierno ? 8.0f : 4.0f;
        for (int i = 0; i < poblacion.longitud(); ++i)
        {
            Bichito& victima = poblacion.obtenerRef(i);
            if (victima.id == id || victima.vida <= 0.0f ||
                victima.tieneGen(GenEstatico::INMUNIDAD))
                continue;
            const float dx = victima.x - x;
            const float dy = victima.y - y;
            if (dx * dx + dy * dy <= 45.0f * 45.0f)
            {
                const float robo = std::min(roboPorSegundo * dt, victima.vida);
                victima.cambiarVida(-robo);
                cambiarVida(robo);
            }
        }
    }

    if (tieneGen(GenEstatico::MITOSIS) &&
        vida >= genetica.vidaMax * 0.90f && cooldownReproduccion <= 0.0f)
    {
        const int padreId = id;
        const int generacionHijo = generacion + 1;
        const float vidaCompartida = vida * 0.5f;
        const float clonX = std::max(30.0f, std::min(x + 20.0f, Config::ANCHO_MUNDO - 30.0f));
        const float clonY = std::max(30.0f, std::min(y + 20.0f, Config::ALTO_MUNDO - 30.0f));
        GenesVariables variables = genetica.variables();
        variables.vidaMax = mutar(variables.vidaMax, variables.vidaMax * 0.016f, 50.0f, 180.0f);
        variables.velocidad = mutar(variables.velocidad, variables.velocidad * 0.016f, 40.0f, 240.0f);
        variables.metabolismo = mutar(variables.metabolismo, 0.016f, 0.1f, 5.0f);
        variables.edadMax = mutar(variables.edadMax, variables.edadMax * 0.016f, 20.0f, 300.0f);
        Genoma genClon(variables, genetica.genEstatico1, genetica.genEstatico2);
        Bichito clon(1, vidaCompartida, clonX, clonY, genClon);
        int nuevoId = 1;
        while (indices.contiene(nuevoId))
            ++nuevoId;
        clon.setId(nuevoId);
        clon.setGeneracion(generacionHijo);
        clon.setPadres(padreId, -1, true);
        clon.setEdadMaxima(variables.edadMax);
        clon.setTarget(clonX + (std::rand() % 80 - 40), clonY + (std::rand() % 80 - 40));
        setVida(vidaCompartida);
        cooldownReproduccion = 14.0f;
        poblacion.generar(clon);
        indices.insertar(nuevoId, poblacion.longitud() - 1);
        return;
    }

    evaluarEstado();
    switch (estado)
    {
    case EstadoBichito::BuscandoComida:
        buscarComida(comidas, indicesComida, rtreeComida);
        break;
    case EstadoBichito::BuscandoPareja:
        if (buscarPareja(poblacion, indices, dt))
            return;
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

void Bichito::buscarPresa(ArregloDinamico<Bichito>& poblacion, Diccionario<int>&)
{
    if (poblacion.longitud() <= 1)
    {
        pasear();
        return;
    }
    float distanciaMinima = 99999.0f;
    int indicePresa = -1;
    for (int i = 0; i < poblacion.longitud(); ++i)
    {
        Bichito& posible = poblacion.obtenerRef(i);
        if (posible.id == id || posible.vida <= 0.0f)
            continue;
        const float dx = posible.x - x;
        const float dy = posible.y - y;
        const float distancia = std::sqrt(dx * dx + dy * dy);
        if (distancia < distanciaMinima)
        {
            distanciaMinima = distancia;
            indicePresa = i;
        }
    }
    cambiarVida(-4.0f * 0.05f);
    if (indicePresa < 0)
    {
        pasear();
        return;
    }
    Bichito& presa = poblacion.obtenerRef(indicePresa);
    setTarget(presa.x, presa.y);
    if (distanciaMinima <= 15.0f)
    {
        const float vidaPresa = presa.vida;
        presa.setVida(0.0f);
        cambiarVida(-12.0f);
        cambiarVida(vidaPresa * 0.8f);
    }
}
