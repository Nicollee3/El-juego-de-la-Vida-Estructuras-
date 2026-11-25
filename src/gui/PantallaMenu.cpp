#include "gui/PantallaMenu.h"
#include "gui/Aplicacion.h"
#include "gui/Config.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace
{
    float snap(float v, float paso)
    {
        return std::floor(v / paso) * paso;
    }

    void agregarQuad(sf::VertexArray& va, float x, float y, float w, float h, sf::Color c)
    {
        va.append(sf::Vertex(sf::Vector2f(x, y), c));
        va.append(sf::Vertex(sf::Vector2f(x + w, y), c));
        va.append(sf::Vertex(sf::Vector2f(x + w, y + h), c));
        va.append(sf::Vertex(sf::Vector2f(x, y + h), c));
    }

    void dibujarMatrizPixel(sf::VertexArray& va, float cx, float cy, float px,
                            const char* const* filas, int nFilas, int nCols,
                            const sf::Color* paleta)
    {
        const float ox = snap(cx - (nCols * px) * 0.5f, px);
        const float oy = snap(cy - (nFilas * px) * 0.5f, px);
        for (int r = 0; r < nFilas; ++r)
        {
            for (int col = 0; col < nCols; ++col)
            {
                const char ch = filas[r][col];
                if (ch < '1' || ch > '7')
                    continue;
                agregarQuad(va, ox + col * px, oy + r * px, px, px, paleta[ch - '1']);
            }
        }
    }
}

PantallaMenu::PantallaMenu(Aplicacion& app)
    : Pantalla(app),
      titulo_(app.recursos(), "EL JUEGO DE LA VIDA", 56,
              sf::Vector2f(Config::ANCHO_VENTANA / 2.0f, 118.0f), Etiqueta::Alineacion::Centro),
      subtitulo_(app.recursos(), "Misión Orbital  ·  Exoplaneta  ·  Análisis de Bichitos", 18,
                 sf::Vector2f(Config::ANCHO_VENTANA / 2.0f, 174.0f), Etiqueta::Alineacion::Centro),
      ayuda_(app.recursos(), "Mouse o Flechas + Enter para navegar", 15,
             sf::Vector2f(Config::ANCHO_VENTANA / 2.0f, 686.0f), Etiqueta::Alineacion::Centro),
      tiempo_(0.0f),
      enfocado_(-1)
{
    titulo_.setColor(sf::Color(235, 250, 255));
    subtitulo_.setColor(sf::Color(115, 225, 200));
    ayuda_.setColor(sf::Color(165, 185, 205));

    const float cx = Config::ANCHO_VENTANA / 2.0f;
    const float y0 = 312.0f;
    const float paso = 78.0f;
    const sf::Vector2u tamBoton(84, 15); // 336x60 px

    botones_.push_back(std::make_unique<Boton>(app.recursos(), "Iniciar simulación",
        sf::Vector2f(cx, y0), [this] { app_.irA(EstadoApp::Simulacion); },
        "ui/boton_menu_principal", tamBoton));
    botones_.push_back(std::make_unique<Boton>(app.recursos(), "Opciones",
        sf::Vector2f(cx, y0 + paso), [this] { app_.irA(EstadoApp::Opciones); },
        "ui/boton_menu_principal", tamBoton));
    botones_.push_back(std::make_unique<Boton>(app.recursos(), "Créditos",
        sf::Vector2f(cx, y0 + 2 * paso), [this] { app_.irA(EstadoApp::Creditos); },
        "ui/boton_menu_principal", tamBoton));
    botones_.push_back(std::make_unique<Boton>(app.recursos(), "Salir",
        sf::Vector2f(cx, y0 + 3 * paso), [this] { app_.irA(EstadoApp::Salir); },
        "ui/boton_menu_principal", tamBoton));

    inicializarFondoAnimado();
}

