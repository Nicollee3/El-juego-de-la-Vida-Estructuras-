#include "MotorGrafico.h"
#include "gui/Config.h"
#include "gui/GestorRecursos.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>

namespace
{
    // NUEVO (corrige un error): Bichito::setVida RECHAZA valores fuera de 0..200 y no hace nada.
    // Antes, cuando la vida iba a quedar negativa, la resta se ignoraba y el bichito nunca moría.
    // Esta función limita el resultado al rango válido antes de llamar a setVida.
    void cambiarVida(Bichito& b, float delta)
    {
        float nueva = b.getVida() + delta;
        if (nueva < 0.0f) nueva = 0.0f;
        if (nueva > 200.0f) nueva = 200.0f;
        b.setVida(nueva);
    }

    std::string enMinusculas(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return s;
    }

    // Colores de reemplazo (solo se ven mientras no existan tus PNG).
    sf::Color colorFondoEstacion(const std::string& estacion)
    {
        if (estacion == "primavera") return sf::Color(34, 60, 40);
        if (estacion == "verano")    return sf::Color(60, 58, 30);
        if (estacion == "otono")     return sf::Color(66, 44, 28);
        if (estacion == "invierno")  return sf::Color(36, 46, 66);
        return sf::Color(30, 30, 35);
    }

    sf::Color colorComida(int prioridad)
    {
        if (prioridad == 1) return sf::Color(255, 215, 0);  // dorada
        if (prioridad == 2) return sf::Color(90, 200, 90);  // verde
        return sf::Color(150, 110, 70);                      // marrón
    }
}

// CAMBIO: ya no se crea la ventana aquí (ni setFramerateLimit): lo hace Aplicacion.
MotorGrafico::MotorGrafico(PilaDinamica& _poblacion, sf::Vector2f _tamanoMundo)
    : tamanoMundo(_tamanoMundo),
      poblacion(_poblacion),
      tiempoCambioEstacion(10.0f),
      temporizadorEstacion(0.0f),
      modoActual(ModoSimulacion::Normal),
      pausado(false),
      velocidadSim(1.0f),
      prioridadComida(1),
      agentesCompetencia(5)
{
}

void MotorGrafico::setVelocidad(float v)
{
    // Máximo 4x: a 120 px/s el bichito avanza 8 px por frame, menos que el radio de captura (15 px).
    // Con más velocidad podría "saltarse" la comida sin tocarla.
    velocidadSim = std::max(0.25f, std::min(v, 4.0f));
}

void MotorGrafico::setPrioridadComida(int p)
{
    if (p >= 1 && p <= 3) prioridadComida = p;
}

void MotorGrafico::agregarComida(float x, float y, int prioridad, int puntosVida)
{
    static int idComida = 1;
    Comida c(idComida++, prioridad, puntosVida, x, y);
    comidasActivas.push_back(c);
    colaComida.insertar(c);
}

void MotorGrafico::agregarBichito()
{
    static int idNuevo = 1000;
    const int rangoX = std::max(1, static_cast<int>(tamanoMundo.x) - 80);
    const int rangoY = std::max(1, static_cast<int>(tamanoMundo.y) - 80);
    const float x = 40.0f + static_cast<float>(std::rand() % rangoX);
    const float y = 40.0f + static_cast<float>(std::rand() % rangoY);

    Bichito b(idNuevo++, 40.0f, x, y);
    poblacion.generar(b);
}

void MotorGrafico::quitarBichito()
{
    if (poblacion.longitud() > 0)
        poblacion.borrar();
}

