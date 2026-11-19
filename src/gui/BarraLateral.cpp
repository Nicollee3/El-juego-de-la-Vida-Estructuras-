#include "gui/BarraLateral.h"
#include "MotorGrafico.h"
#include "gui/Config.h"
#include "gui/GestorRecursos.h"
#include <string>

namespace
{
    // Coordenadas de la barra (la ventana mide 1280 de ancho; la barra empieza en x = 1000).
    constexpr float X0 = Config::ANCHO_MUNDO;                  // borde izquierdo de la barra
    constexpr float CX = X0 + Config::ANCHO_BARRA / 2.0f;      // centro horizontal de la barra
    constexpr float MARGEN = 22.0f;
    constexpr float X_MINI_IZQ = CX - 90.0f;
    constexpr float X_MINI_DER = CX + 90.0f;

    const float VELOCIDADES[] = {0.5f, 1.0f, 2.0f, 4.0f};
    const char* NOMBRES_VELOCIDAD[] = {"0.5x", "1x", "2x", "4x"};
    constexpr int NUM_VELOCIDADES = 4;

    void dibujarTarjeta(sf::RenderTarget& destino, float x, float y, float ancho, float alto,
                        sf::Color relleno, sf::Color borde)
    {
        sf::RectangleShape rect(sf::Vector2f(ancho, alto));
        rect.setPosition(x, y);
        rect.setFillColor(relleno);
        rect.setOutlineThickness(1.5f);
        rect.setOutlineColor(borde);
        destino.draw(rect);
    }
}