void PantallaMenu::inicializarFondoAnimado()
{
    particulas_.clear();
    const float ancho = static_cast<float>(Config::ANCHO_VENTANA);
    const float alto = static_cast<float>(Config::ALTO_VENTANA);

    // Estrellas pixel art en el espacio profundo
    const sf::Color paletaEstrellas[] = {
        sf::Color(235, 248, 255),
        sf::Color(145, 225, 255),
        sf::Color(190, 175, 255),
        sf::Color(255, 238, 165)
    };

    for (int i = 0; i < 65; ++i)
    {
        ParticulaFondo p;
        p.x = static_cast<float>(std::rand() % static_cast<int>(ancho));
        p.y = static_cast<float>(std::rand() % static_cast<int>(alto));
        p.vx = -4.0f - (static_cast<float>(std::rand()) / RAND_MAX) * 8.0f;
        p.vy = 0.0f;
        p.radio = (i % 5 == 0) ? 4.0f : 3.0f;
        p.fase = (static_cast<float>(std::rand()) / RAND_MAX) * 6.28318f;
        p.color = paletaEstrellas[i % 4];
        p.esComida = (i % 5 == 0); // estrellas en cruz brillante
        particulas_.push_back(p);
    }
}

void PantallaMenu::moverFoco(int delta)
{
    const int n = static_cast<int>(botones_.size());
    if (enfocado_ < 0)
        enfocado_ = (delta > 0) ? 0 : n - 1;
    else
        enfocado_ = (enfocado_ + delta + n) % n;
    actualizarFoco();
}

void PantallaMenu::actualizarFoco()
{
    for (int i = 0; i < static_cast<int>(botones_.size()); ++i)
        botones_[i]->setEnfocado(i == enfocado_);
}

void PantallaMenu::procesarEvento(const sf::Event& evento)
{
    if (evento.type == sf::Event::MouseMoved && enfocado_ != -1)
    {
        enfocado_ = -1;
        actualizarFoco();
    }

    for (auto& b : botones_)
        b->procesarEvento(evento);

    if (evento.type == sf::Event::KeyPressed)
    {
        if (evento.key.code == sf::Keyboard::Down)
            moverFoco(+1);
        else if (evento.key.code == sf::Keyboard::Up)
            moverFoco(-1);
        else if ((evento.key.code == sf::Keyboard::Return || evento.key.code == sf::Keyboard::Space) &&
                 enfocado_ >= 0)
            botones_[enfocado_]->activar();
    }
}

void PantallaMenu::actualizar(float dt)
{
    tiempo_ += dt;

    const float ancho = static_cast<float>(Config::ANCHO_VENTANA);
    for (auto& p : particulas_)
    {
        p.x += p.vx * dt;
        if (p.x < -8.0f)
            p.x = ancho + 8.0f;
    }

    // Movimiento flotante sutil del título (en pasos pixelados)
    const float offsetY = snap(std::sin(tiempo_ * 2.0f) * 4.5f, 2.0f);
    titulo_.setPosicion(sf::Vector2f(ancho / 2.0f, 114.0f + offsetY));
}

