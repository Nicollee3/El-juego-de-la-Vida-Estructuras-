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

    // --- NUEVO CÓDIGO GENÉTICO: Generación y colores por gen ---
    Genoma generarGenomaInicial()
    {
        // Velocidad aleatoria entre 85 y 155 (acopla automáticamente su metabolismo)
        float vel = 85.0f + static_cast<float>(std::rand() % 71);
        
        // ~45% nacen normales (NINGUNO), ~55% nacen con algún gen estático inicial
        auto sortearGen = []() -> GenEstatico {
            int r = std::rand() % 100;
            if (r < 45) return GenEstatico::NINGUNO;
            return static_cast<GenEstatico>(1 + (std::rand() % 7));
        };

        GenEstatico g1 = sortearGen();
        GenEstatico g2 = sortearGen();
        if (g1 != GenEstatico::NINGUNO && g1 == g2) g2 = GenEstatico::NINGUNO;

        return Genoma(vel, g1, g2);
    }

    sf::Color colorPorGenoma(const Genoma& g)
    {
        auto tiene = [&](GenEstatico buscado) {
            return g.genEstatico1 == buscado || g.genEstatico2 == buscado;
        };

        if (tiene(GenEstatico::CANIBALISMO))       return sf::Color(235, 50, 50);   // Rojo: Caníbal
        if (tiene(GenEstatico::VAMPIRISMO))        return sf::Color(165, 25, 95);   // Vinotinto/Magenta: Vampiro
        if (tiene(GenEstatico::INMORTAL))          return sf::Color(185, 110, 255); // Morado: Inmortal
        if (tiene(GenEstatico::INMUNIDAD))         return sf::Color(255, 215, 40);  // Dorado: Inmune
        if (tiene(GenEstatico::ENFERMEDAD))        return sf::Color(130, 230, 60);  // Verde tóxico: Enfermo
        if (tiene(GenEstatico::TELETRANSPORTARSE)) return sf::Color(50, 240, 220);  // Turquesa: Teletransporte
        if (tiene(GenEstatico::MITOSIS))           return sf::Color(255, 155, 60);  // Naranja: Mitosis
        
        return sf::Color(70, 190, 255); // Azul celeste: Bichito estándar
    }

    std::string nombreGenEstatico(GenEstatico gen)
    {
        switch (gen)
        {
        case GenEstatico::INMUNIDAD:         return "Inmunidad";
        case GenEstatico::CANIBALISMO:       return "Canibalismo";
        case GenEstatico::VAMPIRISMO:        return "Vampirismo";
        case GenEstatico::TELETRANSPORTARSE: return "Teletransporte";
        case GenEstatico::INMORTAL:          return "Inmortal";
        case GenEstatico::ENFERMEDAD:        return "Enfermedad";
        case GenEstatico::MITOSIS:           return "Mitosis";
        case GenEstatico::NINGUNO:
        default:                           return "Ninguno";
        }
    }

    std::string nombreEstadoBichito(EstadoBichito est)
    {
        switch (est)
        {
        case EstadoBichito::BuscandoComida: return "Buscando comida";
        case EstadoBichito::BuscandoPareja: return "Buscando pareja";
        case EstadoBichito::Cazando:        return "Cazando presa";
        case EstadoBichito::DePaseo:
        default:                            return "De paseo";
        }
    }
    // ------------------------------------------------------------

    constexpr int POBLACION_INICIAL_GENERAL = 60;
    constexpr float ANIOS_POR_SEGUNDO = 5.0f; // 20 años -> 4s, 200 años -> 40s, 400 años -> 80s

    sf::String utf8(const std::string& texto)
    {
        return sf::String::fromUtf8(texto.begin(), texto.end());
    }

    sf::Color colorDesastre(TipoDesastre tipo)
    {
        switch (tipo)
        {
        case TipoDesastre::Meteoritos:     return sf::Color(255, 115, 35);  // naranja fuego
        case TipoDesastre::Diluvio:        return sf::Color(70, 175, 255);  // azul tormenta
        case TipoDesastre::BombaNuclear:   return sf::Color(245, 225, 45);  // amarillo radiactivo
        case TipoDesastre::PatriaMilagro:  return sf::Color(255, 205, 30);  // dorado Colombia
        case TipoDesastre::Sobrepoblacion: return sf::Color(225, 50, 70);   // rojo parca
        case TipoDesastre::Ninguno:
        default:                           return sf::Color::White;
        }
    }

    std::string tituloImpactante(TipoDesastre tipo)
    {
        switch (tipo)
        {
        case TipoDesastre::Meteoritos:     return "LLUVIA DE METEORITOS";
        case TipoDesastre::Diluvio:        return "GRAN DILUVIO";
        case TipoDesastre::BombaNuclear:   return "BOMBA NUCLEAR";
        case TipoDesastre::PatriaMilagro:  return "PATRIA MILAGRO";
        case TipoDesastre::Sobrepoblacion: return "SOBREPOBLACIÓN";
        case TipoDesastre::Ninguno:
        default:                           return "";
        }
    }
}

MotorGrafico::MotorGrafico(ArregloDinamico<Bichito>& _poblacion, sf::Vector2f _tamanoMundo)
    : tamanoMundo(_tamanoMundo),
    poblacion(_poblacion),
    tiempoSimulacion(0.0f),
    muertesHambre(0),
    muertesVejez(0),
    bolsaComida{{1, 2, 2, 3, 3, 3, 3, 3, 3, 3}},
    indiceBolsaComida(0),
    tiempoCambioEstacion(10.0f),
    temporizadorEstacion(0.0f),
    temporizadorComida(0.0f),
    intervaloComida(2.0f),
    tiempoAlertaDesastre(0.0f),
    duracionAlertaDesastre(4.2f),
    tiempoVisual(0.0f),
    modoActual(ModoSimulacion::Normal),
    pausado(true),
    velocidadSim(1.0f),
    prioridadComida(1),
    agentesCompetencia(5),
    bichitosIniciales(POBLACION_INICIAL_GENERAL),
    idBichitoSeleccionado(-1)
{
    mezclarBolsaComida();
    inicializarParticulasClima();
}

void MotorGrafico::setVelocidad(float v)
{
    velocidadSim = std::max(0.25f, std::min(v, 4.0f));
}

void MotorGrafico::setPrioridadComida(int p)
{
    if (p >= 1 && p <= 3) prioridadComida = p;
}

void MotorGrafico::agregarComida(float x, float y, int prioridad, int puntosVida,
                                 OrigenComida origen)
{
    static int idComida = 1;

    Comida c(idComida++, prioridad, puntosVida, x, y, origen);
    comidasActivas.generar(c);
    indicesComida.insertar(c.getId(), comidasActivas.longitud() - 1);
}

void MotorGrafico::agregarBichitoAlFinal(Bichito bichito)
{
    if (indicesBichitos.contiene(bichito.getId()))
    {
        int idDisponible = 1;
        while (indicesBichitos.contiene(idDisponible))
        {
            ++idDisponible;
        }
        bichito.setId(idDisponible);
    }

    poblacion.generar(bichito);
    indicesBichitos.insertar(bichito.getId(), poblacion.longitud() - 1);
    registrarEnColaVejez(bichito);
}

