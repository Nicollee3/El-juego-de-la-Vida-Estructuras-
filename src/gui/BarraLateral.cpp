#include "gui/BarraLateral.h"
#include "MotorGrafico.h"
#include "gui/Config.h"
#include "gui/GestorRecursos.h"
#include <string>

namespace
{
    // Coordenadas de la barra (la ventana mide 1280 de ancho; la barra empieza en x = 1000).
    constexpr float X0 = Config::ANCHO_MUNDO;                 // borde izquierdo de la barra
    constexpr float CX = X0 + Config::ANCHO_BARRA / 2.0f;      // centro horizontal de la barra
    constexpr float MARGEN = 16.0f;
    constexpr float X_MINI_IZQ = X0 + MARGEN + 32.0f;          // centro del botón mini izquierdo
    constexpr float X_MINI_DER = X0 + Config::ANCHO_BARRA - MARGEN - 32.0f;

    const float VELOCIDADES[] = {0.5f, 1.0f, 2.0f, 4.0f};
    const char* NOMBRES_VELOCIDAD[] = {"0.5x", "1x", "2x", "4x"};
    constexpr int NUM_VELOCIDADES = 4;
}

BarraLateral::BarraLateral(GestorRecursos& recursos, MotorGrafico& motor, std::function<void()> alVolverAlMenu)
    : recursos_(recursos),
      motor_(motor),
      alVolverAlMenu_(std::move(alVolverAlMenu)),
      btnPausa_(nullptr),
      btnPrioridad_{nullptr, nullptr, nullptr},
      titulo_(recursos, "SIMULACIÓN", 24, sf::Vector2f(CX, 22), Etiqueta::Alineacion::Centro),
      lineaEstacion_(recursos, "", 16, sf::Vector2f(X0 + MARGEN, 54)),
      lineaBichitos_(recursos, "", 16, sf::Vector2f(X0 + MARGEN, 76)),
      lineaComida_(recursos, "", 16, sf::Vector2f(X0 + MARGEN, 98)),
      lineaArreglo_(recursos, "", 16, sf::Vector2f(X0 + MARGEN, 120)),
      etVelocidad_(recursos, "", 16, sf::Vector2f(CX, 252), Etiqueta::Alineacion::Centro),
      etBichitos_(recursos, "Bichitos", 16, sf::Vector2f(CX, 456), Etiqueta::Alineacion::Centro),
      etPrioridad_(recursos, "Prioridad de comida:", 16, sf::Vector2f(CX, 506), Etiqueta::Alineacion::Centro),
      etAyuda_(recursos, "Espacio=pausa Esc=menú", 16, sf::Vector2f(CX, 604), Etiqueta::Alineacion::Centro),
      indiceVelocidad_(1) // empieza en 1x
{
    const sf::Vector2u mini(16, 16);

    // --- Pausa / Reanudar ---
    btnPausa_ = crearBoton("Pausar", sf::Vector2f(CX, 184),
                           [this] { motor_.setPausa(!motor_.estaPausado()); });

    // --- Velocidad:  [-]  0.5x/1x/2x/4x  [+] ---
    crearBoton("-", sf::Vector2f(X_MINI_IZQ, 252), [this] { cambiarVelocidad(-1); }, "ui/boton_mini", mini);
    crearBoton("+", sf::Vector2f(X_MINI_DER, 252), [this] { cambiarVelocidad(+1); }, "ui/boton_mini", mini);

    // --- Escenarios ---
    crearBoton("Escenario 1", sf::Vector2f(CX, 320), [this] { motor_.cargarEscenarioCompetencia(); });
    crearBoton("Escenario 2", sf::Vector2f(CX, 388), [this] { motor_.cargarEscenarioReproduccion(); });

    // --- Bichitos:  [-] quitar   [+] agregar (usa el arreglo dinámico) ---
    crearBoton("-", sf::Vector2f(X_MINI_IZQ, 456), [this] { motor_.quitarBichito(); }, "ui/boton_mini", mini);
    crearBoton("+", sf::Vector2f(X_MINI_DER, 456), [this] { motor_.agregarBichito(); }, "ui/boton_mini", mini);

    // --- Prioridad de la comida que se coloca con clic: 1, 2, 3 ---
    const float xs[3] = {CX - 80.0f, CX, CX + 80.0f};
    for (int i = 0; i < 3; ++i)
    {
        btnPrioridad_[i] = crearBoton(std::to_string(i + 1), sf::Vector2f(xs[i], 552),
                                      [this, i] { motor_.setPrioridadComida(i + 1); },
                                      "ui/boton_mini", mini);
    }

    // --- Volver al menú ---
    crearBoton("Menú", sf::Vector2f(CX, 656), [this] {
        if (alVolverAlMenu_) alVolverAlMenu_();
    });

    actualizar();
}

Boton* BarraLateral::crearBoton(const std::string& texto, sf::Vector2f centro, std::function<void()> alClic,
                                const std::string& estilo, sf::Vector2u tamNativo)
{
    botones_.push_back(std::make_unique<Boton>(recursos_, texto, centro, std::move(alClic), estilo, tamNativo));
    return botones_.back().get();
}

void BarraLateral::cambiarVelocidad(int delta)
{
    indiceVelocidad_ += delta;
    if (indiceVelocidad_ < 0) indiceVelocidad_ = 0;
    if (indiceVelocidad_ >= NUM_VELOCIDADES) indiceVelocidad_ = NUM_VELOCIDADES - 1;
    motor_.setVelocidad(VELOCIDADES[indiceVelocidad_]);
}

void BarraLateral::procesarEvento(const sf::Event& evento)
{
    for (auto& b : botones_)
        b->procesarEvento(evento);
}

void BarraLateral::actualizar()
{
    lineaEstacion_.setTexto("Estación: " + motor_.getEstacionActual().nombre);
    lineaBichitos_.setTexto("Bichitos: " + std::to_string(motor_.getNumBichitos()));
    lineaComida_.setTexto("Comida: " + std::to_string(motor_.getNumComidas()));
    lineaArreglo_.setTexto("Arreglo: " + std::to_string(motor_.getNumBichitos()) + "/" +
                           std::to_string(motor_.getCapacidadPoblacion()));
    etVelocidad_.setTexto(std::string("Vel: ") + NOMBRES_VELOCIDAD[indiceVelocidad_]);

    btnPausa_->setTexto(motor_.estaPausado() ? "Reanudar" : "Pausar");

    for (int i = 0; i < 3; ++i)
        btnPrioridad_[i]->setSeleccionado(motor_.getPrioridadComida() == i + 1);
}

void BarraLateral::dibujar(sf::RenderTarget& destino) const
{
    // Fondo del panel (textura "ui/panel_barra", imagen de 70x180 estirada a 280x720).
    recursos_.dibujarFondo(destino, "ui/panel_barra", sf::Vector2u(70, 180), sf::Color(45, 45, 62),
                           sf::Vector2f(X0, 0.0f), sf::Vector2f(Config::ANCHO_BARRA, Config::ALTO_MUNDO));

    titulo_.dibujar(destino);
    lineaEstacion_.dibujar(destino);
    lineaBichitos_.dibujar(destino);
    lineaComida_.dibujar(destino);
    lineaArreglo_.dibujar(destino);
    etVelocidad_.dibujar(destino);
    etBichitos_.dibujar(destino);
    etPrioridad_.dibujar(destino);
    etAyuda_.dibujar(destino);

    for (const auto& b : botones_)
        b->dibujar(destino);
}
