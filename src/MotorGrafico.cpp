#include "MotorGrafico.h"
#include <iostream>
#include <cmath>
#include <cstdlib>

MotorGrafico::MotorGrafico(PilaDinamica& _poblacion, unsigned int ancho, unsigned int alto)
    : poblacion(_poblacion),
      ventana(sf::VideoMode(ancho, alto), "Simulador de Vida 2D - Motor Grafico SFML"),
      tiempoCambioEstacion(10.0f),
      temporizadorEstacion(0.0f),
      modoActual(ModoSimulacion::Normal)
{
    ventana.setFramerateLimit(60);
}

void MotorGrafico::agregarComida(float x, float y, int prioridad, int puntosVida)
{
    static int idComida = 1;
    Comida c(idComida++, prioridad, puntosVida, x, y);
    comidasActivas.push_back(c);
    colaComida.insertar(c);
}

void MotorGrafico::cargarEscenarioCompetencia(int nAgentes)
{
    modoActual = ModoSimulacion::EscenarioCompetencia;
    comidasActivas.clear();
    
    // Limpiar población reiniciando índices (eliminando iterativamente)
    while (poblacion.longitud() > 0) {
        poblacion.morir(0);
    }

    // Ubicar 1 unidad de comida en el centro
    float centroX = ventana.getSize().x / 2.0f;
    float centroY = ventana.getSize().y / 2.0f;
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
    float centroX = ventana.getSize().x / 2.0f;
    float centroY = ventana.getSize().y / 2.0f;

    Bichito b1(1, 50.0f, centroX - 80.0f, centroY);
    Bichito b2(2, 50.0f, centroX + 80.0f, centroY);

    b1.setTarget(centroX - 10.0f, centroY);
    b2.setTarget(centroX + 10.0f, centroY);

    b1.setEstado(EstadoBichito::BuscandoPareja);
    b2.setEstado(EstadoBichito::BuscandoPareja);

    poblacion.generar(b1);
    poblacion.generar(b2);
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
        b.setVida(b.getVida() - (estacionActual.desgasteVida * 0.5f * dt));

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
            b.setVida(b.getVida() + comidaUnica.getPuntosDeVida()); // Recibe vida extra
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

                // Reiniciar contadores
                b1.setTiempoCercaPareja(0.0f);
                b2.setTiempoCercaPareja(0.0f);
                b1.setTarget(b1.getX() - 100.0f, b1.getY());
                b2.setTarget(b2.getX() + 100.0f, b2.getY());
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
                    idxCercano = j;
                }
            }

            if (idxCercano != -1) {
                b.setTarget(comidasActivas[idxCercano].getX(), comidasActivas[idxCercano].getY());
                
                // Comer si está al alcance
                if (minDist <= 15.0f) {
                    b.setVida(b.getVida() + comidasActivas[idxCercano].getPuntosDeVida());
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

void MotorGrafico::renderizar()
{
    ventana.clear(sf::Color(30, 30, 35));

    // Renderizar Comida (círculos verdes / amarillos)
    for (const auto& c : comidasActivas) {
        if (!c.isActiva()) continue;
        sf::CircleShape formaComida(6.0f);
        formaComida.setFillColor(sf::Color::Green);
        formaComida.setOrigin(6.0f, 6.0f);
        formaComida.setPosition(c.getX(), c.getY());
        ventana.draw(formaComida);
    }

    // Renderizar Bichitos (círculos azules / cian proporcional a su vida)
    for (int i = 0; i < poblacion.longitud(); ++i) {
        Bichito b = poblacion.obtener(i);
        float radio = 10.0f + (b.getVida() / 10.0f);
        sf::CircleShape formaBichito(radio);
        formaBichito.setFillColor(sf::Color(50, 180, 255));
        formaBichito.setOutlineThickness(2.0f);
        formaBichito.setOutlineColor(sf::Color::White);
        formaBichito.setOrigin(radio, radio);
        formaBichito.setPosition(b.getX(), b.getY());
        ventana.draw(formaBichito);
    }

    ventana.display();
}

void MotorGrafico::ejecutar()
{
    sf::Clock reloj;

    while (ventana.isOpen()) {
        sf::Time lapso = reloj.restart();
        float dt = lapso.asSeconds();

        sf::Event evento;
        while (ventana.pollEvent(evento)) {
            if (evento.type == sf::Event::Closed) {
                ventana.close();
            }

            // Teclas para cambiar escenarios de prueba
            if (evento.type == sf::Event::KeyPressed) {
                if (evento.key.code == sf::Keyboard::Num1) {
                    cargarEscenarioCompetencia(5); // 5 bichitos compitiendo por 1 comida
                } else if (evento.key.code == sf::Keyboard::Num2) {
                    cargarEscenarioReproduccion(); // Escenario de reproducción por proximidad
                } else if (evento.key.code == sf::Keyboard::R) {
                    modoActual = ModoSimulacion::Normal;
                }
            }

            // Click para poner comida manualmente en el mapa
            if (evento.type == sf::Event::MouseButtonPressed) {
                if (evento.mouseButton.button == sf::Mouse::Left) {
                    agregarComida(evento.mouseButton.x, evento.mouseButton.y);
                }
            }
        }

        actualizarFisica(dt);
        renderizar();
    }
}