void MotorGrafico::cargarEscenarioCompetencia(int nAgentes)
{
    if (nAgentes < 0) nAgentes = agentesCompetencia; // NUEVO: valor por defecto configurable
    if (nAgentes < 1) nAgentes = 1;

    modoActual = ModoSimulacion::EscenarioCompetencia;
    comidasActivas.clear();

    // Limpiar población reiniciando índices (eliminando iterativamente)
    while (poblacion.longitud() > 0) {
        poblacion.morir(0);
    }

    // Ubicar 1 unidad de comida en el centro
    // CAMBIO: antes ventana.getSize(); ahora el tamaño del mundo (sin contar la barra lateral).
    float centroX = tamanoMundo.x / 2.0f;
    float centroY = tamanoMundo.y / 2.0f;
    agregarComida(centroX, centroY, 1, 50);

    // Generar N agentes alrededor en círculo dirigiéndose al centro
    float radioInicial = 300.0f;
    for (int i = 0; i < nAgentes; ++i) {
        float angulo = (2.0f * 3.14159f / nAgentes) * i;
        float bx = centroX + radioInicial * std::cos(angulo);
        float by = centroY + radioInicial * std::sin(angulo);

        Bichito b(i + 1, 20.0f, bx, by); // Vida inicial baja (20 pts)
        b.setTarget(centroX, centroY);
        b.setEstado(EstadoBichito::BuscandoComida);
        poblacion.generar(b);
    }
}

void MotorGrafico::cargarEscenarioReproduccion()
{
    modoActual = ModoSimulacion::EscenarioReproduccion;
    comidasActivas.clear();

    while (poblacion.longitud() > 0) {
        poblacion.morir(0);
    }

    // Crear dos agentes ubicados a corta distancia el uno del otro
    // CAMBIO: tamanoMundo en vez de ventana.getSize()
    float centroX = tamanoMundo.x / 2.0f;
    float centroY = tamanoMundo.y / 2.0f;

    Bichito b1(1, 50.0f, centroX - 80.0f, centroY);
    Bichito b2(2, 50.0f, centroX + 80.0f, centroY);

    b1.setTarget(centroX - 10.0f, centroY);
    b2.setTarget(centroX + 10.0f, centroY);

    b1.setEstado(EstadoBichito::BuscandoPareja);
    b2.setEstado(EstadoBichito::BuscandoPareja);

    poblacion.generar(b1);
    poblacion.generar(b2);
}

// NUEVO: reemplaza la parte de eventos que estaba dentro de ejecutar().
// Ya NO maneja el cierre de la ventana (eso lo hace Aplicacion).
void MotorGrafico::procesarEvento(const sf::Event& evento)
{
    // Teclas para cambiar escenarios de prueba
    if (evento.type == sf::Event::KeyPressed) {
        if (evento.key.code == sf::Keyboard::Num1) {
            cargarEscenarioCompetencia();
        } else if (evento.key.code == sf::Keyboard::Num2) {
            cargarEscenarioReproduccion();
        } else if (evento.key.code == sf::Keyboard::R) {
            modoActual = ModoSimulacion::Normal;
        }
    }

    // Click para poner comida manualmente en el mapa.
    // CAMBIO: solo si el clic cae dentro del mundo (no sobre la barra lateral)
    // y con la prioridad elegida en la barra.
    if (evento.type == sf::Event::MouseButtonPressed &&
        evento.mouseButton.button == sf::Mouse::Left &&
        static_cast<float>(evento.mouseButton.x) < tamanoMundo.x &&
        static_cast<float>(evento.mouseButton.y) < tamanoMundo.y) {
        agregarComida(static_cast<float>(evento.mouseButton.x),
                      static_cast<float>(evento.mouseButton.y), prioridadComida);
    }
}

// NUEVO: avanza la simulación, respetando pausa y velocidad.
void MotorGrafico::actualizar(float dt)
{
    if (pausado) return;
    actualizarFisica(dt * velocidadSim);
}

void MotorGrafico::actualizarFisica(float dt)
{
    // 1. Control del ciclo de estaciones
    temporizadorEstacion += dt;
    if (temporizadorEstacion >= tiempoCambioEstacion) {
        estaciones.siguiente_estación();
        temporizadorEstacion = 0.0f;
    }

    Estacion estacionActual = estaciones.obtener_actual();

    // 2. Procesamiento según el modo de simulación
    if (modoActual == ModoSimulacion::EscenarioCompetencia) {
        procesarEscenarioCompetencia(dt);
    } else if (modoActual == ModoSimulacion::EscenarioReproduccion) {
        procesarEscenarioReproduccion(dt);
    } else {
        procesarSimulacionGeneral(dt);
    }

    // 3. Desgaste de vida por la estación actual y muerte
    for (int i = poblacion.longitud() - 1; i >= 0; --i) {
        Bichito& b = poblacion.obtenerRef(i);
        // CAMBIO: cambiarVida() en lugar de setVida(...) para que la vida llegue a 0 y el bichito muera.
        cambiarVida(b, -(estacionActual.desgasteVida * 0.5f * dt));

        if (b.getCooldownReproduccion() > 0.0f) {
            b.setCooldownReproduccion(b.getCooldownReproduccion() - dt);
        }

        if (b.getVida() <= 0.0f) {
            poblacion.morir(i);
        }
    }
}

