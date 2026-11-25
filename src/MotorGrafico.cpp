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
        return Genoma(120.0f);
    }

    sf::Color colorPorGenoma(const Genoma& g)
    {
        auto tiene = [&](GenEstatico buscado) {
            return g.genEstatico1 == buscado || g.genEstatico2 == buscado;
        };

        if (tiene(GenEstatico::CANIBALISMO)) return sf::Color(32, 110, 165);
        return sf::Color(70, 190, 255); // Azul celeste: Bichito estándar
    }

    constexpr int POBLACION_INICIAL_GENERAL = 10;
    constexpr float ANIOS_POR_SEGUNDO = 1.0f; // Un año de edad por segundo de simulación
    constexpr float PROBABILIDAD_MUTACION_GEN_ESTATICO = 0.05f;
    constexpr std::array<GenEstatico, 6> GENES_ESTATICOS = {
        GenEstatico::INMUNIDAD,
        GenEstatico::CANIBALISMO,
        GenEstatico::VAMPIRISMO,
        GenEstatico::TELETRANSPORTARSE,
        GenEstatico::INMORTAL,
        GenEstatico::MITOSIS
    };

    sf::String utf8(const std::string& texto)
    {
        return sf::String::fromUtf8(texto.begin(), texto.end());
    }

    sf::Color colorDesastre(TipoDesastre tipo)
    {
        switch (tipo)
        {
        case TipoDesastre::Meteoritos:        return sf::Color(255, 115, 35); // naranja fuego
        case TipoDesastre::LluviaAcida:       return sf::Color(125, 245, 75); // verde ácido
        case TipoDesastre::TormentaSolar:     return sf::Color(255, 225, 65); // dorado solar
        case TipoDesastre::ErupcionVolcanica: return sf::Color(255, 78, 35);  // lava volcánica
        case TipoDesastre::Sobrepoblacion: return sf::Color(225, 50, 70);   // rojo parca
        case TipoDesastre::Ninguno:
        default:                           return sf::Color::White;
        }
    }

    std::string tituloImpactante(TipoDesastre tipo)
    {
        switch (tipo)
        {
        case TipoDesastre::Meteoritos:        return "LLUVIA DE METEORITOS";
        case TipoDesastre::LluviaAcida:       return "LLUVIA ÁCIDA";
        case TipoDesastre::TormentaSolar:     return "TORMENTA SOLAR";
        case TipoDesastre::ErupcionVolcanica: return "ERUPCIÓN VOLCÁNICA";
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

bool MotorGrafico::tieneGenRegistrado(GenEstatico gen) const
{
    return gen != GenEstatico::NINGUNO &&
           registroGenetico.arbol(gen).cantidadPortadores() > 0;
}

bool MotorGrafico::abrirArbolGenetico(GenEstatico gen)
{
    return panelGenetica.abrirArbol(gen, registroGenetico);
}

void MotorGrafico::agregarComida(float x, float y, int prioridad, int puntosVida,
                                 OrigenComida origen)
{
    static int idComida = 1;

    Comida c(idComida++, prioridad, puntosVida, x, y, origen);
    comidasActivas.generar(c);
    indicesComida.insertar(c.getId(), comidasActivas.longitud() - 1);
}

void MotorGrafico::agregarBichitoAlFinal(Bichito bichito, bool permitirFundadorAdicional)
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

    intentarMutarGenEstatico(bichito);
    poblacion.generar(bichito);
    indicesBichitos.insertar(bichito.getId(), poblacion.longitud() - 1);
    registrarEnColaVejez(bichito);
    registrarEnArbolGenealogico(bichito, permitirFundadorAdicional);
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
    registroGenetico.registrarMuerte(idEliminado);
    panelGenetica.marcarFallecido(idEliminado);
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
    registroGenetico.limpiar();
    panelGenetica.cerrar();
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

void MotorGrafico::registrarEnArbolGenealogico(const Bichito& bichito,
                                               bool permitirFundadorAdicional)
{
    registroGenetico.registrarBichito(
        bichito.getId(), bichito.getIdPadre(), bichito.getIdMadre(),
        bichito.getGeneracion(), bichito.nacioPorMitosis(), bichito.getGenetica(),
        permitirFundadorAdicional);
}

void MotorGrafico::intentarMutarGenEstatico(Bichito& bichito)
{
    if (bichito.getGeneracion() <= 1 ||
        static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) >=
            PROBABILIDAD_MUTACION_GEN_ESTATICO)
        return;

    std::array<GenEstatico, GENES_ESTATICOS.size()> disponibles{};
    std::size_t cantidadDisponibles = 0;
    for (GenEstatico gen : GENES_ESTATICOS)
    {
        if (registroGenetico.puedeMutar(gen) && !bichito.tieneGen(gen))
            disponibles[cantidadDisponibles++] = gen;
    }

    if (cantidadDisponibles == 0)
        return;

    const std::size_t indice = static_cast<std::size_t>(std::rand()) % cantidadDisponibles;
    bichito.agregarGenEstatico(disponibles[indice]);
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
        const float muerteEsperada = tiempoSimulacion +
            std::max(0.0f, b.getEdadMaxima() - b.getEdadActual()) / ANIOS_POR_SEGUNDO;
        if (b.getEdadActual() < b.getEdadMaxima() &&
            muerteEsperada - tope.tiempoMuerte > 0.05f)
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

void MotorGrafico::agregarBichito(GenEstatico gen)
{
    static int idNuevo = 1000;
    const int rangoX = std::max(1, static_cast<int>(tamanoMundo.x) - 80);
    const int rangoY = std::max(1, static_cast<int>(tamanoMundo.y) - 80);
    const float x = 40.0f + static_cast<float>(std::rand() % rangoX);
    const float y = 40.0f + static_cast<float>(std::rand() % rangoY);

    const Genoma genoma = gen == GenEstatico::NINGUNO
        ? generarGenomaInicial()
        : Genoma(120.0f, gen);
    Bichito b(idNuevo++, 40.0f, x, y, genoma);
    agregarBichitoAlFinal(b, gen != GenEstatico::NINGUNO);
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
        
        // Todos los fundadores nacen sin genes estáticos.
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
    if (panelGenetica.procesarEvento(evento, registroGenetico))
        return;

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
            if (idBichitoSeleccionado == idEncontrado) {
                idBichitoSeleccionado = -1;
                panelGenetica.cerrar();
            } else {
                idBichitoSeleccionado = idEncontrado;
                int indice = -1;
                if (indicesBichitos.buscar(idEncontrado, indice))
                    panelGenetica.seleccionar(poblacion.obtenerRef(indice));
            }
        } else {
            idBichitoSeleccionado = -1;
            panelGenetica.cerrar();
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
                hijo.setGeneracion(std::max(b1.getGeneracion(), b2.getGeneracion()) + 1);
                hijo.setPadres(b1.getId(), b2.getId(), false);
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
        const std::string nombreEstacion = estaciones.obtener_actual().nombre;
        bichito.actualizar(dt, comidasActivas, indicesComida, rtreeComida,
                           poblacion, indicesBichitos,
                           nombreEstacion == "Verano", nombreEstacion == "Invierno");
    }
    for (int i = poblacionAntes; i < poblacion.longitud(); ++i) {
        Bichito& bichito = poblacion.obtenerRef(i);
        intentarMutarGenEstatico(bichito);
        registrarEnColaVejez(bichito);
        registrarEnArbolGenealogico(bichito);
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

        if (b.tieneGen(GenEstatico::VAMPIRISMO))
        {
            sf::CircleShape aura(45.0f);
            aura.setOrigin(45.0f, 45.0f);
            aura.setPosition(b.getX(), b.getY());
            aura.setFillColor(sf::Color(170, 45, 105, 24));
            aura.setOutlineThickness(1.0f);
            aura.setOutlineColor(sf::Color(210, 80, 145, 75));
            destino.draw(aura);
        }
        if (b.getDestelloTeletransporte() > 0.0f)
        {
            const sf::Uint8 alpha = static_cast<sf::Uint8>(
                180.0f * std::min(1.0f, b.getDestelloTeletransporte() / 0.18f));
            sf::CircleShape destello(25.0f);
            destello.setOrigin(25.0f, 25.0f);
            destello.setPosition(b.getX(), b.getY());
            destello.setFillColor(sf::Color(120, 230, 255, alpha));
            destino.draw(destello);
        }

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

        if (b.getId() == idBichitoSeleccionado)
        {
            const float radioResalte = diametro * 0.5f + 6.0f;
            sf::CircleShape resalte(radioResalte);
            resalte.setOrigin(radioResalte, radioResalte);
            resalte.setPosition(std::round(b.getX()), std::round(b.getY()));
            resalte.setFillColor(sf::Color::Transparent);
            resalte.setOutlineThickness(3.0f);
            resalte.setOutlineColor(sf::Color(255, 230, 90, 245));
            destino.draw(resalte);
        }
    }

    dibujarEfectosEstacion(destino, estacion);
    dibujarAnimacionDesastre(destino);
    dibujarAlertaDesastre(destino, recursos);
    int indiceSeleccionado = -1;
    if (indicesBichitos.buscar(idBichitoSeleccionado, indiceSeleccionado))
        panelGenetica.actualizarSeleccion(poblacion.obtenerRef(indiceSeleccionado));
    panelGenetica.dibujar(destino, recursos, registroGenetico);
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
    case TipoDesastre::LluviaAcida:
        bajasStr = "LA LLUVIA ÁCIDA HA ELIMINADO " + nStr + " BICHITOS";
        break;
    case TipoDesastre::TormentaSolar:
        bajasStr = "LA TORMENTA SOLAR HA ELIMINADO " + nStr + " BICHITOS";
        break;
    case TipoDesastre::ErupcionVolcanica:
        bajasStr = "LA ERUPCIÓN VOLCÁNICA HA ELIMINADO " + nStr + " BICHITOS";
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
    // Helper de pixel art: dibuja una matriz de caracteres alineada a la grilla de píxeles
    // en un único VertexArray (cero rotación suave, cero círculos vectoriales).
    auto agregarQuadPixel = [](sf::VertexArray& va, float x, float y, float tam, sf::Color c)
    {
        va.append(sf::Vertex(sf::Vector2f(x, y), c));
        va.append(sf::Vertex(sf::Vector2f(x + tam, y), c));
        va.append(sf::Vertex(sf::Vector2f(x + tam, y + tam), c));
        va.append(sf::Vertex(sf::Vector2f(x, y + tam), c));
    };

    auto snap = [](float v, float paso) -> float
    {
        return std::floor(v / paso) * paso;
    };

    auto dibujarMatriz = [&](sf::VertexArray& va, float cx, float cy, float px,
                             const char* const* filas, int nFilas, int nCols,
                             sf::Color c1, sf::Color c2, sf::Color c3)
    {
        const float origenX = snap(cx - (nCols * px) * 0.5f, px);
        const float origenY = snap(cy - (nFilas * px) * 0.5f, px);
        for (int r = 0; r < nFilas; ++r)
        {
            for (int col = 0; col < nCols; ++col)
            {
                const char ch = filas[r][col];
                if (ch == '.' || ch == ' ')
                    continue;
                sf::Color color = c1;
                if (ch == '2') color = c2;
                else if (ch == '3') color = c3;
                agregarQuadPixel(va, origenX + col * px, origenY + r * px, px, color);
            }
        }
    };

    sf::VertexArray quads(sf::Quads);

    if (estacion == "invierno")
    {
        // Copos de nieve pixel art (3 tamaños/patrones para dar profundidad)
        static const char* const COPO_GRANDE[5] = {
            "..2..",
            ".212.",
            "21312",
            ".212.",
            "..2.."
        };
        static const char* const COPO_MEDIO[3] = {
            ".2.",
            "212",
            ".2."
        };
        static const char* const COPO_CHICO[2] = {
            "11",
            "11"
        };

        for (const auto& p : particulasClima)
        {
            const float px = (p.variante == 0) ? 4.0f : 3.0f;
            const sf::Color blanco(245, 252, 255, 230);
            const sf::Color celeste(175, 225, 255, 215);
            const sf::Color brillo(255, 255, 255, 255);

            if (p.variante == 0)
                dibujarMatriz(quads, p.x, p.y, px, COPO_GRANDE, 5, 5, blanco, celeste, brillo);
            else if (p.variante == 1 || p.variante == 2)
                dibujarMatriz(quads, p.x, p.y, px, COPO_MEDIO, 3, 3, blanco, celeste, brillo);
            else
                dibujarMatriz(quads, p.x, p.y, px, COPO_CHICO, 2, 2, blanco, celeste, brillo);
        }
        destino.draw(quads);
        return;
    }

    if (estacion == "otono")
    {
        // Hojas otoñales pixel art con 2 frames de balanceo (izq / der) y tallito oscuro
        static const char* const HOJA_IZQ[5] = {
            "...3.",
            "..21.",
            ".211.",
            ".112.",
            "..2.."
        };
        static const char* const HOJA_DER[5] = {
            ".3...",
            ".12..",
            ".112.",
            ".211.",
            "..2.."
        };

        const sf::Color principales[4] = {
            sf::Color(238, 128, 36, 230),
            sf::Color(214, 72, 38, 230),
            sf::Color(242, 188, 46, 230),
            sf::Color(196, 98, 38, 225)
        };
        const sf::Color sombras[4] = {
            sf::Color(186, 84, 22, 230),
            sf::Color(156, 42, 24, 230),
            sf::Color(194, 134, 26, 230),
            sf::Color(140, 62, 24, 225)
        };
        const sf::Color tallo(92, 52, 28, 235);

        for (const auto& p : particulasClima)
        {
            const float px = (p.variante % 2 == 0) ? 4.0f : 3.0f;
            const bool frameDer = std::sin(tiempoVisual * 2.6f + p.fase) >= 0.0f;
            dibujarMatriz(quads, p.x, p.y, px, frameDer ? HOJA_DER : HOJA_IZQ, 5, 5,
                          principales[p.variante], sombras[p.variante], tallo);
        }
        destino.draw(quads);
        return;
    }

    if (estacion == "primavera")
    {
        // Pétalos de sakura / flores pixel art en 2 frames de caída suave
        static const char* const PETALO_A[4] = {
            ".11.",
            "1121",
            ".21.",
            "...."
        };
        static const char* const PETALO_B[4] = {
            "..1.",
            ".112",
            "121.",
            ".1.."
        };

        const sf::Color principales[4] = {
            sf::Color(255, 175, 208, 225),
            sf::Color(255, 212, 228, 220),
            sf::Color(255, 244, 250, 215),
            sf::Color(232, 172, 248, 215)
        };
        const sf::Color sombras[4] = {
            sf::Color(232, 122, 168, 225),
            sf::Color(240, 162, 192, 220),
            sf::Color(235, 208, 224, 215),
            sf::Color(196, 128, 222, 215)
        };

        for (const auto& p : particulasClima)
        {
            const float px = (p.variante == 0) ? 4.0f : 3.0f;
            const bool frameB = std::sin(tiempoVisual * 2.8f + p.fase) >= 0.0f;
            dibujarMatriz(quads, p.x, p.y, px, frameB ? PETALO_B : PETALO_A, 4, 4,
                          principales[p.variante], sombras[p.variante], sombras[p.variante]);
        }
        destino.draw(quads);
        return;
    }

    if (estacion == "verano")
    {
        // 1) Sol 100% pixel art en la esquina superior derecha con 2 frames de rayos
        static const char* const SOL_A[15] = {
            ".......2.......",
            ".......2.......",
            "..2...222...2..",
            "...2.11111.2...",
            "....1133311....",
            "...113333311...",
            "..21333333312..",
            "222133333331222",
            "..21333333312..",
            "...113333311...",
            "....1133311....",
            "...2.11111.2...",
            "..2...222...2..",
            ".......2.......",
            ".......2......."
        };
        static const char* const SOL_B[15] = {
            "...............",
            ".2.....2.....2.",
            "..2...222...2..",
            "...2.11111.2...",
            "....1133311....",
            "...113333311...",
            "..21333333312..",
            ".2213333333122.",
            "..21333333312..",
            "...113333311...",
            "....1133311....",
            "...2.11111.2...",
            "..2...222...2..",
            ".2.....2.....2.",
            "..............."
        };

        const float solX = tamanoMundo.x - 84.0f;
        const float solY = 80.0f;
        const bool frameSolB = (static_cast<int>(tiempoVisual * 2.2f) % 2) == 1;

        // Halo pixelado sutil detrás del sol
        const float pxHalo = 8.0f;
        static const char* const HALO_SOL[9] = {
            "..11111..",
            ".1111111.",
            "111111111",
            "111111111",
            "111111111",
            "111111111",
            "111111111",
            ".1111111.",
            "..11111.."
        };
        const sf::Uint8 alphaHalo = frameSolB ? 36 : 24;
        dibujarMatriz(quads, solX, solY, pxHalo, HALO_SOL, 9, 9,
                      sf::Color(255, 220, 80, alphaHalo),
                      sf::Color(255, 220, 80, alphaHalo),
                      sf::Color(255, 220, 80, alphaHalo));

        dibujarMatriz(quads, solX, solY, 6.0f, frameSolB ? SOL_B : SOL_A, 15, 15,
                      sf::Color(255, 208, 60, 235),  // borde dorado
                      sf::Color(255, 178, 45, 210),  // rayos cálidos
                      sf::Color(255, 245, 145, 245)); // centro brillante

        // 2) Partículas doradas de verano (mismo estilo flotante subiendo, en pixel art)
        static const char* const MOTA_CRUZ[3] = {
            ".2.",
            "212",
            ".2."
        };
        for (int i = 0; i < static_cast<int>(particulasClima.size()); i += 2)
        {
            const auto& p = particulasClima[static_cast<std::size_t>(i)];
            const float brillo = 0.5f + 0.5f * std::sin(tiempoVisual * 3.0f + p.fase);
            const sf::Uint8 aCentro = static_cast<sf::Uint8>(165.0f * brillo);
            const sf::Uint8 aBorde = static_cast<sf::Uint8>(85.0f * brillo);
            const float my = tamanoMundo.y - p.y;

            if (brillo > 0.72f)
            {
                dibujarMatriz(quads, p.x, my, 4.0f, MOTA_CRUZ, 3, 3,
                              sf::Color(255, 248, 175, aCentro),
                              sf::Color(255, 225, 95, aBorde),
                              sf::Color(255, 255, 210, aCentro));
            }
            else
            {
                const float px = 4.0f;
                agregarQuadPixel(quads, snap(p.x, px), snap(my, px), px,
                                 sf::Color(255, 238, 130, aCentro));
            }
        }
        destino.draw(quads);
    }
}

void MotorGrafico::dibujarAnimacionDesastre(sf::RenderTarget& destino) const
{
    if (tiempoAlertaDesastre <= 0.0f || alertaDesastre.tipo == TipoDesastre::Ninguno)
        return;

    const float t = duracionAlertaDesastre - tiempoAlertaDesastre;
    const float fade = (tiempoAlertaDesastre < 1.0f) ? std::max(0.0f, tiempoAlertaDesastre) : 1.0f;

    auto snap = [](float v, float paso) -> float
    {
        return std::floor(v / paso) * paso;
    };

    auto agregarQuad = [&](sf::VertexArray& va, float x, float y, float w, float h, sf::Color c)
    {
        if (x + w <= 0.0f || x >= tamanoMundo.x || y + h <= 0.0f || y >= tamanoMundo.y)
            return;
        const float x0 = std::max(0.0f, x);
        const float y0 = std::max(0.0f, y);
        const float x1 = std::min(tamanoMundo.x, x + w);
        const float y1 = std::min(tamanoMundo.y, y + h);
        va.append(sf::Vertex(sf::Vector2f(x0, y0), c));
        va.append(sf::Vertex(sf::Vector2f(x1, y0), c));
        va.append(sf::Vertex(sf::Vector2f(x1, y1), c));
        va.append(sf::Vertex(sf::Vector2f(x0, y1), c));
    };

    auto dibujarMatriz = [&](sf::VertexArray& va, float cx, float cy, float px,
                             const char* const* filas, int nFilas, int nCols,
                             const sf::Color* paleta)
    {
        const float origenX = snap(cx - (nCols * px) * 0.5f, px);
        const float origenY = snap(cy - (nFilas * px) * 0.5f, px);
        for (int r = 0; r < nFilas; ++r)
        {
            for (int col = 0; col < nCols; ++col)
            {
                const char ch = filas[r][col];
                if (ch < '1' || ch > '6')
                    continue;
                agregarQuad(va, origenX + col * px, origenY + r * px, px, px, paleta[ch - '1']);
            }
        }
    };

    auto dibujarAnilloPixel = [&](sf::VertexArray& va, float cx, float cy, float radio,
                                  float px, sf::Color cBorde, sf::Color cBrillo)
    {
        const int rCeldas = static_cast<int>(radio / px);
        if (rCeldas <= 0)
            return;
        const float baseX = snap(cx, px);
        const float baseY = snap(cy, px);
        const int pasos = std::max(16, rCeldas * 6);
        for (int i = 0; i < pasos; ++i)
        {
            const float ang = (6.283185f * static_cast<float>(i)) / static_cast<float>(pasos);
            const float rx = baseX + std::round(std::cos(ang) * rCeldas) * px;
            const float ry = baseY + std::round(std::sin(ang) * rCeldas) * px;
            agregarQuad(va, rx, ry, px, px, (i % 3 == 0) ? cBrillo : cBorde);
        }
    };

    sf::VertexArray quads(sf::Quads);

    if (alertaDesastre.tipo == TipoDesastre::LluviaAcida)
    {
        // 1. Velo verde tóxico atmosférico
        agregarQuad(quads, 0.0f, 0.0f, tamanoMundo.x, tamanoMundo.y,
                    sf::Color(16, 52, 22, static_cast<sf::Uint8>(105.0f * fade)));

        // 2. Descargas eléctricas ácidas pixel art en zigzag
        const bool rayo1 = (t > 0.15f && t < 0.30f);
        const bool rayo2 = (t > 1.35f && t < 1.50f);
        if (rayo1 || rayo2)
        {
            agregarQuad(quads, 0.0f, 0.0f, tamanoMundo.x, tamanoMundo.y, sf::Color(185, 255, 130, 95));
            const float rxBase = rayo1 ? 260.0f : 740.0f;
            const float pxRayo = 8.0f;
            float rx = snap(rxBase, pxRayo);
            for (int seg = 0; seg < 45; ++seg)
            {
                const float ry = static_cast<float>(seg) * pxRayo * 1.5f;
                if (seg % 6 == 2 || seg % 6 == 3) rx += pxRayo;
                else if (seg % 6 == 5) rx -= pxRayo * 2.0f;
                agregarQuad(quads, rx - pxRayo, ry, pxRayo * 3.0f, pxRayo * 1.5f, sf::Color(110, 245, 75, 165));
                agregarQuad(quads, rx, ry, pxRayo, pxRayo * 1.5f, sf::Color(235, 255, 185, 245));
            }
        }

        // 3. Gotas de lluvia ácida verde neón / lima en escaleritas pixel art 4x4
        const float px = 4.0f;
        const sf::Color cAcido(110, 245, 65, static_cast<sf::Uint8>(225.0f * fade));
        const sf::Color cLima(215, 255, 115, static_cast<sf::Uint8>(240.0f * fade));

        for (int i = 0; i < 130; ++i)
        {
            const float seedX = static_cast<float>((i * 73) % static_cast<int>(tamanoMundo.x));
            const float seedY = static_cast<float>((i * 137) % static_cast<int>(tamanoMundo.y));
            const float gx = snap(std::fmod(seedX + t * 210.0f, tamanoMundo.x), px);
            const float gy = snap(std::fmod(seedY + t * 700.0f, tamanoMundo.y), px);

            agregarQuad(quads, gx, gy, px, px * 2.0f, cAcido);
            agregarQuad(quads, gx + px, gy + px * 2.0f, px, px * 2.0f, cAcido);
            agregarQuad(quads, gx + px * 2.0f, gy + px * 4.0f, px, px * 2.0f, cLima);
        }

        // 4. Burbujas / salpicaduras ácidas pixel art en la superficie
        for (int i = 0; i < 18; ++i)
        {
            const float bx = snap(static_cast<float>((i * 157 + 40) % static_cast<int>(tamanoMundo.x - 40)), 4.0f);
            const float by = snap(static_cast<float>((i * 211 + 80) % static_cast<int>(tamanoMundo.y - 40)), 4.0f);
            const float pulso = std::fmod(t * 2.4f + static_cast<float>(i) * 0.37f, 1.0f);
            const sf::Uint8 aB = static_cast<sf::Uint8>((1.0f - pulso) * 210.0f * fade);
            if (pulso < 0.5f)
            {
                agregarQuad(quads, bx, by, 8.0f, 8.0f, sf::Color(150, 255, 85, aB));
            }
            else
            {
                agregarQuad(quads, bx - 4.0f, by, 4.0f, 4.0f, sf::Color(185, 255, 110, aB));
                agregarQuad(quads, bx + 8.0f, by, 4.0f, 4.0f, sf::Color(185, 255, 110, aB));
                agregarQuad(quads, bx + 2.0f, by - 4.0f, 4.0f, 4.0f, sf::Color(225, 255, 150, aB));
            }
        }
        destino.draw(quads);
        return;
    }

    if (alertaDesastre.tipo == TipoDesastre::TormentaSolar)
    {
        const float cx = tamanoMundo.x / 2.0f;
        const float cy = tamanoMundo.y / 2.0f;

        // 1. Estallido blanco-dorado cegador de la llamarada solar
        if (t < 1.35f)
        {
            const float intensidad = 1.0f - (t / 1.35f);
            const float curva = intensidad * intensidad;
            agregarQuad(quads, 0.0f, 0.0f, tamanoMundo.x, tamanoMundo.y,
                        sf::Color(255, 250, 225, static_cast<sf::Uint8>(255.0f * curva)));
        }

        // 2. Cortina de plasma / llamaradas solares pixel art bajando desde el cielo
        const float colPx = 16.0f;
        const int cols = static_cast<int>(std::ceil(tamanoMundo.x / colPx));
        for (int i = 0; i < cols; ++i)
        {
            const float x = static_cast<float>(i) * colPx;
            const float altura = snap(64.0f + std::sin(t * 7.0f + static_cast<float>(i) * 0.55f) * 44.0f +
                                      std::cos(t * 4.0f + static_cast<float>(i) * 0.9f) * 24.0f, 8.0f);
            agregarQuad(quads, x, 0.0f, colPx, altura,
                        sf::Color(255, 155, 30, static_cast<sf::Uint8>(115.0f * fade)));
            agregarQuad(quads, x, 0.0f, colPx, std::max(8.0f, altura * 0.5f),
                        sf::Color(255, 235, 110, static_cast<sf::Uint8>(145.0f * fade)));
        }

        // 3. Estrella / llamarada solar pixel art arriba del banner
        if (t > 0.2f)
        {
            static const char* const ESTRELLA_A[11] = {
                ".....4.....",
                ".....3.....",
                "..3..2..3..",
                "...21112...",
                "..2134312..",
                "43214441234",
                "..2134312..",
                "...21112...",
                "..3..2..3..",
                ".....3.....",
                ".....4....."
            };
            static const char* const ESTRELLA_B[11] = {
                ".....3.....",
                ".4...2...4.",
                "..3.222.3..",
                "...21312...",
                "..2134312..",
                ".321444123.",
                "..2134312..",
                "...21312...",
                "..3.222.3..",
                ".4...2...4.",
                ".....3....."
            };
            const sf::Uint8 aS = static_cast<sf::Uint8>(240.0f * fade);
            const sf::Color palSolar[4] = {
                sf::Color(255, 145, 35, aS),  // 1 naranja plasma
                sf::Color(255, 205, 55, aS),  // 2 dorado solar
                sf::Color(255, 242, 130, aS), // 3 amarillo intenso
                sf::Color(255, 255, 240, aS)  // 4 núcleo blanco
            };
            const bool frameB = (static_cast<int>(t * 6.0f) % 2) == 1;
            dibujarMatriz(quads, cx, 146.0f, 8.0f, frameB ? ESTRELLA_B : ESTRELLA_A, 11, 11, palSolar);
        }

        // 4. Doble onda electromagnética pixel art expandiéndose desde el centro
        const float radioOnda = 32.0f + t * 420.0f;
        if (radioOnda < 860.0f)
        {
            const float alphaOnda = std::max(0.0f, 1.0f - (radioOnda / 860.0f));
            const sf::Uint8 aO = static_cast<sf::Uint8>(235.0f * alphaOnda);
            dibujarAnilloPixel(quads, cx, cy, radioOnda, 8.0f,
                               sf::Color(255, 225, 95, aO),
                               sf::Color(255, 255, 235, aO));
            if (radioOnda > 56.0f)
            {
                dibujarAnilloPixel(quads, cx, cy, radioOnda - 28.0f, 8.0f,
                                   sf::Color(115, 235, 255, static_cast<sf::Uint8>(aO * 0.75f)),
                                   sf::Color(225, 250, 255, static_cast<sf::Uint8>(aO * 0.75f)));
            }
        }
        destino.draw(quads);
        return;
    }

    if (alertaDesastre.tipo == TipoDesastre::Meteoritos)
    {
        static const char* const METEORO[6] = {
            ".33...",
            "3223..",
            "32112.",
            ".21112",
            "..2112",
            "...22."
        };
        const sf::Color palMeteoro[3] = {
            sf::Color(90, 58, 48, 245),   // 1 roca oscura
            sf::Color(255, 115, 30, 245), // 2 fuego naranja
            sf::Color(255, 232, 95, 245)  // 3 chispa amarilla
        };

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

                // Estela pixelada de bloques de fuego atrás del meteorito
                for (int s = 1; s <= 7; ++s)
                {
                    const float ex = snap(mx - static_cast<float>(s) * 10.0f, 4.0f);
                    const float ey = snap(my - static_cast<float>(s) * 11.5f, 4.0f);
                    const sf::Uint8 aEst = static_cast<sf::Uint8>(std::max(0, 225 - s * 28));
                    const float tamB = (s <= 3) ? 8.0f : 4.0f;
                    const sf::Color cEst = (s % 2 == 0) ? sf::Color(255, 220, 80, aEst)
                                                        : sf::Color(255, 105, 25, aEst);
                    agregarQuad(quads, ex, ey, tamB, tamB, cEst);
                }

                dibujarMatriz(quads, mx, my, 4.0f, METEORO, 6, 6, palMeteoro);
            }
            else
            {
                // Explosión y cráter pixel art al impactar
                const float impF = (localT - 0.75f) / 1.05f;
                const float rImp = 12.0f + impF * 66.0f;
                const sf::Uint8 aImp = static_cast<sf::Uint8>(235.0f * (1.0f - impF) * fade);

                dibujarAnilloPixel(quads, destinoX, destinoY, rImp, 6.0f,
                                   sf::Color(255, 120, 30, aImp),
                                   sf::Color(255, 225, 95, aImp));

                // Chispas cuadradas saltando desde el impacto
                for (int c = 0; c < 6; ++c)
                {
                    const float ang = static_cast<float>(c) * 1.0472f + static_cast<float>(i);
                    const float cx = snap(destinoX + std::cos(ang) * (rImp * 0.75f), 4.0f);
                    const float cy = snap(destinoY + std::sin(ang) * (rImp * 0.75f), 4.0f);
                    agregarQuad(quads, cx, cy, 4.0f, 4.0f, sf::Color(255, 240, 140, aImp));
                }
            }
        }
        destino.draw(quads);
        return;
    }

    if (alertaDesastre.tipo == TipoDesastre::Sobrepoblacion)
    {
        // Velo carmesí oscuro
        agregarQuad(quads, 0.0f, 0.0f, tamanoMundo.x, tamanoMundo.y,
                    sf::Color(40, 8, 18, static_cast<sf::Uint8>(115.0f * fade)));

        // La Parca 100% pixel art (2 frames alternando el barrido de la guadaña)
        static const char* const PARCA_A[16] = {
            ".....22222......",
            "....2111112..555",
            "...2111111125555",
            "...21311131254..",
            "...211111112.4..",
            "....2111112..4..",
            "...211111112.4..",
            "..211111111124..",
            "..21111111114...",
            ".211111111114...",
            ".211111111142...",
            ".211111111142...",
            "21111111114112..",
            "21111111114112..",
            "22112112114222..",
            "..22.22.224....."
        };
        static const char* const PARCA_B[16] = {
            ".....22222......",
            "....2111112.....",
            "...211111112....",
            "...2131113125555",
            "...211111112555.",
            "....2111112..4..",
            "...211111112.4..",
            "..211111111124..",
            "..21111111114...",
            ".211111111114...",
            ".211111111142...",
            ".211111111142...",
            "21111111114112..",
            "21111111114112..",
            "22112112114222..",
            "..22.22.224....."
        };

        const sf::Uint8 a = static_cast<sf::Uint8>(248.0f * fade);
        const sf::Color palParca[5] = {
            sf::Color(16, 14, 24, a),    // 1 túnica negra
            sf::Color(165, 35, 58, a),   // 2 borde carmesí
            sf::Color(255, 45, 55, a),   // 3 ojos rojos brillantes
            sf::Color(125, 88, 62, a),   // 4 mango de guadaña
            sf::Color(225, 238, 250, a)  // 5 hoja plateada
        };

        const float px = tamanoMundo.x / 2.0f;
        const float py = 148.0f + snap(std::sin(t * 3.5f) * 8.0f, 4.0f);
        const bool frameB = (static_cast<int>(t * 4.5f) % 2) == 1;
        dibujarMatriz(quads, px, py, 8.0f, frameB ? PARCA_B : PARCA_A, 16, 16, palParca);
        destino.draw(quads);
        return;
    }

    if (alertaDesastre.tipo == TipoDesastre::ErupcionVolcanica)
    {
        // 1. Velo cálido de humo y lava
        agregarQuad(quads, 0.0f, 0.0f, tamanoMundo.x, tamanoMundo.y,
                    sf::Color(55, 16, 10, static_cast<sf::Uint8>(105.0f * fade)));

        // 2. Volcán pixel art con cráter incandescente y lava cayendo (arriba del banner)
        static const char* const VOLCAN_A[11] = {
            "......54445.......",
            ".....4433344......",
            "......33333.......",
            ".....1143411......",
            "....111343111.....",
            "...12114311211....",
            "..1211131111211...",
            ".122111411111211..",
            "12211111111111211.",
            "121111111111111211",
            "111111111111111111"
        };
        static const char* const VOLCAN_B[11] = {
            ".....5433345......",
            "......43334.......",
            ".....5343435......",
            ".....1134311......",
            "....111434111.....",
            "...12113411211....",
            "..1211141111211...",
            ".122111311111211..",
            "12211114111111211.",
            "121111111111111211",
            "111111111111111111"
        };

        const sf::Uint8 aV = static_cast<sf::Uint8>(245.0f * fade);
        const sf::Color palVolcan[5] = {
            sf::Color(52, 38, 36, aV),    // 1 roca volcánica oscura
            sf::Color(84, 62, 56, aV),    // 2 relieve de roca
            sf::Color(255, 78, 28, aV),   // 3 lava roja-naranja
            sf::Color(255, 178, 42, aV),  // 4 lava dorada brillante
            sf::Color(110, 96, 98, static_cast<sf::Uint8>(190.0f * fade)) // 5 humo/ceniza
        };

        const float vx = tamanoMundo.x / 2.0f;
        const float vy = 152.0f;
        const bool frameB = (static_cast<int>(t * 5.0f) % 2) == 1;
        dibujarMatriz(quads, vx, vy, 8.0f, frameB ? VOLCAN_B : VOLCAN_A, 11, 18, palVolcan);

        // 3. Bombas de lava pixel art disparadas en arco parabólico desde el cráter
        for (int i = 0; i < 10; ++i)
        {
            const float ciclo = std::fmod(t + static_cast<float>(i) * 0.24f, 1.35f);
            const float dir = (i % 2 == 0) ? 1.0f : -1.0f;
            const float velX = dir * (95.0f + static_cast<float>((i * 47) % 180));
            const float velY = -(180.0f + static_cast<float>((i * 31) % 110));
            const float bx = snap(vx + velX * ciclo, 4.0f);
            const float by = snap((vy - 36.0f) + velY * ciclo + 260.0f * ciclo * ciclo, 4.0f);
            const sf::Uint8 aB = static_cast<sf::Uint8>(235.0f * fade);

            agregarQuad(quads, bx - 4.0f, by - 4.0f, 8.0f, 8.0f, sf::Color(255, 85, 25, aB));
            agregarQuad(quads, bx, by, 4.0f, 4.0f, sf::Color(255, 230, 95, aB));
        }

        // 4. Lluvia de ceniza y brasas pixel art por todo el mapa
        const float px = 4.0f;
        for (int i = 0; i < 95; ++i)
        {
            const float seedX = static_cast<float>((i * 89) % static_cast<int>(tamanoMundo.x));
            const float seedY = static_cast<float>((i * 149) % static_cast<int>(tamanoMundo.y));
            const float ex = snap(std::fmod(seedX + t * 55.0f, tamanoMundo.x), px);
            const float ey = snap(std::fmod(seedY + t * 180.0f, tamanoMundo.y), px);
            const sf::Color cBrasa = (i % 3 == 0)
                                         ? sf::Color(255, 195, 55, static_cast<sf::Uint8>(215.0f * fade))
                                         : (i % 3 == 1)
                                               ? sf::Color(255, 85, 30, static_cast<sf::Uint8>(205.0f * fade))
                                               : sf::Color(150, 140, 145, static_cast<sf::Uint8>(165.0f * fade));
            agregarQuad(quads, ex, ey, px, px, cBrasa);
        }
        destino.draw(quads);
    }
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