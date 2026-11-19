#include "gui/PantallaOpciones.h"
#include "gui/Aplicacion.h"
#include "gui/Config.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace
{
    constexpr int MIN_BICHITOS = 1;
    constexpr int MAX_BICHITOS = 300;
    constexpr int PRESETS[5] = {20, 60, 100, 150, 200};
}

PantallaOpciones::PantallaOpciones(Aplicacion& app)
    : Pantalla(app),
      titulo_(app.recursos(), "OPCIONES", 54,
              sf::Vector2f(Config::ANCHO_VENTANA / 2.0f, 82.0f), Etiqueta::Alineacion::Centro),
      etBichitosTitulo_(app.recursos(), "Bichitos iniciales (escribe con teclado o elige):", 20,
                        sf::Vector2f(Config::ANCHO_VENTANA / 2.0f, 188.0f), Etiqueta::Alineacion::Centro),
      etValorBichitos_(app.recursos(), "", 30,
                       sf::Vector2f(Config::ANCHO_VENTANA / 2.0f, 248.0f), Etiqueta::Alineacion::Centro),
      etPresets_(app.recursos(), "Cantidades predeterminadas:", 15,
                 sf::Vector2f(Config::ANCHO_VENTANA / 2.0f, 304.0f), Etiqueta::Alineacion::Centro),
      etEstacion_(app.recursos(), "", 22,
                  sf::Vector2f(Config::ANCHO_VENTANA / 2.0f, 444.0f), Etiqueta::Alineacion::Centro),
      nota_(app.recursos(), "Rango: 1 a 300 bichitos  ·  Los cambios se aplican al iniciar la simulación", 14,
            sf::Vector2f(Config::ANCHO_VENTANA / 2.0f, 520.0f), Etiqueta::Alineacion::Centro),
      btnPresets_{nullptr, nullptr, nullptr, nullptr, nullptr},
      cajaEntrada_(Config::ANCHO_VENTANA / 2.0f - 105.0f, 222.0f, 210.0f, 52.0f),
      textoEntrada_(std::to_string(app.ajustes().bichitosIniciales)),
      tiempo_(0.0f)
{
    titulo_.setColor(sf::Color(235, 250, 255));
    etBichitosTitulo_.setColor(sf::Color(115, 225, 200));
    etValorBichitos_.setColor(sf::Color(255, 255, 255));
    etPresets_.setColor(sf::Color(175, 195, 220));
    etEstacion_.setColor(sf::Color(240, 245, 255));
    nota_.setColor(sf::Color(160, 180, 205));

    const float cx = Config::ANCHO_VENTANA / 2.0f;
    const sf::Vector2u tamPaso(22, 12);   // 88x48 px
    const sf::Vector2u tamPreset(24, 11); // 96x44 px
    const sf::Vector2u tamMini(15, 12);   // 60x48 px
    const sf::Vector2u tamVolver(72, 14); // 288x56 px

    // --- Fila 1: [-10]  [ caja numérica ]  [+10] ---
    botones_.push_back(std::make_unique<Boton>(app.recursos(), "-10", sf::Vector2f(cx - 176.0f, 248.0f),
        [this] { establecerBichitos(app_.ajustes().bichitosIniciales - 10); },
        "ui/boton_op_paso", tamPaso));
    botones_.push_back(std::make_unique<Boton>(app.recursos(), "+10", sf::Vector2f(cx + 176.0f, 248.0f),
        [this] { establecerBichitos(app_.ajustes().bichitosIniciales + 10); },
        "ui/boton_op_paso", tamPaso));

    // --- Fila 2: Presets rápidos (20, 60, 100, 150, 200) ---
    const float separacion = 116.0f;
    const float inicioX = cx - 2.0f * separacion;
    for (int i = 0; i < 5; ++i)
    {
        const int cantidad = PRESETS[i];
        botones_.push_back(std::make_unique<Boton>(
            app.recursos(), std::to_string(cantidad),
            sf::Vector2f(inicioX + static_cast<float>(i) * separacion, 348.0f),
            [this, cantidad] { establecerBichitos(cantidad); },
            "ui/boton_op_preset", tamPreset));
        btnPresets_[static_cast<std::size_t>(i)] = botones_.back().get();
        btnPresets_[static_cast<std::size_t>(i)]->setColorTinte(sf::Color(145, 225, 215));
    }

    // --- Fila 3: Duración de estación ([-]  10 s  [+]) ---
    botones_.push_back(std::make_unique<Boton>(app.recursos(), "-", sf::Vector2f(cx - 240.0f, 444.0f),
        [this] {
            Ajustes& a = app_.ajustes();
            if (a.duracionEstacion > 5.0f) a.duracionEstacion -= 5.0f;
            refrescar();
        }, "ui/boton_op_mini", tamMini));
    botones_.push_back(std::make_unique<Boton>(app.recursos(), "+", sf::Vector2f(cx + 240.0f, 444.0f),
        [this] {
            Ajustes& a = app_.ajustes();
            if (a.duracionEstacion < 60.0f) a.duracionEstacion += 5.0f;
            refrescar();
        }, "ui/boton_op_mini", tamMini));

    // --- Volver ---
    botones_.push_back(std::make_unique<Boton>(app.recursos(), "Volver", sf::Vector2f(cx, 622.0f),
        [this] {
            aplicarTextoEntrada();
            app_.irA(EstadoApp::Menu);
        }, "ui/boton_menu_principal", tamVolver));

    refrescar();
}

void PantallaOpciones::establecerBichitos(int valor)
{
    const int limitado = std::max(MIN_BICHITOS, std::min(valor, MAX_BICHITOS));
    app_.ajustes().bichitosIniciales = limitado;
    textoEntrada_ = std::to_string(limitado);
    refrescar();
}