void MotorGrafico::procesarEscenarioCompetencia(float dt)
{
    if (comidasActivas.empty()) return;

    Comida& comidaUnica = comidasActivas[0];
    float radioColisionComida = 15.0f;

    for (int i = 0; i < poblacion.longitud(); ++i) {
        Bichito& b = poblacion.obtenerRef(i);
        b.setTarget(comidaUnica.getX(), comidaUnica.getY());
        b.moverHaciaObjetivo(dt);

        float dx = comidaUnica.getX() - b.getX();
        float dy = comidaUnica.getY() - b.getY();
        float dist = std::sqrt(dx * dx + dy * dy);

        // El primer agente en llegar consume la comida y extiende su vida
        if (dist <= radioColisionComida && comidaUnica.isActiva()) {
            comidaUnica.setActiva(false);
            cambiarVida(b, static_cast<float>(comidaUnica.getPuntosDeVida())); // CAMBIO: cambiarVida
            std::cout << "¡El Bichito ID " << b.getId() << " ha ganado la comida y aumentado su vida!\n";
        }
    }

    // Eliminar comida consumida
    if (!comidaUnica.isActiva()) {
        comidasActivas.clear();
    }
}

void MotorGrafico::procesarEscenarioReproduccion(float dt)
{
    float radioProximidad = 35.0f;
    float tiempoRequerido = 2.0f; // Tiempo cerca en segundos para reproducirse

    for (int i = 0; i < poblacion.longitud(); ++i) {
        Bichito& b = poblacion.obtenerRef(i);
        b.moverHaciaObjetivo(dt);
    }

    if (poblacion.longitud() >= 2) {
        Bichito& b1 = poblacion.obtenerRef(0);
        Bichito& b2 = poblacion.obtenerRef(1);

        float dx = b1.getX() - b2.getX();
        float dy = b1.getY() - b2.getY();
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist <= radioProximidad) {
            b1.setTiempoCercaPareja(b1.getTiempoCercaPareja() + dt);
            b2.setTiempoCercaPareja(b2.getTiempoCercaPareja() + dt);

            if (b1.getTiempoCercaPareja() >= tiempoRequerido) {
                // Generar nuevo bichito hijo en el punto medio
                float hijoX = (b1.getX() + b2.getX()) / 2.0f;
                float hijoY = (b1.getY() + b2.getY()) / 2.0f;

                int nuevoId = poblacion.longitud() + 100;
                Bichito hijo(nuevoId, 40.0f, hijoX, hijoY);
                hijo.setTarget(hijoX + (rand() % 100 - 50), hijoY + (rand() % 100 - 50));
                poblacion.generar(hijo);

                std::cout << "¡Nacio un nuevo Bichito (ID: " << nuevoId << ") por reproduccion!\n";

                // OJO: poblacion.generar() puede mover el arreglo en memoria, y b1/b2 serían
                // referencias inválidas. Las volvemos a pedir antes de seguir usándolas.
                Bichito& p1 = poblacion.obtenerRef(0);
                Bichito& p2 = poblacion.obtenerRef(1);

                // Reiniciar contadores
                p1.setTiempoCercaPareja(0.0f);
                p2.setTiempoCercaPareja(0.0f);
                p1.setTarget(p1.getX() - 100.0f, p1.getY());
                p2.setTarget(p2.getX() + 100.0f, p2.getY());
            }
        } else {
            b1.setTiempoCercaPareja(0.0f);
            b2.setTiempoCercaPareja(0.0f);
        }
    }
}

