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
        return sf::Color(150, 110, 70);                     // cafe
    }
}

// CAMBIO: ya no se crea la ventana aquí (ni setFramerateLimit): lo hace Aplicacion.
MotorGrafico::MotorGrafico(PilaDinamica<Bichito>& _poblacion, sf::Vector2f _tamanoMundo)
    : tamanoMundo(_tamanoMundo),
    poblacion(_poblacion),
    tiempoCambioEstacion(10.0f),
    temporizadorEstacion(0.0f),
    temporizadorComida(0.0f),
    intervaloComida(2.0f),
    modoActual(ModoSimulacion::Normal),
    pausado(true),
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

    Estacion estacionActual = estaciones.obtener_actual();
    // if ((estacionActual.nombre == "Otono" || estacionActual.nombre == "Invierno") && prioridad == 1)
    // {
    //     prioridad = 2;
    // }

    Comida c(idComida++, prioridad, puntosVida, x, y);
    comidasActivas.generar(c); // Solo va a la lista activa (antes push_back)
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

void MotorGrafico::cargarSimulacionGeneral()
{
    modoActual = ModoSimulacion::Normal;
    pausado = true; // Empieza pausado
    comidasActivas.vaciar();

    // Limpiar población anterior
    while (poblacion.longitud() > 0) {
        poblacion.morir(0);
    }

    // Llenar el pool inicial en la cola
    inicializarPoolComida();

    // Generar la cantidad de bichitos elegida en Opciones en posiciones aleatorias
    for (int i = 0; i < agentesCompetencia; ++i) {
        agregarBichito();
    }

    intervaloComida = calcularIntervaloComida(estaciones.obtener_actual().nombre);
    temporizadorComida = 0.0f;
}



void MotorGrafico::cargarEscenarioCompetencia(int nAgentes)
{
    if (nAgentes < 0) nAgentes = agentesCompetencia; // NUEVO: valor por defecto configurable
    if (nAgentes < 1) nAgentes = 1;

    modoActual = ModoSimulacion::EscenarioCompetencia;
    comidasActivas.vaciar(); // antes clear()

    // Limpiar población reiniciando índices (eliminando iterativamente)
    while (poblacion.longitud() > 0) {
        poblacion.morir(0);
    }

    // Ubicar 1 unidad de comida en el centro
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
    comidasActivas.vaciar(); // antes clear()

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
        b.cambiarVida(-(estacionActual.desgasteVida * 0.5f * dt));
        
        if (b.getCooldownReproduccion() > 0.0f) {
            b.setCooldownReproduccion(b.getCooldownReproduccion() - dt);
        }
    
        // Comprobar si el bichito ha muerto
        if (b.getVida() <= 0.0f) {
            // 1. Guardamos la posición exacta antes de eliminarlo
            float xMuerte = b.getX();
            float yMuerte = b.getY();
        
            // 2. Generamos una nueva comida de Prioridad 2 en esa ubicación
            //    (puedes ajustar los puntos de vida que da esa comida, por defecto 30 o el que tenga configurado)
            agregarComida(xMuerte, yMuerte, 2, 30);
        
            // 3. Eliminamos el bichito de la población
            poblacion.morir(i);
        }
    }
}