BarraLateral::BarraLateral(GestorRecursos& recursos, MotorGrafico& motor, std::function<void()> alVolverAlMenu)
    : recursos_(recursos),
      motor_(motor),
      alVolverAlMenu_(std::move(alVolverAlMenu)),
      btnPausa_(nullptr),
      btnPrioridad_{nullptr, nullptr, nullptr},
      titulo_(recursos, "SIMULACIÓN", 20, sf::Vector2f(CX, 20), Etiqueta::Alineacion::Centro),
      lineaEstacion_(recursos, "", 13, sf::Vector2f(X0 + MARGEN, 50)),
      lineaBichitos_(recursos, "", 13, sf::Vector2f(X0 + MARGEN, 68)),
      lineaComida_(recursos, "", 13, sf::Vector2f(X0 + MARGEN, 86)),
      lineaArreglo_(recursos, "", 13, sf::Vector2f(X0 + MARGEN, 104)),
      lineaDesastre_(recursos, "", 12, sf::Vector2f(X0 + MARGEN, 122)),
      etVelocidad_(recursos, "", 14, sf::Vector2f(CX, 222), Etiqueta::Alineacion::Centro),
      etBichitos_(recursos, "Bichitos", 14, sf::Vector2f(CX, 310), Etiqueta::Alineacion::Centro),
      etPrioridad_(recursos, "Tamaño de comida:", 13, sf::Vector2f(CX, 344), Etiqueta::Alineacion::Centro),
      etDesastres_(recursos, "DESASTRES · LA PARCA", 13, sf::Vector2f(CX, 432), Etiqueta::Alineacion::Centro),
      etAyuda_(recursos, "Espacio=pausa  Esc=menú", 12, sf::Vector2f(CX, 616), Etiqueta::Alineacion::Centro),
      indiceVelocidad_(1) // empieza en 1x
{
    titulo_.setColor(sf::Color(225, 248, 255));
    lineaEstacion_.setColor(sf::Color(225, 232, 245));
    lineaBichitos_.setColor(sf::Color(115, 215, 255));
    lineaComida_.setColor(sf::Color(255, 220, 110));
    lineaArreglo_.setColor(sf::Color(180, 195, 215));
    lineaDesastre_.setColor(sf::Color(255, 145, 140));
    etPrioridad_.setColor(sf::Color(200, 215, 230));
    etDesastres_.setColor(sf::Color(255, 130, 135));
    etAyuda_.setColor(sf::Color(150, 168, 190));

    const sf::Vector2u tamAncho(58, 9);  // 232x36 px
    const sf::Vector2u tamMedio(28, 9);  // 112x36 px
    const sf::Vector2u tamMini(13, 9);   // 52x36 px
    const sf::Vector2u tamMenu(58, 10);  // 232x40 px

    // --- Pausa / Reanudar ---
    btnPausa_ = crearBoton("Pausar", sf::Vector2f(CX, 176),
                           [this] { motor_.setPausa(!motor_.estaPausado()); },
                           "ui/boton_barra_ancho", tamAncho);

    // --- Velocidad:  [-]  0.5x/1x/2x/4x  [+] ---
    crearBoton("-", sf::Vector2f(X_MINI_IZQ, 222), [this] { cambiarVelocidad(-1); }, "ui/boton_barra_mini", tamMini);
    crearBoton("+", sf::Vector2f(X_MINI_DER, 222), [this] { cambiarVelocidad(+1); }, "ui/boton_barra_mini", tamMini);

    // --- Escenarios (lado a lado) ---
    crearBoton("Esc. 1", sf::Vector2f(CX - 60.0f, 266), [this] { motor_.cargarEscenarioCompetencia(); },
               "ui/boton_barra_medio", tamMedio);
    crearBoton("Esc. 2", sf::Vector2f(CX + 60.0f, 266), [this] { motor_.cargarEscenarioReproduccion(); },
               "ui/boton_barra_medio", tamMedio);

    // --- Bichitos:  [-] quitar   [+] agregar ---
    crearBoton("-", sf::Vector2f(X_MINI_IZQ, 310), [this] { motor_.quitarBichito(); }, "ui/boton_barra_mini", tamMini);
    crearBoton("+", sf::Vector2f(X_MINI_DER, 310), [this] { motor_.agregarBichito(); }, "ui/boton_barra_mini", tamMini);

    // --- Prioridad de la comida que se coloca con clic: 1, 2, 3 ---
    const float xs[3] = {CX - 76.0f, CX, CX + 76.0f};
    const sf::Color tintesComida[3] = {
        sf::Color(255, 225, 120), // dorada
        sf::Color(140, 235, 155), // verde
        sf::Color(215, 175, 135)  // cafe
    };
    for (int i = 0; i < 3; ++i)
    {
        btnPrioridad_[i] = crearBoton(std::to_string(3 - i), sf::Vector2f(xs[i], 374),
                                      [this, i] { motor_.setPrioridadComida(i + 1); },
                                      "ui/boton_barra_mini", tamMini);
        btnPrioridad_[i]->setColorTinte(tintesComida[i]);
    }

    // --- Desastres Naturales (La Parca) con color temático ---
    Boton* btnMet = crearBoton("Meteoritos", sf::Vector2f(CX - 60.0f, 468),
                               [this] { motor_.activarDesastre(TipoDesastre::Meteoritos); },
                               "ui/boton_barra_medio", tamMedio);
    btnMet->setColorTinte(sf::Color(255, 170, 115));

    Boton* btnDil = crearBoton("Diluvio", sf::Vector2f(CX + 60.0f, 468),
                               [this] { motor_.activarDesastre(TipoDesastre::Diluvio); },
                               "ui/boton_barra_medio", tamMedio);
    btnDil->setColorTinte(sf::Color(135, 205, 255));

    Boton* btnNuc = crearBoton("Nuclear", sf::Vector2f(CX - 60.0f, 514),
                               [this] { motor_.activarDesastre(TipoDesastre::BombaNuclear); },
                               "ui/boton_barra_medio", tamMedio);
    btnNuc->setColorTinte(sf::Color(245, 230, 125));

    Boton* btnPat = crearBoton("Patria M.", sf::Vector2f(CX + 60.0f, 514),
                               [this] { motor_.activarDesastre(TipoDesastre::PatriaMilagro); },
                               "ui/boton_barra_medio", tamMedio);
    btnPat->setColorTinte(sf::Color(255, 210, 95));

    Boton* btnSob = crearBoton("Sobrepoblación", sf::Vector2f(CX, 560),
                               [this] { motor_.activarDesastre(TipoDesastre::Sobrepoblacion); },
                               "ui/boton_barra_ancho", tamAncho);
    btnSob->setColorTinte(sf::Color(255, 125, 135));

    // --- Volver al menú ---
    Boton* btnMenu = crearBoton("Menú", sf::Vector2f(CX, 658), [this] {
        if (alVolverAlMenu_) alVolverAlMenu_();
    }, "ui/boton_barra_menu", tamMenu);
    btnMenu->setColorTinte(sf::Color(185, 200, 225));

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
    lineaDesastre_.setTexto("Desastre: " + motor_.getResumenUltimoDesastre());
    etVelocidad_.setTexto(std::string("Vel: ") + NOMBRES_VELOCIDAD[indiceVelocidad_]);

    if (motor_.estaPausado())
    {
        btnPausa_->setTexto("Reanudar");
        btnPausa_->setColorTinte(sf::Color(125, 235, 180));
    }
    else
    {
        btnPausa_->setTexto("Pausar");
        btnPausa_->setColorTinte(sf::Color(250, 210, 125));
    }

    for (int i = 0; i < 3; ++i)
        btnPrioridad_[i]->setSeleccionado(motor_.getPrioridadComida() == i + 1);
}