void PantallaMenu::dibujar(sf::RenderTarget& destino)
{
    const float ancho = static_cast<float>(Config::ANCHO_VENTANA);
    const float alto = static_cast<float>(Config::ALTO_VENTANA);
    const float cx = ancho / 2.0f;

    // 1. Fondo espacial profundo con gradiente nocturno
    sf::VertexArray fondo(sf::Quads, 4);
    fondo[0] = sf::Vertex(sf::Vector2f(0.0f, 0.0f), sf::Color(7, 11, 24));
    fondo[1] = sf::Vertex(sf::Vector2f(ancho, 0.0f), sf::Color(12, 18, 38));
    fondo[2] = sf::Vertex(sf::Vector2f(ancho, alto), sf::Color(11, 34, 46));
    fondo[3] = sf::Vertex(sf::Vector2f(0.0f, alto), sf::Color(8, 24, 38));
    destino.draw(fondo);

    // 2. Cuadrícula sutil tipo escáner orbital
    sf::VertexArray grilla(sf::Lines);
    const sf::Color colorGrilla(85, 195, 215, 14);
    for (float x = 0.0f; x <= ancho; x += 64.0f)
    {
        grilla.append(sf::Vertex(sf::Vector2f(x, 0.0f), colorGrilla));
        grilla.append(sf::Vertex(sf::Vector2f(x, alto), colorGrilla));
    }
    for (float y = 0.0f; y <= alto; y += 64.0f)
    {
        grilla.append(sf::Vertex(sf::Vector2f(0.0f, y), colorGrilla));
        grilla.append(sf::Vertex(sf::Vector2f(ancho, y), colorGrilla));
    }
    destino.draw(grilla);

    sf::VertexArray pixelArt(sf::Quads);

    // 3. Estrellas pixel art titilando en el espacio
    for (const auto& p : particulas_)
    {
        const float brillo = 0.35f + 0.65f * (0.5f + 0.5f * std::sin(tiempo_ * 2.8f + p.fase));
        const sf::Uint8 a = static_cast<sf::Uint8>(230.0f * brillo);
        const float px = 4.0f;
        const float sx = snap(p.x, px);
        const float sy = snap(p.y, px);

        if (p.esComida && brillo > 0.72f)
        {
            const sf::Color cBr(p.color.r, p.color.g, p.color.b, a);
            const sf::Color cSuave(p.color.r, p.color.g, p.color.b, static_cast<sf::Uint8>(a / 2));
            agregarQuad(pixelArt, sx, sy, px, px, cBr);
            agregarQuad(pixelArt, sx - px, sy, px, px, cSuave);
            agregarQuad(pixelArt, sx + px, sy, px, px, cSuave);
            agregarQuad(pixelArt, sx, sy - px, px, px, cSuave);
            agregarQuad(pixelArt, sx, sy + px, px, px, cSuave);
        }
        else
        {
            agregarQuad(pixelArt, sx, sy, px, px, sf::Color(p.color.r, p.color.g, p.color.b, a));
        }
    }

    // Estrella fugaz pixel art cruzando cada ~6 segundos
    const float cicloFugaz = std::fmod(tiempo_, 6.5f);
    if (cicloFugaz < 1.1f)
    {
        const float f = cicloFugaz / 1.1f;
        const float fx = snap(1150.0f - f * 520.0f, 4.0f);
        const float fy = snap(40.0f + f * 180.0f, 4.0f);
        for (int s = 0; s < 6; ++s)
        {
            const sf::Uint8 aF = static_cast<sf::Uint8>(std::max(0, 220 - s * 36));
            agregarQuad(pixelArt, fx + s * 8.0f, fy - s * 4.0f, 4.0f, 4.0f, sf::Color(210, 245, 255, aF));
        }
    }

    // 4. Exoplaneta alienígena 100% pixel art a la izquierda
    static const char* const EXOPLANETA[16] = {
        ".....111111.....",
        "...1122223311...",
        "..122233332221..",
        ".12233344322211.",
        ".12233443222111.",
        "1222222222233111",
        "1223322222334311",
        "1233432222333211",
        "1223322222222111",
        "1122222222221111",
        ".11222233221111.",
        ".11122334311111.",
        "..111223321111..",
        "...1111111111...",
        ".....111111.....",
        "................"
    };
    const sf::Color palPlaneta[4] = {
        sf::Color(20, 74, 96, 235),   // 1 océano profundo
        sf::Color(34, 128, 146, 235), // 2 océano claro
        sf::Color(52, 198, 152, 235), // 3 continente alienígena esmeralda
        sf::Color(135, 250, 195, 240) // 4 zona bioluminiscente
    };

    const float planX = 188.0f;
    const float planY = 432.0f + snap(std::sin(tiempo_ * 1.1f) * 6.0f, 4.0f);

    // Anillo trasero pixelado del planeta
    for (int i = -11; i <= 11; ++i)
    {
        const float rx = snap(planX + static_cast<float>(i) * 12.0f, 4.0f);
        const float ry = snap(planY - 24.0f - static_cast<float>(i) * 4.0f, 4.0f);
        agregarQuad(pixelArt, rx, ry, 12.0f, 8.0f, sf::Color(115, 220, 235, 95));
    }

    dibujarMatrizPixel(pixelArt, planX, planY, 12.0f, EXOPLANETA, 16, 16, palPlaneta);

    // Anillo frontal pixelado del planeta
    for (int i = -11; i <= 11; ++i)
    {
        if (i >= -6 && i <= 6)
        {
            const float rx = snap(planX + static_cast<float>(i) * 12.0f, 4.0f);
            const float ry = snap(planY + 26.0f - static_cast<float>(i) * 4.0f, 4.0f);
            agregarQuad(pixelArt, rx, ry, 12.0f, 8.0f, sf::Color(145, 235, 245, 165));
        }
    }

    // 5. Nave espacial de exploración 100% pixel art a la derecha + haz de escaneo
    static const char* const NAVE_A[12] = {
        "......11......",
        ".....1221.....",
        "....123321....",
        "....123321....",
        "...11222211...",
        "..1412222141..",
        ".144122221441.",
        "14411222211441",
        "141..1111..141",
        ".....5665.....",
        "......55......",
        ".............."
    };
    static const char* const NAVE_B[12] = {
        "......11......",
        ".....1221.....",
        "....123321....",
        "....123321....",
        "...11222211...",
        "..1412222141..",
        ".144122221441.",
        "14411222211441",
        "141..1111..141",
        "....566665....",
        ".....5665.....",
        "......55......"
    };
    const sf::Color palNave[6] = {
        sf::Color(150, 175, 205, 245), // 1 casco gris azulado
        sf::Color(220, 236, 250, 245), // 2 fuselaje blanco
        sf::Color(80, 235, 245, 245),  // 3 cabina cian luminosa
        sf::Color(70, 115, 175, 245),  // 4 alas azules
        sf::Color(255, 130, 40, 240),  // 5 llama externa
        sf::Color(255, 238, 120, 245)  // 6 núcleo propulsor
    };

    const float naveX = 1092.0f;
    const float naveY = 344.0f + snap(std::sin(tiempo_ * 2.2f) * 10.0f, 4.0f);
    const bool frameProp = (static_cast<int>(tiempo_ * 8.0f) % 2) == 1;

    // Haz de escaneo cian pixelado debajo de la nave + mini bichitos escaneados
    for (int fila = 0; fila < 18; ++fila)
    {
        const float anchoHaz = 28.0f + static_cast<float>(fila) * 10.0f;
        const float hy = snap(naveY + 60.0f + static_cast<float>(fila) * 12.0f, 4.0f);
        const sf::Uint8 aHaz = static_cast<sf::Uint8>(std::max(10, 58 - fila * 3));
        agregarQuad(pixelArt, snap(naveX - anchoHaz * 0.5f, 4.0f), hy, anchoHaz, 8.0f,
                    sf::Color(75, 235, 225, aHaz));
    }

    // Mini bichito pixel art flotando dentro del haz de escaneo
    static const char* const MINI_BICHITO[5] = {
        ".111.",
        "11111",
        "12121",
        "11111",
        ".1.1."
    };
    const sf::Color palBichito[2] = {
        sf::Color(70, 205, 255, 230),
        sf::Color(15, 25, 45, 235)
    };
    const float bxScan = naveX + snap(std::sin(tiempo_ * 2.4f) * 18.0f, 4.0f);
    const float byScan = naveY + 178.0f + snap(std::cos(tiempo_ * 2.0f) * 10.0f, 4.0f);
    dibujarMatrizPixel(pixelArt, bxScan, byScan, 4.0f, MINI_BICHITO, 5, 5, palBichito);

    dibujarMatrizPixel(pixelArt, naveX, naveY, 8.0f, frameProp ? NAVE_B : NAVE_A, 12, 14, palNave);

    destino.draw(pixelArt);

    // 6. Línea decorativa luminosa bajo el subtítulo (misma posición principal)
    sf::RectangleShape linea(sf::Vector2f(420.0f, 4.0f));
    linea.setOrigin(210.0f, 2.0f);
    linea.setPosition(cx, 206.0f);
    linea.setFillColor(sf::Color(80, 220, 195, 175));
    destino.draw(linea);

    // 7. Tarjeta central semitransparente para los botones (misma posición principal)
    const sf::Vector2f tamTarjeta(430.0f, 386.0f);
    sf::RectangleShape sombraTarjeta(tamTarjeta);
    sombraTarjeta.setOrigin(tamTarjeta.x / 2.0f, tamTarjeta.y / 2.0f);
    sombraTarjeta.setPosition(cx + 8.0f, 434.0f);
    sombraTarjeta.setFillColor(sf::Color(0, 0, 0, 105));
    destino.draw(sombraTarjeta);

    sf::RectangleShape tarjeta(tamTarjeta);
    tarjeta.setOrigin(tamTarjeta.x / 2.0f, tamTarjeta.y / 2.0f);
    tarjeta.setPosition(cx, 426.0f);
    tarjeta.setFillColor(sf::Color(13, 21, 36, 215));
    tarjeta.setOutlineThickness(3.0f);
    tarjeta.setOutlineColor(sf::Color(75, 205, 195, 165));
    destino.draw(tarjeta);

    // Esquinas pixel art tipo visor de nave en la tarjeta central
    sf::VertexArray visor(sf::Quads);
    const float leftT = cx - tamTarjeta.x * 0.5f;
    const float rightT = cx + tamTarjeta.x * 0.5f;
    const float topT = 426.0f - tamTarjeta.y * 0.5f;
    const float botT = 426.0f + tamTarjeta.y * 0.5f;
    const sf::Color cEsquina(110, 245, 220, 235);
    // Superior izquierda / derecha
    agregarQuad(visor, leftT - 4.0f, topT - 4.0f, 20.0f, 4.0f, cEsquina);
    agregarQuad(visor, leftT - 4.0f, topT - 4.0f, 4.0f, 20.0f, cEsquina);
    agregarQuad(visor, rightT - 16.0f, topT - 4.0f, 20.0f, 4.0f, cEsquina);
    agregarQuad(visor, rightT, topT - 4.0f, 4.0f, 20.0f, cEsquina);
    // Inferior izquierda / derecha
    agregarQuad(visor, leftT - 4.0f, botT, 20.0f, 4.0f, cEsquina);
    agregarQuad(visor, leftT - 4.0f, botT - 16.0f, 4.0f, 20.0f, cEsquina);
    agregarQuad(visor, rightT - 16.0f, botT, 20.0f, 4.0f, cEsquina);
    agregarQuad(visor, rightT, botT - 16.0f, 4.0f, 20.0f, cEsquina);

    // Luz parpadeante de estado de escáner en la esquina superior izquierda de la tarjeta
    const bool luzOn = (static_cast<int>(tiempo_ * 2.5f) % 2) == 0;
    agregarQuad(visor, leftT + 14.0f, topT + 12.0f, 8.0f, 8.0f,
                luzOn ? sf::Color(90, 250, 175, 245) : sf::Color(35, 115, 85, 190));
    destino.draw(visor);

    // 8. Resplandor suave detrás del botón principal ("Iniciar simulación")
    if (!botones_.empty())
    {
        const sf::FloatRect b0 = botones_[0]->getLimites();
        const float pulsoGlow = 0.7f + 0.3f * std::sin(tiempo_ * 3.0f);
        sf::RectangleShape glow(sf::Vector2f(b0.width + 14.0f, b0.height + 14.0f));
        glow.setOrigin(glow.getSize().x / 2.0f, glow.getSize().y / 2.0f);
        glow.setPosition(b0.left + b0.width / 2.0f, b0.top + b0.height / 2.0f);
        glow.setFillColor(sf::Color(60, 220, 185, static_cast<sf::Uint8>(45.0f * pulsoGlow)));
        glow.setOutlineThickness(2.0f);
        glow.setOutlineColor(sf::Color(90, 240, 205, static_cast<sf::Uint8>(160.0f * pulsoGlow)));
        destino.draw(glow);
    }

    // 9. Textos y botones (misma estructura y posiciones principales)
    titulo_.dibujar(destino);
    subtitulo_.dibujar(destino);
    for (const auto& b : botones_)
        b->dibujar(destino);
    ayuda_.dibujar(destino);
}