void PantallaOpciones::aplicarTextoEntrada()
{
    if (textoEntrada_.empty())
    {
        establecerBichitos(MIN_BICHITOS);
        return;
    }

    int valor = std::stoi(textoEntrada_);
    establecerBichitos(valor);
}

void PantallaOpciones::refrescar()
{
    const Ajustes& a = app_.ajustes();
    const bool parpadeoCursor = (std::fmod(tiempo_, 1.0f) < 0.55f);
    const std::string mostrado = textoEntrada_.empty() ? "" : textoEntrada_;
    etValorBichitos_.setTexto(mostrado + (parpadeoCursor ? "|" : " "));
    etEstacion_.setTexto("Duración de estación: " + std::to_string(static_cast<int>(a.duracionEstacion)) + " s");

    for (int i = 0; i < 5; ++i)
    {
        if (btnPresets_[static_cast<std::size_t>(i)] != nullptr)
        {
            btnPresets_[static_cast<std::size_t>(i)]->setSeleccionado(a.bichitosIniciales == PRESETS[i]);
        }
    }
}

void PantallaOpciones::procesarEvento(const sf::Event& evento)
{
    for (auto& b : botones_)
        b->procesarEvento(evento);

    // Entrada directa por teclado para escribir el número de bichitos (0-9 y Backspace)
    if (evento.type == sf::Event::TextEntered)
    {
        const sf::Uint32 ch = evento.text.unicode;
        if (ch >= '0' && ch <= '9')
        {
            if (textoEntrada_.size() < 3)
            {
                if (textoEntrada_ == "0")
                    textoEntrada_.clear();
                textoEntrada_.push_back(static_cast<char>(ch));
                int val = std::stoi(textoEntrada_);
                if (val > MAX_BICHITOS)
                {
                    val = MAX_BICHITOS;
                    textoEntrada_ = std::to_string(val);
                }
                if (val >= MIN_BICHITOS)
                    app_.ajustes().bichitosIniciales = val;
                refrescar();
            }
        }
        else if (ch == 8) // Backspace
        {
            if (!textoEntrada_.empty())
            {
                textoEntrada_.pop_back();
                if (!textoEntrada_.empty())
                {
                    int val = std::max(MIN_BICHITOS, std::stoi(textoEntrada_));
                    app_.ajustes().bichitosIniciales = val;
                }
                refrescar();
            }
        }
    }

    if (evento.type == sf::Event::KeyPressed)
    {
        if (evento.key.code == sf::Keyboard::Return)
        {
            aplicarTextoEntrada();
        }
        else if (evento.key.code == sf::Keyboard::Escape)
        {
            aplicarTextoEntrada();
            app_.irA(EstadoApp::Menu);
        }
    }
}

void PantallaOpciones::actualizar(float dt)
{
    tiempo_ += dt;
    refrescar();
}

void PantallaOpciones::dibujar(sf::RenderTarget& destino)
{
    const float ancho = static_cast<float>(Config::ANCHO_VENTANA);
    const float alto = static_cast<float>(Config::ALTO_VENTANA);
    const float cx = ancho / 2.0f;

    // 1. Fondo con gradiente nocturno a juego con Menú y Créditos
    sf::VertexArray fondo(sf::Quads, 4);
    fondo[0] = sf::Vertex(sf::Vector2f(0.0f, 0.0f), sf::Color(11, 18, 34));
    fondo[1] = sf::Vertex(sf::Vector2f(ancho, 0.0f), sf::Color(15, 24, 46));
    fondo[2] = sf::Vertex(sf::Vector2f(ancho, alto), sf::Color(16, 46, 56));
    fondo[3] = sf::Vertex(sf::Vector2f(0.0f, alto), sf::Color(12, 36, 48));
    destino.draw(fondo);

    // 2. Tarjeta central estilizada
    const sf::Vector2f tamTarjeta(760.0f, 405.0f);
    sf::RectangleShape tarjeta(tamTarjeta);
    tarjeta.setOrigin(tamTarjeta.x / 2.0f, tamTarjeta.y / 2.0f);
    tarjeta.setPosition(cx, 354.0f);
    tarjeta.setFillColor(sf::Color(15, 24, 40, 205));
    tarjeta.setOutlineThickness(2.5f);
    tarjeta.setOutlineColor(sf::Color(75, 205, 195, 150));
    destino.draw(tarjeta);

    // 3. Recuadro editable del número de bichitos
    sf::RectangleShape caja(sf::Vector2f(cajaEntrada_.width, cajaEntrada_.height));
    caja.setPosition(cajaEntrada_.left, cajaEntrada_.top);
    caja.setFillColor(sf::Color(8, 14, 26, 235));
    caja.setOutlineThickness(2.5f);
    caja.setOutlineColor(sf::Color(95, 235, 205, 220));
    destino.draw(caja);

    // Separador sutil entre sección de bichitos y sección de estación
    sf::RectangleShape sep(sf::Vector2f(620.0f, 2.0f));
    sep.setOrigin(310.0f, 1.0f);
    sep.setPosition(cx, 396.0f);
    sep.setFillColor(sf::Color(80, 150, 175, 85));
    destino.draw(sep);

    // 4. Etiquetas y botones
    titulo_.dibujar(destino);
    etBichitosTitulo_.dibujar(destino);
    etValorBichitos_.dibujar(destino);
    etPresets_.dibujar(destino);
    etEstacion_.dibujar(destino);
    nota_.dibujar(destino);
    for (const auto& b : botones_)
        b->dibujar(destino);
}