void BarraLateral::dibujar(sf::RenderTarget& destino) const
{
    // 1. Fondo con gradiente oscuro elegante
    sf::VertexArray fondo(sf::Quads, 4);
    fondo[0] = sf::Vertex(sf::Vector2f(X0, 0.0f), sf::Color(17, 23, 38));
    fondo[1] = sf::Vertex(sf::Vector2f(X0 + Config::ANCHO_BARRA, 0.0f), sf::Color(22, 29, 46));
    fondo[2] = sf::Vertex(sf::Vector2f(X0 + Config::ANCHO_BARRA, Config::ALTO_MUNDO), sf::Color(14, 28, 38));
    fondo[3] = sf::Vertex(sf::Vector2f(X0, Config::ALTO_MUNDO), sf::Color(12, 22, 34));
    destino.draw(fondo);

    // Línea divisoria izquierda
    sf::RectangleShape separador(sf::Vector2f(2.0f, Config::ALTO_MUNDO));
    separador.setPosition(X0, 0.0f);
    separador.setFillColor(sf::Color(70, 195, 190, 140));
    destino.draw(separador);

    // 2. Tarjetas de sección (Estado, Controles y Desastres)
    const float xTarjeta = X0 + 14.0f;
    const float wTarjeta = Config::ANCHO_BARRA - 28.0f;

    dibujarTarjeta(destino, xTarjeta, 38.0f, wTarjeta, 100.0f,
                   sf::Color(10, 15, 26, 195), sf::Color(65, 155, 170, 110));
    dibujarTarjeta(destino, xTarjeta, 148.0f, wTarjeta, 256.0f,
                   sf::Color(10, 15, 26, 175), sf::Color(80, 110, 155, 95));
    dibujarTarjeta(destino, xTarjeta, 414.0f, wTarjeta, 178.0f,
                   sf::Color(26, 13, 21, 195), sf::Color(215, 75, 88, 145));

    // 3. Textos y botones
    titulo_.dibujar(destino);
    lineaEstacion_.dibujar(destino);
    lineaBichitos_.dibujar(destino);
    lineaComida_.dibujar(destino);
    lineaArreglo_.dibujar(destino);
    lineaDesastre_.dibujar(destino);
    etVelocidad_.dibujar(destino);
    etBichitos_.dibujar(destino);
    etPrioridad_.dibujar(destino);
    etDesastres_.dibujar(destino);
    etAyuda_.dibujar(destino);

    for (const auto& b : botones_)
        b->dibujar(destino);
}