void MotorGrafico::eliminarBichitoEn(int indice, bool retirarDeCola)
{
    const int ultimoIndice = poblacion.longitud() - 1;
    const Bichito &eliminado = poblacion.obtenerRef(indice);
    const int idEliminado = eliminado.getId();
    const int idMovido = indice != ultimoIndice ? poblacion.obtenerRef(ultimoIndice).getId() : -1;

    if (retirarDeCola && eliminado.estaCondenado())
    {
        colaEliminacion.eliminar(idEliminado);
    }
    indicesBichitos.eliminar(idEliminado);
    poblacion.morir(indice);
    if (indice != ultimoIndice)
    {
        indicesBichitos.insertar(idMovido, indice);
    }
}

void MotorGrafico::eliminarBichitoPorId(int id)
{
    int indice = -1;
    if (!indicesBichitos.buscar(id, indice))
    {
        return;
    }
    eliminarBichitoEn(indice);
}

ResultadoDesastre MotorGrafico::activarDesastre(TipoDesastre tipo)
{
    ResultadoDesastre res = motorDesastres.ejecutarDesastre(
        tipo, poblacion, [this](int id) { eliminarBichitoPorId(id); }, true);
    if (res.bajasReales > 0)
    {
        mostrarAlertaDesastre(res);
    }
    return res;
}

void MotorGrafico::vaciarPoblacion()
{
    poblacion.vaciar();
    indicesBichitos.vaciar();
    colaEliminacion.vaciar();
    colaVejez.vaciar();
    tiempoSimulacion = 0.0f;
    muertesHambre = 0;
    muertesVejez = 0;
    idBichitoSeleccionado = -1;
}

void MotorGrafico::registrarEnColaVejez(const Bichito& bichito)
{
    if (bichito.esInmortal())
        return;

    const float aniosRestantes = std::max(0.0f, bichito.getEdadMaxima() - bichito.getEdadActual());
    const float segundosRestantes = aniosRestantes / ANIOS_POR_SEGUNDO;
    colaVejez.insertar(bichito.getId(), tiempoSimulacion + segundosRestantes, bichito.getEdadMaxima());
}

void MotorGrafico::procesarMuertesPorHambre()
{
    for (int i = poblacion.longitud() - 1; i >= 0; --i)
    {
        const Bichito& b = poblacion.obtenerRef(i);
        if (b.murioDeHambre())
        {
            ++muertesHambre;
            agregarComida(b.getX(), b.getY(), 2, 30, OrigenComida::Cadaver);
            eliminarBichitoEn(i);
        }
    }
}

void MotorGrafico::procesarMuertesPorVejez()
{
    EntradaVejez tope;
    while (colaVejez.ver_primero(tope) && tope.tiempoMuerte <= tiempoSimulacion)
    {
        colaVejez.eliminar_primero(tope);

        int indice = -1;
        if (!indicesBichitos.buscar(tope.id, indice))
            continue;

        const Bichito& b = poblacion.obtenerRef(indice);
        if (b.esInmortal())
            continue;

        ++muertesVejez;
        agregarComida(b.getX(), b.getY(), 2, 30, OrigenComida::Cadaver);
        eliminarBichitoEn(indice);
    }
}

void MotorGrafico::encolarCondenados()
{
    for (int i = 0; i < poblacion.longitud(); ++i)
    {
        Bichito &bichito = poblacion.obtenerRef(i);
        if (bichito.condenar())
        {
            colaEliminacion.insertar_al_ultimo(bichito.getId());
        }
    }
}

void MotorGrafico::procesarColaEliminacion(int elementosPendientes)
{
    for (int i = 0; i < elementosPendientes; ++i)
    {
        int idCondenado;
        if (!colaEliminacion.eliminar_primero(idCondenado))
        {
            return;
        }

        int indice;
        if (!indicesBichitos.buscar(idCondenado, indice))
        {
            continue;
        }

        Bichito &bichito = poblacion.obtenerRef(indice);
        if (!bichito.estaCondenado())
        {
            continue;
        }

        agregarComida(bichito.getX(), bichito.getY(), 2, 30, OrigenComida::Cadaver);
        eliminarBichitoEn(indice, false);
        return;
    }
}

void MotorGrafico::eliminarComidaEn(int indice)
{
    const int ultimoIndice = comidasActivas.longitud() - 1;
    const int idEliminado = comidasActivas.obtenerRef(indice).getId();
    const int idMovido = indice != ultimoIndice ? comidasActivas.obtenerRef(ultimoIndice).getId() : -1;

    indicesComida.eliminar(idEliminado);
    comidasActivas.morir(indice);
    if (indice != ultimoIndice)
    {
        indicesComida.insertar(idMovido, indice);
    }
}

void MotorGrafico::vaciarComidas()
{
    comidasActivas.vaciar();
    indicesComida.vaciar();
}

void MotorGrafico::agregarBichito()
{
    static int idNuevo = 1000;
    const int rangoX = std::max(1, static_cast<int>(tamanoMundo.x) - 80);
    const int rangoY = std::max(1, static_cast<int>(tamanoMundo.y) - 80);
    const float x = 40.0f + static_cast<float>(std::rand() % rangoX);
    const float y = 40.0f + static_cast<float>(std::rand() % rangoY);

    // MODIFICADO: Nace con genoma aleatorio inicial
    Bichito b(idNuevo++, 40.0f, x, y, generarGenomaInicial());
    agregarBichitoAlFinal(b);
}

void MotorGrafico::quitarBichito()
{
    if (poblacion.longitud() > 0)
        eliminarBichitoEn(poblacion.longitud() - 1);
}

void MotorGrafico::cargarSimulacionGeneral()
{
    modoActual = ModoSimulacion::Normal;
    pausado = true;
    vaciarComidas();
    vaciarPoblacion();
    mezclarBolsaComida();

    const int rangoX = std::max(1, static_cast<int>(tamanoMundo.x) - 80);
    const int rangoY = std::max(1, static_cast<int>(tamanoMundo.y) - 80);
    const int cantidadInicial = std::max(1, bichitosIniciales);
    for (int i = 0; i < cantidadInicial; ++i) {
        const float x = 40.0f + static_cast<float>(std::rand() % rangoX);
        const float y = 40.0f + static_cast<float>(std::rand() % rangoY);
        const float vidaInicial = 40.0f + static_cast<float>(std::rand() % 56);
        
        // MODIFICADO: Cada fundador nace con su propio genoma aleatorio
        Bichito b(i + 1, vidaInicial, x, y, generarGenomaInicial());
        agregarBichitoAlFinal(b);
    }

    constexpr int COMIDA_INICIAL_GENERAL = 18;
    for (int i = 0; i < COMIDA_INICIAL_GENERAL; ++i) {
        const int prioridad = obtenerRarezaSiguiente();
        const int puntosVida = prioridad == 1 ? 50 : (prioridad == 2 ? 30 : 15);
        const float cx = 30.0f + (static_cast<float>(std::rand()) / RAND_MAX) * (Config::ANCHO_MUNDO - 60.0f);
        const float cy = 30.0f + (static_cast<float>(std::rand()) / RAND_MAX) * (Config::ALTO_MUNDO - 60.0f);
        agregarComida(cx, cy, prioridad, puntosVida);
    }

    intervaloComida = calcularIntervaloComida(estaciones.obtener_actual().nombre);
    temporizadorComida = 0.0f;
    tiempoAlertaDesastre = 0.0f;
    motorDesastres.reiniciar();
}

