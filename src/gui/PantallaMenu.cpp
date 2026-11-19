#include "gui/PantallaMenu.h"
#include "gui/Aplicacion.h"
#include "gui/Config.h"
#include <cmath>
#include <cstdlib>

namespace
{
    sf::String utf8(const std::string& texto)
    {
        return sf::String::fromUtf8(texto.begin(), texto.end());
    }
}

PantallaMenu::PantallaMenu(Aplicacion& app)
    : Pantalla(app),
      titulo_(app.recursos(), "EL JUEGO DE LA VIDA", 56,
              sf::Vector2f(Config::ANCHO_VENTANA / 2.0f, 118.0f), Etiqueta::Alineacion::Centro),
      subtitulo_(app.recursos(), "Ecosistema  ·  Genética  ·  Desastres Naturales", 18,
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

    // Bichitos luminosos flotando en el fondo
    const sf::Color paletaBichitos[] = {
        sf::Color(65, 205, 255),
        sf::Color(85, 235, 185),
        sf::Color(120, 175, 255),
        sf::Color(170, 130, 255)
    };

    for (int i = 0; i < 26; ++i)
    {
        ParticulaFondo p;
        p.x = static_cast<float>(std::rand() % static_cast<int>(ancho));
        p.y = static_cast<float>(std::rand() % static_cast<int>(alto));
        const float ang = (static_cast<float>(std::rand()) / RAND_MAX) * 6.28318f;
        const float vel = 14.0f + (static_cast<float>(std::rand()) / RAND_MAX) * 26.0f;
        p.vx = std::cos(ang) * vel;
        p.vy = std::sin(ang) * vel;
        p.radio = 5.0f + (static_cast<float>(std::rand()) / RAND_MAX) * 7.0f;
        p.fase = (static_cast<float>(std::rand()) / RAND_MAX) * 6.28318f;
        p.color = paletaBichitos[i % 4];
        p.esComida = false;
        particulas_.push_back(p);
    }

    // Partículas de comida doradas/verdes flotando lentamente
    for (int i = 0; i < 18; ++i)
    {
        ParticulaFondo p;
        p.x = static_cast<float>(std::rand() % static_cast<int>(ancho));
        p.y = static_cast<float>(std::rand() % static_cast<int>(alto));
        p.vx = -6.0f + (static_cast<float>(std::rand()) / RAND_MAX) * 12.0f;
        p.vy = -8.0f - (static_cast<float>(std::rand()) / RAND_MAX) * 10.0f;
        p.radio = 3.0f + (static_cast<float>(std::rand()) / RAND_MAX) * 2.5f;
        p.fase = (static_cast<float>(std::rand()) / RAND_MAX) * 6.28318f;
        p.color = (i % 3 == 0) ? sf::Color(255, 215, 75) : sf::Color(100, 225, 120);
        p.esComida = true;
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
    const float alto = static_cast<float>(Config::ALTO_VENTANA);

    for (auto& p : particulas_)
    {
        p.x += p.vx * dt;
        p.y += p.vy * dt;

        if (p.esComida)
        {
            if (p.y < -15.0f) p.y = alto + 15.0f;
            if (p.x < -15.0f) p.x = ancho + 15.0f;
            if (p.x > ancho + 15.0f) p.x = -15.0f;
        }
        else
        {
            if (p.x < 10.0f || p.x > ancho - 10.0f) p.vx = -p.vx;
            if (p.y < 10.0f || p.y > alto - 10.0f) p.vy = -p.vy;
        }
    }

    // Movimiento flotante sutil del título
    const float offsetY = std::sin(tiempo_ * 2.0f) * 4.5f;
    titulo_.setPosicion(sf::Vector2f(ancho / 2.0f, 114.0f + offsetY));
}

void PantallaMenu::dibujar(sf::RenderTarget& destino)
{
    const float ancho = static_cast<float>(Config::ANCHO_VENTANA);
    const float alto = static_cast<float>(Config::ALTO_VENTANA);
    const float cx = ancho / 2.0f;

    // 1. Fondo con gradiente nocturno bioluminiscente
    sf::VertexArray fondo(sf::Quads, 4);
    fondo[0] = sf::Vertex(sf::Vector2f(0.0f, 0.0f), sf::Color(11, 18, 34));
    fondo[1] = sf::Vertex(sf::Vector2f(ancho, 0.0f), sf::Color(15, 24, 46));
    fondo[2] = sf::Vertex(sf::Vector2f(ancho, alto), sf::Color(16, 46, 56));
    fondo[3] = sf::Vertex(sf::Vector2f(0.0f, alto), sf::Color(12, 36, 48));
    destino.draw(fondo);

    // 2. Cuadrícula sutil de laboratorio
    sf::VertexArray grilla(sf::Lines);
    const sf::Color colorGrilla(100, 190, 210, 16);
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

    // 3. Ecosistema vivo en el fondo (bichitos con halo + partículas de comida)
    for (const auto& p : particulas_)
    {
        const float pulso = 0.85f + 0.15f * std::sin(tiempo_ * 2.5f + p.fase);

        if (!p.esComida)
        {
            sf::CircleShape halo(p.radio * 2.4f * pulso);
            halo.setOrigin(halo.getRadius(), halo.getRadius());
            halo.setPosition(p.x, p.y);
            halo.setFillColor(sf::Color(p.color.r, p.color.g, p.color.b, 26));
            destino.draw(halo);
        }

        sf::CircleShape cuerpo(p.radio * pulso);
        cuerpo.setOrigin(cuerpo.getRadius(), cuerpo.getRadius());
        cuerpo.setPosition(p.x, p.y);
        cuerpo.setFillColor(sf::Color(p.color.r, p.color.g, p.color.b, p.esComida ? 150 : 120));
        destino.draw(cuerpo);
    }

    // 4. Línea decorativa luminosa bajo el subtítulo
    sf::RectangleShape linea(sf::Vector2f(420.0f, 3.0f));
    linea.setOrigin(210.0f, 1.5f);
    linea.setPosition(cx, 206.0f);
    linea.setFillColor(sf::Color(80, 220, 195, 175));
    destino.draw(linea);

    // 5. Tarjeta central semitransparente para los botones
    const sf::Vector2f tamTarjeta(430.0f, 386.0f);
    sf::RectangleShape sombraTarjeta(tamTarjeta);
    sombraTarjeta.setOrigin(tamTarjeta.x / 2.0f, tamTarjeta.y / 2.0f);
    sombraTarjeta.setPosition(cx + 8.0f, 434.0f);
    sombraTarjeta.setFillColor(sf::Color(0, 0, 0, 95));
    destino.draw(sombraTarjeta);

    sf::RectangleShape tarjeta(tamTarjeta);
    tarjeta.setOrigin(tamTarjeta.x / 2.0f, tamTarjeta.y / 2.0f);
    tarjeta.setPosition(cx, 426.0f);
    tarjeta.setFillColor(sf::Color(15, 24, 40, 205));
    tarjeta.setOutlineThickness(2.5f);
    tarjeta.setOutlineColor(sf::Color(75, 205, 195, 150));
    destino.draw(tarjeta);

    // 6. Resplandor suave detrás del botón principal ("Iniciar simulación")
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

    // 7. Textos y botones
    titulo_.dibujar(destino);
    subtitulo_.dibujar(destino);
    for (const auto& b : botones_)
        b->dibujar(destino);
    ayuda_.dibujar(destino);
}