void MotorGrafico::procesarEscenarioCompetencia(float dt)
{
    if (comidasActivas.longitud() == 0) return; // antes empty()

    Comida& comidaUnica = comidasActivas.obtenerRef(0); // antes comidasActivas[0]
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
            b.cambiarVida(static_cast<float>(comidaUnica.getPuntosDeVida())); // CAMBIO: cambiarVida
            std::cout << "¡El Bichito ID " << b.getId() << " ha ganado la comida y aumentado su vida!\n";
        }
    }

    // Eliminar comida consumida
    if (!comidaUnica.isActiva()) {
        comidasActivas.vaciar(); // antes clear()
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
    temporizadorComida += dt;

    // Extraer del pool según el temporizador e intervalo con jitter
    if (temporizadorComida >= intervaloComida && !colaComida.esvacia())
    {
        Comida c = extraerComidaSegunEstacion();
        if (c.getId() != -1)
        {
            // Asignar posición aleatoria en el mapa justo al desencolar
            float margen = 30.0f;
            float minX = margen;
            float maxX = Config::ANCHO_MUNDO - margen;
            float minY = margen;
            float maxY = Config::ALTO_MUNDO - margen;

            float rangoX = maxX - minX;
            float rangoY = maxY - minY;

            float rx = minX + (static_cast<float>(rand()) / RAND_MAX) * rangoX;
            float ry = minY + (static_cast<float>(rand()) / RAND_MAX) * rangoY;
            c.setPosicion(rx, ry);

            comidasActivas.generar(c);
        }

        // Recalcular intervalo para el próximo drop
        intervaloComida = calcularIntervaloComida(estaciones.obtener_actual().nombre);
        temporizadorComida = 0.0f;
    }

    // Actualización física de bichitos
    for (int i = 0; i < poblacion.longitud(); ++i) {
        poblacion.obtenerRef(i).actualizar(dt, comidasActivas, poblacion);
    }
    
    // Eliminar comidas consumidas. Si provienen del POOL, se re-encolan (Ciclo Cerrado)
    for (int i = comidasActivas.longitud() - 1; i >= 0; --i) {
        Comida& c = comidasActivas.obtenerRef(i);
        if (!c.isActiva()) {
            if (c.getOrigen() == OrigenComida::Pool) {
                Comida reciclada = c;
                reciclada.setActiva(true);
                colaComida.insertar(reciclada); // Vuelve al final de la cola correspondiente
            }
            comidasActivas.morir(i);
        }
    }
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
    for (int i = 0; i < comidasActivas.longitud(); ++i) {
        const Comida& c = comidasActivas.obtenerRef(i);
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
    
    /*
 * @brief Extrae una comida de la cola procesando la restricción de estación activa.
 * @details Si la estación actual en la ColaCircular es Primavera o Verano, extrae
 *          usando 'extraer_segun_probabilidad()' (distribución entre las 3 prioridades).
 *          Si es Otoño e Invierno, utiliza 'extraer_sin_prioridad_alta()' para simular
 *          escasez omitiendo la comida de Prioridad 1.
 * @return Comida Objeto extraído de la cola correspondiente, o Comida(-1, -1, -1) si no hay disponibilidad.
 */
Comida MotorGrafico::extraerComidaSegunEstacion()
{
    Estacion estacionActual = estaciones.obtener_actual();

    if (estacionActual.nombre == "Primavera" || estacionActual.nombre == "Verano")
    {
        return colaComida.extraer_segun_probabilidad();
    }
    else
    {
        return colaComida.extraer_sin_prioridad_alta();
    }
}

void MotorGrafico::inicializarPoolComida()
{
    colaComida.vaciar();
    static int idPool = 1;

    // P1 (Rara): 5 ítems, 50 HP
    for (int i = 0; i < 5; ++i)
        colaComida.insertar(Comida(idPool++, 1, 50, 0, 0, OrigenComida::Pool));

    // P2 (Media): 10 ítems, 30 HP
    for (int i = 0; i < 10; ++i)
        colaComida.insertar(Comida(idPool++, 2, 30, 0, 0, OrigenComida::Pool));

    // P3 (Común): 15 ítems, 15 HP
    for (int i = 0; i < 15; ++i)
        colaComida.insertar(Comida(idPool++, 3, 15, 0, 0, OrigenComida::Pool));
}

float MotorGrafico::calcularIntervaloComida(const std::string& nombreEstacion)
{
    float base = 2.0f;
    if (nombreEstacion == "Primavera") base = 1.2f;
    else if (nombreEstacion == "Verano") base = 2.0f;
    else if (nombreEstacion == "Otono") base = 3.5f;
    else if (nombreEstacion == "Invierno") base = 5.5f;

    // Frecuencia con ligera variación aleatoria (jitter entre 0.7x y 1.3x)
    float factor = 0.7f + 0.6f * (static_cast<float>(std::rand()) / RAND_MAX);
    return base * factor;
}