void MotorGrafico::cargarEscenarioCompetencia(int nAgentes)
{
    if (nAgentes < 0) nAgentes = agentesCompetencia;
    if (nAgentes < 1) nAgentes = 1;

    modoActual = ModoSimulacion::EscenarioCompetencia;
    vaciarComidas();
    vaciarPoblacion();

    float centroX = tamanoMundo.x / 2.0f;
    float centroY = tamanoMundo.y / 2.0f;
    agregarComida(centroX, centroY, 1, 50);

    float radioInicial = 300.0f;
    for (int i = 0; i < nAgentes; ++i) {
        float angulo = (2.0f * 3.14159f / nAgentes) * i;
        float bx = centroX + radioInicial * std::cos(angulo);
        float by = centroY + radioInicial * std::sin(angulo);

        Bichito b(i + 1, 20.0f, bx, by);
        b.setTarget(centroX, centroY);
        b.setEstado(EstadoBichito::BuscandoComida);
        agregarBichitoAlFinal(b);
    }
}

void MotorGrafico::cargarEscenarioReproduccion()
{
    modoActual = ModoSimulacion::EscenarioReproduccion;
    vaciarComidas();
    vaciarPoblacion();

    float centroX = tamanoMundo.x / 2.0f;
    float centroY = tamanoMundo.y / 2.0f;

    Bichito b1(1, 50.0f, centroX - 80.0f, centroY);
    Bichito b2(2, 50.0f, centroX + 80.0f, centroY);

    b1.setTarget(centroX - 10.0f, centroY);
    b2.setTarget(centroX + 10.0f, centroY);

    b1.setEstado(EstadoBichito::BuscandoPareja);
    b2.setEstado(EstadoBichito::BuscandoPareja);

    agregarBichitoAlFinal(b1);
    agregarBichitoAlFinal(b2);
}

void MotorGrafico::procesarEvento(const sf::Event& evento)
{
    if (evento.type == sf::Event::KeyPressed) {
        if (evento.key.code == sf::Keyboard::Num1) {
            cargarEscenarioCompetencia();
        } else if (evento.key.code == sf::Keyboard::Num2) {
            cargarEscenarioReproduccion();
        } else if (evento.key.code == sf::Keyboard::R) {
            modoActual = ModoSimulacion::Normal;
        }
    }

    if (evento.type == sf::Event::MouseButtonPressed &&
        evento.mouseButton.button == sf::Mouse::Left &&
        static_cast<float>(evento.mouseButton.x) < tamanoMundo.x &&
        static_cast<float>(evento.mouseButton.y) < tamanoMundo.y) {

        const float mx = static_cast<float>(evento.mouseButton.x);
        const float my = static_cast<float>(evento.mouseButton.y);
        const float radioClic = 20.0f;

        int idEncontrado = -1;
        float mejorDistSq = radioClic * radioClic;

        for (int i = 0; i < poblacion.longitud(); ++i) {
            const Bichito& b = poblacion.obtenerRef(i);
            const float dx = b.getX() - mx;
            const float dy = b.getY() - my;
            const float distSq = dx * dx + dy * dy;
            if (distSq <= mejorDistSq) {
                mejorDistSq = distSq;
                idEncontrado = b.getId();
            }
        }

        if (idEncontrado != -1) {
            idBichitoSeleccionado = (idBichitoSeleccionado == idEncontrado) ? -1 : idEncontrado;
        } else if (idBichitoSeleccionado != -1) {
            idBichitoSeleccionado = -1;
        } else {
            agregarComida(mx, my, prioridadComida, 30, OrigenComida::Clic);
        }
    }
}

void MotorGrafico::actualizar(float dt)
{
    tiempoVisual += dt;
    actualizarEfectosClima(dt);
    if (tiempoAlertaDesastre > 0.0f)
    {
        tiempoAlertaDesastre = std::max(0.0f, tiempoAlertaDesastre - dt);
    }

    if (pausado) return;
    actualizarFisica(dt * velocidadSim);
}

void MotorGrafico::actualizarFisica(float dt)
{
    tiempoSimulacion += dt;

    temporizadorEstacion += dt;
    if (temporizadorEstacion >= tiempoCambioEstacion) {
        estaciones.siguiente_estación();
        temporizadorEstacion = 0.0f;
    }

    Estacion estacionActual = estaciones.obtener_actual();

    if (modoActual == ModoSimulacion::EscenarioCompetencia) {
        procesarEscenarioCompetencia(dt);
    } else if (modoActual == ModoSimulacion::EscenarioReproduccion) {
        procesarEscenarioReproduccion(dt);
    } else {
        procesarSimulacionGeneral(dt);
    }

    for (int i = 0; i < poblacion.longitud(); ++i) {
        Bichito& b = poblacion.obtenerRef(i);
        b.envejecer(dt * ANIOS_POR_SEGUNDO);
        b.cambiarVida(-(estacionActual.desgasteVida * 1.15f * dt));

        if (b.getCooldownReproduccion() > 0.0f) {
            b.setCooldownReproduccion(b.getCooldownReproduccion() - dt);
        }
    }

    procesarMuertesPorHambre();
    procesarMuertesPorVejez();
}

void MotorGrafico::procesarEscenarioCompetencia(float dt)
{
    if (comidasActivas.longitud() == 0)
    {
        for (int i = 0; i < poblacion.longitud(); ++i)
        {
            Bichito &b = poblacion.obtenerRef(i);
            if (b.estaCondenado())
            {
                b.actualizarSoloPaseo(dt);
            }
        }
        return;
    }

    Comida& comidaUnica = comidasActivas.obtenerRef(0);
    float radioColisionComida = 15.0f;

    for (int i = 0; i < poblacion.longitud(); ++i) {
        Bichito& b = poblacion.obtenerRef(i);
        if (b.estaCondenado())
        {
            b.actualizarSoloPaseo(dt);
            continue;
        }
        b.setTarget(comidaUnica.getX(), comidaUnica.getY());
        b.moverHaciaObjetivo(dt);

        float dx = comidaUnica.getX() - b.getX();
        float dy = comidaUnica.getY() - b.getY();
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist <= radioColisionComida && comidaUnica.isActiva()) {
            comidaUnica.setActiva(false);
            b.cambiarVida(static_cast<float>(comidaUnica.getPuntosDeVida()));
            std::cout << "¡El Bichito ID " << b.getId() << " ha ganado la comida y aumentado su vida!\n";
        }
    }

    if (!comidaUnica.isActiva()) {
        vaciarComidas();
    }
}