void MotorGrafico::procesarSimulacionGeneral(float dt)
{
    for (int i = 0; i < poblacion.longitud(); ++i) {
        Bichito& b = poblacion.obtenerRef(i);

        // Si no tiene objetivo cercano o llegó a él, buscar comida más cercana
        if (!comidasActivas.empty()) {
            int idxCercano = -1;
            float minDist = 999999.0f;

            for (size_t j = 0; j < comidasActivas.size(); ++j) {
                if (!comidasActivas[j].isActiva()) continue;
                float dx = comidasActivas[j].getX() - b.getX();
                float dy = comidasActivas[j].getY() - b.getY();
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist < minDist) {
                    minDist = dist;
                    idxCercano = static_cast<int>(j);
                }
            }

            if (idxCercano != -1) {
                b.setTarget(comidasActivas[idxCercano].getX(), comidasActivas[idxCercano].getY());

                // Comer si está al alcance
                if (minDist <= 15.0f) {
                    cambiarVida(b, static_cast<float>(comidasActivas[idxCercano].getPuntosDeVida())); // CAMBIO
                    comidasActivas[idxCercano].setActiva(false);
                }
            }
        }

        b.moverHaciaObjetivo(dt);
    }

    // Limpiar comidas consumidas
    std::vector<Comida> temporales;
    for (const auto& c : comidasActivas) {
        if (c.isActiva()) temporales.push_back(c);
    }
    comidasActivas = temporales;
}

// CAMBIO: antes se llamaba renderizar() y dibujaba en su propia ventana (clear + display).
// Ahora recibe dónde dibujar (destino) y el gestor de texturas. La Aplicacion hace clear/display.
void MotorGrafico::dibujar(sf::RenderTarget& destino, GestorRecursos& recursos)
{
    // 1) Fondo: cambia con la estación. Archivo: assets/textures/fondos/fondo_<estacion>.png
    const std::string estacion = enMinusculas(estaciones.obtener_actual().nombre);
    recursos.dibujarFondo(destino, "fondos/fondo_" + estacion, sf::Vector2u(250, 180),
                          colorFondoEstacion(estacion), sf::Vector2f(0.0f, 0.0f), tamanoMundo);

    // 2) Comida: antes círculos verdes; ahora un sprite por prioridad (entidades/comida_1, _2, _3).
    for (const auto& c : comidasActivas) {
        if (!c.isActiva()) continue;

        const sf::Texture& tex = recursos.textura("entidades/comida_" + std::to_string(c.getPrioridad()),
                                                  sf::Vector2u(8, 8), colorComida(c.getPrioridad()),
                                                  Forma::Circulo);
        sf::Sprite sprite(tex);
        sprite.setOrigin(tex.getSize().x / 2.0f, tex.getSize().y / 2.0f); // origen al centro
        sprite.setScale(Config::ESCALA_COMIDA, Config::ESCALA_COMIDA);
        sprite.setPosition(std::round(c.getX()), std::round(c.getY()));
        destino.draw(sprite);
    }

    // 3) Bichitos: antes círculos azules; ahora el sprite entidades/bichito.
    for (int i = 0; i < poblacion.longitud(); ++i) {
        const Bichito& b = poblacion.obtenerRef(i);

        const sf::Texture& tex = recursos.textura("entidades/bichito", sf::Vector2u(16, 16),
                                                  sf::Color(50, 180, 255), Forma::Circulo);
        sf::Sprite sprite(tex);
        sprite.setOrigin(tex.getSize().x / 2.0f, tex.getSize().y / 2.0f);

        
        float diametro = 20.0f; // Tamaño base del bichito

        if (Config::BICHITO_CRECE)
            diametro = 2.0f * (10.0f + b.getVida() / 10.0f); // Mientras más vida, más grande (radio = 10 + vida/10).

        float escala = diametro / static_cast<float>(tex.getSize().x);
        sprite.setScale(escala, escala);

        sprite.setPosition(std::round(b.getX()), std::round(b.getY()));
        destino.draw(sprite);
    }
}