void MotorGrafico::procesarEscenarioReproduccion(float dt)
{
    float radioProximidad = 35.0f;
    float tiempoRequerido = 2.0f;

    for (int i = 0; i < poblacion.longitud(); ++i) {
        Bichito& b = poblacion.obtenerRef(i);
        if (b.estaCondenado())
        {
            b.actualizarSoloPaseo(dt);
        }
        else
        {
            b.moverHaciaObjetivo(dt);
        }
    }

    if (poblacion.longitud() >= 2 &&
        !poblacion.obtenerRef(0).estaCondenado() &&
        !poblacion.obtenerRef(1).estaCondenado()) {
        Bichito& b1 = poblacion.obtenerRef(0);
        Bichito& b2 = poblacion.obtenerRef(1);

        float dx = b1.getX() - b2.getX();
        float dy = b1.getY() - b2.getY();
        float dist = std::sqrt(dx * dx + dy * dy);

        if (dist <= radioProximidad) {
            b1.setTiempoCercaPareja(b1.getTiempoCercaPareja() + dt);
            b2.setTiempoCercaPareja(b2.getTiempoCercaPareja() + dt);

            if (b1.getTiempoCercaPareja() >= tiempoRequerido) {
                float hijoX = (b1.getX() + b2.getX()) / 2.0f;
                float hijoY = (b1.getY() + b2.getY()) / 2.0f;

                int nuevoId = poblacion.longitud() + 100;
                Bichito hijo(nuevoId, 40.0f, hijoX, hijoY);
                hijo.setTarget(hijoX + (rand() % 100 - 50), hijoY + (rand() % 100 - 50));
                agregarBichitoAlFinal(hijo);

                std::cout << "¡Nacio un nuevo Bichito (ID: " << nuevoId << ") por reproduccion!\n";

                Bichito& p1 = poblacion.obtenerRef(0);
                Bichito& p2 = poblacion.obtenerRef(1);

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

    constexpr int MAX_COMIDA_ACTIVA = 80;
    if (temporizadorComida >= intervaloComida)
    {
        float margen = 30.0f;
        float minX = margen;
        float maxX = Config::ANCHO_MUNDO - margen;
        float minY = margen;
        float maxY = Config::ALTO_MUNDO - margen;

        float rangoX = maxX - minX;
        float rangoY = maxY - minY;

        if (comidasActivas.longitud() < MAX_COMIDA_ACTIVA)
        {
            const int prioridad = obtenerRarezaSiguiente();
            const int puntosVida = prioridad == 1 ? 50 : (prioridad == 2 ? 30 : 15);
            float rx = minX + (static_cast<float>(rand()) / RAND_MAX) * rangoX;
            float ry = minY + (static_cast<float>(rand()) / RAND_MAX) * rangoY;
            agregarComida(rx, ry, prioridad, puntosVida);
        }

        intervaloComida = calcularIntervaloComida(estaciones.obtener_actual().nombre);
        temporizadorComida = 0.0f;
    }

    rtreeComida.limpiar();
    for (int i = 0; i < comidasActivas.longitud(); ++i) {
        const Comida& c = comidasActivas.obtenerRef(i);
        if (c.isActiva()) {
            rtreeComida.insertar(c.getId(), c.getX(), c.getY());
        }
    }

    const int poblacionAntes = poblacion.longitud();
    for (int i = 0; i < poblacionAntes; ++i) {
        Bichito &bichito = poblacion.obtenerRef(i);
        bichito.actualizar(dt, comidasActivas, indicesComida, rtreeComida, poblacion, indicesBichitos);
    }
    for (int i = poblacionAntes; i < poblacion.longitud(); ++i) {
        registrarEnColaVejez(poblacion.obtenerRef(i));
    }
    
    for (int i = comidasActivas.longitud() - 1; i >= 0; --i) {
        Comida& c = comidasActivas.obtenerRef(i);
        if (!c.isActiva())
            eliminarComidaEn(i);
    }

    if (motorDesastres.actualizar(dt, poblacion, [this](int id) { eliminarBichitoPorId(id); }))
    {
        const ResultadoDesastre& res = motorDesastres.getUltimoResultado();
        if (res.bajasReales > 0)
        {
            mostrarAlertaDesastre(res);
        }
    }
}

void MotorGrafico::dibujar(sf::RenderTarget& destino, GestorRecursos& recursos)
{
    const std::string estacion = enMinusculas(estaciones.obtener_actual().nombre);
    recursos.dibujarFondo(destino, "fondos/fondo_" + estacion, sf::Vector2u(250, 180),
    colorFondoEstacion(estacion), sf::Vector2f(0.0f, 0.0f), tamanoMundo);

    for (int i = 0; i < comidasActivas.longitud(); ++i) {
        const Comida& c = comidasActivas.obtenerRef(i);
        if (!c.isActiva()) continue;

        const sf::Texture& tex = recursos.textura("entidades/comida_" + std::to_string(c.getPrioridad()),
        sf::Vector2u(8, 8), colorComida(c.getPrioridad()),
        Forma::Circulo);
        sf::Sprite sprite(tex);
        sprite.setOrigin(tex.getSize().x / 2.0f, tex.getSize().y / 2.0f);
        sprite.setScale(Config::ESCALA_COMIDA, Config::ESCALA_COMIDA);
        sprite.setPosition(std::round(c.getX()), std::round(c.getY()));
        destino.draw(sprite);
    }

    // MODIFICADO: Bichitos coloreados dinámicamente según su genética
    for (int i = 0; i < poblacion.longitud(); ++i) {
        const Bichito& b = poblacion.obtenerRef(i);

        const sf::Texture& tex = recursos.textura("entidades/bichito", sf::Vector2u(16, 16),
        sf::Color::White, Forma::Circulo);
        sf::Sprite sprite(tex);
        sprite.setOrigin(tex.getSize().x / 2.0f, tex.getSize().y / 2.0f);
        sprite.setColor(colorPorGenoma(b.getGenetica()));

        float diametro = 20.0f;

        if (Config::BICHITO_CRECE)
            diametro = 2.0f * (10.0f + b.getVida() / 10.0f);

        float escala = diametro / static_cast<float>(tex.getSize().x);
        sprite.setScale(escala, escala);

        sprite.setPosition(std::round(b.getX()), std::round(b.getY()));
        destino.draw(sprite);
    }

    dibujarEfectosEstacion(destino, estacion);
    dibujarAnimacionDesastre(destino);
    dibujarTooltipBichito(destino, recursos);
    dibujarAlertaDesastre(destino, recursos);
}

void MotorGrafico::mostrarAlertaDesastre(const ResultadoDesastre& resultado)
{
    alertaDesastre = resultado;
    tiempoAlertaDesastre = duracionAlertaDesastre;
}

void MotorGrafico::dibujarAlertaDesastre(sf::RenderTarget& destino, GestorRecursos& recursos) const
{
    if (tiempoAlertaDesastre <= 0.0f || alertaDesastre.tipo == TipoDesastre::Ninguno)
        return;

    const float tiempoTranscurrido = duracionAlertaDesastre - tiempoAlertaDesastre;

    float factorAlpha = 1.0f;
    if (tiempoAlertaDesastre < 1.0f)
    {
        factorAlpha = std::max(0.0f, tiempoAlertaDesastre);
    }

    const sf::Color acento = colorDesastre(alertaDesastre.tipo);

    if (tiempoTranscurrido < 0.65f)
    {
        const float flash = 1.0f - (tiempoTranscurrido / 0.65f);
        sf::RectangleShape overlay(tamanoMundo);
        overlay.setPosition(0.0f, 0.0f);
        overlay.setFillColor(sf::Color(acento.r, acento.g, acento.b,
                                       static_cast<sf::Uint8>(flash * 110.0f)));
        destino.draw(overlay);
    }

    const float cx = tamanoMundo.x / 2.0f;
    const float cy = tamanoMundo.y / 2.0f;
    const sf::Vector2f tamPanel(680.0f, 185.0f);

    sf::RectangleShape panel(tamPanel);
    panel.setOrigin(tamPanel.x / 2.0f, tamPanel.y / 2.0f);
    panel.setPosition(cx, cy);
    panel.setFillColor(sf::Color(14, 14, 22, static_cast<sf::Uint8>(225.0f * factorAlpha)));
    panel.setOutlineThickness(5.0f);
    panel.setOutlineColor(sf::Color(acento.r, acento.g, acento.b,
                                    static_cast<sf::Uint8>(255.0f * factorAlpha)));
    destino.draw(panel);

    auto centrarTexto = [](sf::Text& t, float x, float y) {
        const sf::FloatRect b = t.getLocalBounds();
        t.setOrigin(std::round(b.left + b.width / 2.0f), std::round(b.top + b.height / 2.0f));
        t.setPosition(std::round(x), std::round(y));
    };

    const sf::Uint8 alphaTexto = static_cast<sf::Uint8>(255.0f * factorAlpha);

    const std::string encabezadoStr = (alertaDesastre.tipo == TipoDesastre::Sobrepoblacion)
                                          ? "¡LA PARCA EQUILIBRA LA POBLACIÓN!"
                                          : "¡DESASTRE NATURAL!";
    sf::Text txtEncabezado(utf8(encabezadoStr), recursos.fuente(), 18);
    txtEncabezado.setFillColor(sf::Color(acento.r, acento.g, acento.b, alphaTexto));
    txtEncabezado.setOutlineColor(sf::Color(0, 0, 0, alphaTexto));
    txtEncabezado.setOutlineThickness(2.0f);
    centrarTexto(txtEncabezado, cx, cy - 58.0f);
    destino.draw(txtEncabezado);

    sf::Text txtTitulo(utf8(tituloImpactante(alertaDesastre.tipo)), recursos.fuente(), 38);
    txtTitulo.setStyle(sf::Text::Bold);
    txtTitulo.setFillColor(sf::Color(255, 255, 255, alphaTexto));
    txtTitulo.setOutlineColor(sf::Color(0, 0, 0, alphaTexto));
    txtTitulo.setOutlineThickness(3.0f);
    centrarTexto(txtTitulo, cx, cy - 10.0f);
    destino.draw(txtTitulo);

    const std::string nStr = std::to_string(alertaDesastre.bajasReales);
    std::string bajasStr;
    switch (alertaDesastre.tipo)
    {
    case TipoDesastre::Meteoritos:
        bajasStr = "LOS METEORITOS HAN ELIMINADO " + nStr + " BICHITOS";
        break;
    case TipoDesastre::Diluvio:
        bajasStr = "EL DILUVIO HA ELIMINADO " + nStr + " BICHITOS";
        break;
    case TipoDesastre::BombaNuclear:
        bajasStr = "LA BOMBA NUCLEAR HA ELIMINADO " + nStr + " BICHITOS";
        break;
    case TipoDesastre::PatriaMilagro:
        bajasStr = "ABELARDO ELIMINÓ A " + nStr + " GUERRILLEROS (BICHITOS)";
        break;
    case TipoDesastre::Sobrepoblacion:
    default:
        bajasStr = "LA PARCA ELIMINÓ " + nStr + " BICHITOS";
        break;
    }
    sf::Text txtBajas(utf8(bajasStr), recursos.fuente(), 18);
    txtBajas.setFillColor(sf::Color(255, 225, 190, alphaTexto));
    txtBajas.setOutlineColor(sf::Color(0, 0, 0, alphaTexto));
    txtBajas.setOutlineThickness(2.0f);
    centrarTexto(txtBajas, cx, cy + 36.0f);
    destino.draw(txtBajas);

    const std::string nivelesStr =
        "Prioridad  N1: " + std::to_string(alertaDesastre.bajasPorNivel[0]) +
        "  |  N2: " + std::to_string(alertaDesastre.bajasPorNivel[1]) +
        "  |  N3: " + std::to_string(alertaDesastre.bajasPorNivel[2]);
    sf::Text txtNiveles(utf8(nivelesStr), recursos.fuente(), 14);
    txtNiveles.setFillColor(sf::Color(190, 200, 220, alphaTexto));
    centrarTexto(txtNiveles, cx, cy + 68.0f);
    destino.draw(txtNiveles);
}

void MotorGrafico::inicializarParticulasClima()
{
    particulasClima.clear();
    particulasClima.reserve(60);

    for (int i = 0; i < 60; ++i)
    {
        ParticulaClima p;
        p.x = (static_cast<float>(std::rand()) / RAND_MAX) * tamanoMundo.x;
        p.y = (static_cast<float>(std::rand()) / RAND_MAX) * tamanoMundo.y;
        p.vx = 12.0f + (static_cast<float>(std::rand()) / RAND_MAX) * 20.0f;
        p.vy = 26.0f + (static_cast<float>(std::rand()) / RAND_MAX) * 42.0f;
        p.tam = 3.5f + (static_cast<float>(std::rand()) / RAND_MAX) * 4.5f;
        p.rotacion = (static_cast<float>(std::rand()) / RAND_MAX) * 360.0f;
        p.velRot = -70.0f + (static_cast<float>(std::rand()) / RAND_MAX) * 140.0f;
        p.fase = (static_cast<float>(std::rand()) / RAND_MAX) * 6.28318f;
        p.variante = i % 4;
        particulasClima.push_back(p);
    }
}

void MotorGrafico::actualizarEfectosClima(float dt)
{
    for (auto& p : particulasClima)
    {
        p.y += p.vy * dt;
        p.x += (p.vx * 0.45f + std::sin(tiempoVisual * 1.8f + p.fase) * 22.0f) * dt;
        p.rotacion += p.velRot * dt;

        if (p.y > tamanoMundo.y + 12.0f)
        {
            p.y = -12.0f;
            p.x = (static_cast<float>(std::rand()) / RAND_MAX) * tamanoMundo.x;
        }
        if (p.x > tamanoMundo.x - 6.0f)
            p.x = 6.0f;
        else if (p.x < 6.0f)
            p.x = tamanoMundo.x - 6.0f;
    }
}

void MotorGrafico::dibujarEfectosEstacion(sf::RenderTarget& destino, const std::string& estacion) const
{
    if (estacion == "invierno")
    {
        for (const auto& p : particulasClima)
        {
            const sf::Color colorCopo = (p.variante == 0)
                                            ? sf::Color(200, 235, 255, 210)
                                            : sf::Color(245, 250, 255, 220);
            const float largo = p.tam * 1.7f;
            const float grosor = std::max(1.5f, p.tam * 0.32f);

            sf::RectangleShape barra1(sf::Vector2f(largo, grosor));
            barra1.setOrigin(largo / 2.0f, grosor / 2.0f);
            barra1.setPosition(p.x, p.y);
            barra1.setRotation(p.rotacion);
            barra1.setFillColor(colorCopo);
            destino.draw(barra1);

            sf::RectangleShape barra2(sf::Vector2f(largo, grosor));
            barra2.setOrigin(largo / 2.0f, grosor / 2.0f);
            barra2.setPosition(p.x, p.y);
            barra2.setRotation(p.rotacion + 60.0f);
            barra2.setFillColor(colorCopo);
            destino.draw(barra2);

            sf::RectangleShape barra3(sf::Vector2f(largo, grosor));
            barra3.setOrigin(largo / 2.0f, grosor / 2.0f);
            barra3.setPosition(p.x, p.y);
            barra3.setRotation(p.rotacion + 120.0f);
            barra3.setFillColor(colorCopo);
            destino.draw(barra3);
        }
        return;
    }

    if (estacion == "otono")
    {
        const sf::Color paletaOtono[4] = {
            sf::Color(235, 125, 35, 215),
            sf::Color(210, 75, 35, 215),
            sf::Color(240, 185, 45, 215),
            sf::Color(175, 90, 40, 210)
        };
        for (const auto& p : particulasClima)
        {
            sf::CircleShape hoja(p.tam * 0.95f, 4);
            hoja.setOrigin(hoja.getRadius(), hoja.getRadius());
            hoja.setScale(1.45f, 0.72f);
            hoja.setPosition(p.x, p.y);
            hoja.setRotation(p.rotacion);
            hoja.setFillColor(paletaOtono[p.variante]);
            destino.draw(hoja);
        }
        return;
    }

    if (estacion == "primavera")
    {
        const sf::Color paletaPrimavera[4] = {
            sf::Color(255, 175, 205, 210),
            sf::Color(255, 210, 225, 205),
            sf::Color(255, 245, 250, 200),
            sf::Color(235, 170, 245, 195)
        };
        for (const auto& p : particulasClima)
        {
            sf::CircleShape petalo(p.tam * 0.85f, 8);
            petalo.setOrigin(petalo.getRadius(), petalo.getRadius());
            petalo.setScale(1.35f, 0.7f);
            petalo.setPosition(p.x, p.y);
            petalo.setRotation(p.rotacion);
            petalo.setFillColor(paletaPrimavera[p.variante]);
            destino.draw(petalo);
        }
        return;
    }

    if (estacion == "verano")
    {
        const sf::Vector2f posSol(tamanoMundo.x - 86.0f, 82.0f);
        const float pulso = 1.0f + 0.07f * std::sin(tiempoVisual * 2.5f);

        sf::CircleShape haloExt(74.0f * pulso);
        haloExt.setOrigin(haloExt.getRadius(), haloExt.getRadius());
        haloExt.setPosition(posSol);
        haloExt.setFillColor(sf::Color(255, 215, 70, 28));
        destino.draw(haloExt);

        sf::CircleShape haloMed(52.0f * pulso);
        haloMed.setOrigin(haloMed.getRadius(), haloMed.getRadius());
        haloMed.setPosition(posSol);
        haloMed.setFillColor(sf::Color(255, 225, 85, 55));
        destino.draw(haloMed);

        for (int i = 0; i < 12; ++i)
        {
            const float ang = tiempoVisual * 20.0f + static_cast<float>(i) * 30.0f;
            sf::RectangleShape rayo(sf::Vector2f(22.0f, 5.0f));
            rayo.setOrigin(-38.0f, 2.5f);
            rayo.setPosition(posSol);
            rayo.setRotation(ang);
            rayo.setFillColor(sf::Color(255, 230, 95, 190));
            destino.draw(rayo);
        }

        sf::CircleShape disco(32.0f);
        disco.setOrigin(32.0f, 32.0f);
        disco.setPosition(posSol);
        disco.setFillColor(sf::Color(255, 232, 95, 235));
        destino.draw(disco);

        for (int i = 0; i < static_cast<int>(particulasClima.size()); i += 2)
        {
            const auto& p = particulasClima[static_cast<std::size_t>(i)];
            const float brillo = 0.5f + 0.5f * std::sin(tiempoVisual * 3.0f + p.fase);
            sf::CircleShape mota(p.tam * 0.55f);
            mota.setOrigin(mota.getRadius(), mota.getRadius());
            mota.setPosition(p.x, tamanoMundo.y - p.y);
            mota.setFillColor(sf::Color(255, 238, 130, static_cast<sf::Uint8>(135.0f * brillo)));
            destino.draw(mota);
        }
    }
}

void MotorGrafico::dibujarAnimacionDesastre(sf::RenderTarget& destino) const
{
    if (tiempoAlertaDesastre <= 0.0f || alertaDesastre.tipo == TipoDesastre::Ninguno)
        return;

    const float t = duracionAlertaDesastre - tiempoAlertaDesastre;
    const float progreso = std::min(1.0f, std::max(0.0f, t / duracionAlertaDesastre));
    const float fade = (tiempoAlertaDesastre < 1.0f) ? std::max(0.0f, tiempoAlertaDesastre) : 1.0f;

    if (alertaDesastre.tipo == TipoDesastre::Diluvio)
    {
        sf::RectangleShape velo(tamanoMundo);
        velo.setFillColor(sf::Color(15, 45, 95, static_cast<sf::Uint8>(95.0f * fade)));
        destino.draw(velo);

        if ((t > 0.15f && t < 0.28f) || (t > 1.35f && t < 1.46f))
        {
            sf::RectangleShape rayoFlash(tamanoMundo);
            rayoFlash.setFillColor(sf::Color(210, 235, 255, 130));
            destino.draw(rayoFlash);
        }

        sf::VertexArray gotas(sf::Lines);
        for (int i = 0; i < 140; ++i)
        {
            const float seedX = static_cast<float>((i * 73) % static_cast<int>(tamanoMundo.x));
            const float seedY = static_cast<float>((i * 137) % static_cast<int>(tamanoMundo.y));
            const float gx = std::fmod(seedX + t * 240.0f, tamanoMundo.x);
            const float gy = std::fmod(seedY + t * 760.0f, tamanoMundo.y);
            const sf::Color cGota(140, 210, 255, static_cast<sf::Uint8>(215.0f * fade));
            gotas.append(sf::Vertex(sf::Vector2f(gx, gy), cGota));
            gotas.append(sf::Vertex(sf::Vector2f(std::min(tamanoMundo.x, gx + 8.0f),
                                                 std::min(tamanoMundo.y, gy + 24.0f)), cGota));
        }
        destino.draw(gotas);
        return;
    }

    if (alertaDesastre.tipo == TipoDesastre::BombaNuclear)
    {
        const float cx = tamanoMundo.x / 2.0f;
        const float cy = tamanoMundo.y / 2.0f;

        if (t < 1.35f)
        {
            const float intensidad = 1.0f - (t / 1.35f);
            const float curva = intensidad * intensidad;
            sf::RectangleShape flashBlanco(tamanoMundo);
            flashBlanco.setFillColor(sf::Color(255, 255, 255,
                                               static_cast<sf::Uint8>(255.0f * curva)));
            destino.draw(flashBlanco);
        }

        const float radioOnda = 30.0f + t * 420.0f;
        if (radioOnda < 850.0f)
        {
            const float alphaOnda = std::max(0.0f, 1.0f - (radioOnda / 850.0f));
            sf::CircleShape onda(radioOnda, 48);
            onda.setOrigin(radioOnda, radioOnda);
            onda.setPosition(cx, cy);
            onda.setFillColor(sf::Color(255, 240, 140, static_cast<sf::Uint8>(45.0f * alphaOnda)));
            onda.setOutlineThickness(10.0f);
            onda.setOutlineColor(sf::Color(255, 255, 230, static_cast<sf::Uint8>(230.0f * alphaOnda)));
            destino.draw(onda);
        }
        return;
    }

    if (alertaDesastre.tipo == TipoDesastre::Meteoritos)
    {
        for (int i = 0; i < 8; ++i)
        {
            const float inicio = static_cast<float>(i) * 0.28f;
            const float localT = t - inicio;
            if (localT < 0.0f || localT > 1.8f) continue;

            const float destinoX = 140.0f + static_cast<float>((i * 193) % 740);
            const float destinoY = 140.0f + static_cast<float>((i * 131) % 460);

            if (localT < 0.75f)
            {
                const float f = localT / 0.75f;
                const float mx = (destinoX - 280.0f) + 280.0f * f;
                const float my = (destinoY - 320.0f) + 320.0f * f;

                if (mx >= 0.0f && mx <= tamanoMundo.x && my >= 0.0f && my <= tamanoMundo.y)
                {
                    sf::Vertex estela[2] = {
                        sf::Vertex(sf::Vector2f(std::max(0.0f, mx - 55.0f), std::max(0.0f, my - 63.0f)),
                                   sf::Color(255, 110, 20, 0)),
                        sf::Vertex(sf::Vector2f(mx, my), sf::Color(255, 180, 50, 235))
                    };
                    destino.draw(estela, 2, sf::Lines);

                    sf::CircleShape roca(10.0f);
                    roca.setOrigin(10.0f, 10.0f);
                    roca.setPosition(mx, my);
                    roca.setFillColor(sf::Color(255, 125, 35, 240));
                    roca.setOutlineThickness(3.0f);
                    roca.setOutlineColor(sf::Color(255, 235, 110, 230));
                    destino.draw(roca);
                }
            }
            else
            {
                const float impF = (localT - 0.75f) / 1.05f;
                const float rImp = 12.0f + impF * 70.0f;
                const sf::Uint8 aImp = static_cast<sf::Uint8>(220.0f * (1.0f - impF) * fade);

                sf::CircleShape anillo(rImp, 32);
                anillo.setOrigin(rImp, rImp);
                anillo.setPosition(destinoX, destinoY);
                anillo.setFillColor(sf::Color(255, 110, 30, static_cast<sf::Uint8>(aImp / 3)));
                anillo.setOutlineThickness(4.0f);
                anillo.setOutlineColor(sf::Color(255, 175, 65, aImp));
                destino.draw(anillo);
            }
        }
        return;
    }

    if (alertaDesastre.tipo == TipoDesastre::Sobrepoblacion)
    {
        sf::RectangleShape velo(tamanoMundo);
        velo.setFillColor(sf::Color(40, 8, 18, static_cast<sf::Uint8>(115.0f * fade)));
        destino.draw(velo);

        const float px = tamanoMundo.x / 2.0f;
        const float py = 148.0f + std::sin(t * 3.5f) * 8.0f;
        const sf::Uint8 a = static_cast<sf::Uint8>(245.0f * fade);

        sf::CircleShape aura(74.0f);
        aura.setOrigin(74.0f, 74.0f);
        aura.setPosition(px, py);
        aura.setFillColor(sf::Color(180, 20, 45, static_cast<sf::Uint8>(65.0f * fade)));
        destino.draw(aura);

        sf::ConvexShape manto(3);
        manto.setPoint(0, sf::Vector2f(px, py - 46.0f));
        manto.setPoint(1, sf::Vector2f(px - 56.0f, py + 62.0f));
        manto.setPoint(2, sf::Vector2f(px + 56.0f, py + 62.0f));
        manto.setFillColor(sf::Color(18, 16, 26, a));
        manto.setOutlineThickness(3.0f);
        manto.setOutlineColor(sf::Color(150, 35, 55, a));
        destino.draw(manto);

        sf::CircleShape capucha(26.0f);
        capucha.setOrigin(26.0f, 26.0f);
        capucha.setPosition(px, py - 24.0f);
        capucha.setFillColor(sf::Color(10, 8, 16, a));
        capucha.setOutlineThickness(2.5f);
        capucha.setOutlineColor(sf::Color(165, 40, 60, a));
        destino.draw(capucha);

        sf::CircleShape ojoIzq(4.5f);
        ojoIzq.setOrigin(4.5f, 4.5f);
        ojoIzq.setPosition(px - 9.0f, py - 24.0f);
        ojoIzq.setFillColor(sf::Color(255, 45, 55, a));
        destino.draw(ojoIzq);

        sf::CircleShape ojoDer(4.5f);
        ojoDer.setOrigin(4.5f, 4.5f);
        ojoDer.setPosition(px + 9.0f, py - 24.0f);
        ojoDer.setFillColor(sf::Color(255, 45, 55, a));
        destino.draw(ojoDer);

        const float angGuadana = -35.0f + std::sin(t * 5.0f) * 28.0f;
        sf::RectangleShape mango(sf::Vector2f(7.0f, 130.0f));
        mango.setOrigin(3.5f, 95.0f);
        mango.setPosition(px + 42.0f, py + 24.0f);
        mango.setRotation(angGuadana);
        mango.setFillColor(sf::Color(115, 85, 65, a));
        destino.draw(mango);

        const float rad = (angGuadana - 90.0f) * 3.14159265f / 180.0f;
        const float puntaX = (px + 42.0f) + std::cos(rad) * 90.0f;
        const float puntaY = (py + 24.0f) + std::sin(rad) * 90.0f;
        sf::ConvexShape hoja(4);
        hoja.setPoint(0, sf::Vector2f(0.0f, 0.0f));
        hoja.setPoint(1, sf::Vector2f(-54.0f, -8.0f));
        hoja.setPoint(2, sf::Vector2f(-70.0f, 14.0f));
        hoja.setPoint(3, sf::Vector2f(-12.0f, 12.0f));
        hoja.setPosition(puntaX, puntaY);
        hoja.setRotation(angGuadana);
        hoja.setFillColor(sf::Color(220, 232, 245, a));
        hoja.setOutlineThickness(2.0f);
        hoja.setOutlineColor(sf::Color(255, 90, 100, a));
        destino.draw(hoja);
        return;
    }

    if (alertaDesastre.tipo == TipoDesastre::PatriaMilagro)
    {
        constexpr int COLUMNAS = 40;
        const float anchoCol = tamanoMundo.x / static_cast<float>(COLUMNAS);
        const sf::Uint8 alphaBandera = static_cast<sf::Uint8>(135.0f * fade);

        for (int i = 0; i < COLUMNAS; ++i)
        {
            const float x = static_cast<float>(i) * anchoCol;
            const float onda = std::sin(t * 6.0f + static_cast<float>(i) * 0.35f) * 14.0f;

            const float hAmarillo = tamanoMundo.y * 0.50f;
            const float hAzul = tamanoMundo.y * 0.25f;
            const float hRojo = tamanoMundo.y * 0.25f;

            sf::RectangleShape franjaAm(sf::Vector2f(anchoCol + 1.0f, hAmarillo + 20.0f));
            franjaAm.setPosition(x, onda - 10.0f);
            franjaAm.setFillColor(sf::Color(252, 209, 22, alphaBandera));
            destino.draw(franjaAm);

            sf::RectangleShape franjaAz(sf::Vector2f(anchoCol + 1.0f, hAzul + 4.0f));
            franjaAz.setPosition(x, hAmarillo + onda);
            franjaAz.setFillColor(sf::Color(0, 56, 147, alphaBandera));
            destino.draw(franjaAz);

            sf::RectangleShape franjaRo(sf::Vector2f(anchoCol + 1.0f, hRojo + 24.0f));
            franjaRo.setPosition(x, hAmarillo + hAzul + onda);
            franjaRo.setFillColor(sf::Color(206, 17, 38, alphaBandera));
            destino.draw(franjaRo);
        }
    }

    (void)progreso;
}

void MotorGrafico::dibujarTooltipBichito(sf::RenderTarget& destino, GestorRecursos& recursos) const
{
    if (idBichitoSeleccionado == -1)
        return;

    int indice = -1;
    if (!indicesBichitos.buscar(idBichitoSeleccionado, indice) ||
        indice < 0 || indice >= poblacion.longitud())
    {
        return;
    }

    const Bichito& b = poblacion.obtenerRef(indice);
    const Genoma gen = b.getGenetica();
    const sf::Color colorGen = colorPorGenoma(gen);

    // 1. Aro resaltado alrededor del bichito seleccionado
    float radioBichito = 10.0f;
    if (Config::BICHITO_CRECE)
        radioBichito = 10.0f + b.getVida() / 10.0f;

    sf::CircleShape aro(radioBichito + 5.0f);
    aro.setOrigin(radioBichito + 5.0f, radioBichito + 5.0f);
    aro.setPosition(std::round(b.getX()), std::round(b.getY()));
    aro.setFillColor(sf::Color::Transparent);
    aro.setOutlineThickness(2.5f);
    aro.setOutlineColor(sf::Color(255, 235, 90, 240));
    destino.draw(aro);

    // 2. Armar texto con la información completa del bichito
    const bool esInmortalTotal = b.esInmortal() ||
                                gen.genEstatico1 == GenEstatico::INMORTAL ||
                                gen.genEstatico2 == GenEstatico::INMORTAL;

    const std::string edadStr = esInmortalTotal
        ? "Inmortal"
        : std::to_string(static_cast<int>(b.getEdadActual())) + " / " +
          std::to_string(static_cast<int>(b.getEdadMaxima())) + " años";

    const int metabEntero = static_cast<int>(gen.metabolismo);
    const int metabDecimal = static_cast<int>(std::round((gen.metabolismo - metabEntero) * 10.0f));
    const std::string metabStr = std::to_string(metabEntero) + "." + std::to_string(std::abs(metabDecimal));

    const std::string info =
        "Bichito #" + std::to_string(b.getId()) + "  [" + nombreEstadoBichito(b.getEstado()) + "]\n" +
        "Vida: " + std::to_string(static_cast<int>(b.getVida())) + "  |  Edad: " + edadStr + "\n" +
        "Vel: " + std::to_string(static_cast<int>(gen.velocidad)) + " px/s  |  Metab: " + metabStr + "\n" +
        "Gen 1: " + nombreGenEstatico(gen.genEstatico1) + "\n" +
        "Gen 2: " + nombreGenEstatico(gen.genEstatico2);

    sf::Text texto(utf8(info), recursos.fuente(), 13);
    texto.setFillColor(sf::Color(245, 248, 255));

    const sf::FloatRect bounds = texto.getLocalBounds();
    const float padX = 10.0f;
    const float padY = 8.0f;
    const float anchoCaja = bounds.width + padX * 2.0f;
    const float altoCaja = bounds.height + padY * 2.0f;

    // 3. Posicionar tarjeta encima del bichito sin salirse de los bordes del mapa
    float cajaX = b.getX() - anchoCaja * 0.5f;
    float cajaY = b.getY() - radioBichito - altoCaja - 12.0f;

    if (cajaX < 8.0f)
        cajaX = 8.0f;
    if (cajaX + anchoCaja > tamanoMundo.x - 8.0f)
        cajaX = tamanoMundo.x - anchoCaja - 8.0f;
    if (cajaY < 8.0f)
        cajaY = b.getY() + radioBichito + 12.0f;

    sf::RectangleShape fondo(sf::Vector2f(anchoCaja, altoCaja));
    fondo.setPosition(std::round(cajaX), std::round(cajaY));
    fondo.setFillColor(sf::Color(16, 20, 32, 232));
    fondo.setOutlineThickness(2.0f);
    fondo.setOutlineColor(colorGen);

    texto.setPosition(std::round(cajaX + padX - bounds.left),
                    std::round(cajaY + padY - bounds.top));

    destino.draw(fondo);
    destino.draw(texto);
}
    
void MotorGrafico::mezclarBolsaComida()
{
    for (int i = static_cast<int>(bolsaComida.size()) - 1; i > 0; --i) {
        const int indiceAleatorio = std::rand() % (i + 1);
        std::swap(bolsaComida[i], bolsaComida[indiceAleatorio]);
    }
    indiceBolsaComida = 0;
}

int MotorGrafico::obtenerRarezaSiguiente() {
    const std::string& nombreEstacion = estaciones.obtener_actual().nombre;
    const bool excluirRara = (nombreEstacion == "Otono" || nombreEstacion == "Invierno");

    while (true) {
        if (indiceBolsaComida >= static_cast<int>(bolsaComida.size())) {
            mezclarBolsaComida();
        }

        const int rareza = bolsaComida[indiceBolsaComida];

        if (excluirRara && rareza == 1) {
            indiceBolsaComida++; 
            continue;
        }
        indiceBolsaComida++;
        return rareza;
    }
}

float MotorGrafico::calcularIntervaloComida(const std::string& nombreEstacion)
{
    float base = 0.95f;
    if (nombreEstacion == "Primavera") base = 0.65f;
    else if (nombreEstacion == "Verano") base = 0.95f;
    else if (nombreEstacion == "Otono") base = 1.60f;
    else if (nombreEstacion == "Invierno") base = 2.40f;

    float factor = 0.7f + 0.6f * (static_cast<float>(std::rand()) / RAND_MAX);
    return base * factor;
}