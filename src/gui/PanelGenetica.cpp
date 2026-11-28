#include "gui/PanelGenetica.h"
#include "gui/Config.h"
#include "gui/GestorRecursos.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <string>

namespace
{
    constexpr float ALTURA_CABECERA_ARBOL = 60.0f;

    sf::Vector2i posicionMouse(const sf::RenderTarget& destino)
    {
        const sf::RenderWindow* ventana =
            dynamic_cast<const sf::RenderWindow*>(&destino);
        return ventana != nullptr
            ? sf::Mouse::getPosition(*ventana)
            : sf::Vector2i(-1000000, -1000000);
    }

    const char* nombreGen(GenEstatico gen)
    {
        switch (gen)
        {
        case GenEstatico::INMUNIDAD: return "Inmunidad";
        case GenEstatico::CANIBALISMO: return "Canibalismo";
        case GenEstatico::VAMPIRISMO: return "Vampirismo";
        case GenEstatico::TELETRANSPORTARSE: return "Teletransporte";
        case GenEstatico::INMORTAL: return "Inmortal";
        case GenEstatico::MITOSIS: return "Mitosis";
        case GenEstatico::NINGUNO: default: return "Ninguno";
        }
    }

    const char* logoGen(GenEstatico gen)
    {
        switch (gen)
        {
        case GenEstatico::INMUNIDAD: return "I";
        case GenEstatico::CANIBALISMO: return "C";
        case GenEstatico::VAMPIRISMO: return "V";
        case GenEstatico::TELETRANSPORTARSE: return "T";
        case GenEstatico::INMORTAL: return "M";
        case GenEstatico::MITOSIS: return "D";
        case GenEstatico::NINGUNO: default: return "-";
        }
    }

    sf::Color colorGen(GenEstatico gen)
    {
        switch (gen)
        {
        case GenEstatico::INMUNIDAD: return sf::Color(120, 205, 135);
        case GenEstatico::CANIBALISMO: return sf::Color(32, 110, 165);
        case GenEstatico::VAMPIRISMO: return sf::Color(175, 65, 110);
        case GenEstatico::TELETRANSPORTARSE: return sf::Color(70, 195, 205);
        case GenEstatico::INMORTAL: return sf::Color(185, 140, 235);
        case GenEstatico::MITOSIS: return sf::Color(235, 165, 75);
        case GenEstatico::NINGUNO: default: return sf::Color(90, 100, 115);
        }
    }

    sf::Text texto(const std::string& str, const sf::Font& fuente, unsigned tam,
                   sf::Color color, float x, float y)
    {
        sf::Text salida(sf::String::fromUtf8(str.begin(), str.end()), fuente, tam);
        salida.setFillColor(color);
        salida.setPosition(x, y);
        return salida;
    }

    int hojas(NodoGenealogico* nodo)
    {
        return nodo != nullptr ? nodo->hojas : 0;
    }

}

PanelGenetica::PanelGenetica()
    : visible(false), arbolActivo(false), idSeleccionado(-1), generacion(1),
      vivo(true), vida(0.0f), edad(0.0f), genoma(), estado(EstadoBichito::DePaseo),
      genMostrado(GenEstatico::NINGUNO), indiceFundador(0), zoom(1.0f),
      desplazamiento(0.0f, 0.0f), arrastrando(false), ultimoMouse(0.0f, 0.0f) {}

void PanelGenetica::seleccionar(const Bichito& bichito)
{
    visible = true;
    idSeleccionado = bichito.getId();
    generacion = bichito.getGeneracion();
    vivo = true;
    vida = bichito.getVida();
    edad = bichito.getEdadActual();
    genoma = bichito.getGenetica();
    estado = bichito.getEstado();
    arbolActivo = false;
    genMostrado = GenEstatico::NINGUNO;
    indiceFundador = 0;
}

void PanelGenetica::actualizarSeleccion(const Bichito& bichito)
{
    if (!visible || idSeleccionado != bichito.getId() || !vivo)
        return;
    generacion = bichito.getGeneracion();
    vida = bichito.getVida();
    edad = bichito.getEdadActual();
    genoma = bichito.getGenetica();
    estado = bichito.getEstado();
}

void PanelGenetica::marcarFallecido(int id)
{
    if (visible && idSeleccionado == id)
    {
        vivo = false;
        vida = 0.0f;
    }
}

bool PanelGenetica::abrirArbol(GenEstatico gen, RegistroGenetico& registro)
{
    if (gen == GenEstatico::NINGUNO || registro.arbol(gen).cantidadFundadores() == 0)
        return false;

    visible = false;
    arbolActivo = true;
    genMostrado = gen;
    indiceFundador = 0;
    zoom = 1.0f;
    desplazamiento = sf::Vector2f(0.0f, 0.0f);
    arrastrando = false;
    return true;
}

void PanelGenetica::cerrar()
{
    visible = false;
    arbolActivo = false;
    arrastrando = false;
}

bool PanelGenetica::cerrarArbol()
{
    if (!arbolActivo)
        return false;
    arbolActivo = false;
    arrastrando = false;
    return true;
}

bool PanelGenetica::procesarEvento(const sf::Event& evento, RegistroGenetico& registro)
{
    if (evento.type == sf::Event::KeyPressed && evento.key.code == sf::Keyboard::Escape)
        return cerrarArbol();

    if (evento.type == sf::Event::MouseButtonReleased &&
        evento.mouseButton.button == sf::Mouse::Left)
    {
        arrastrando = false;
        return arbolActivo;
    }

    if (arbolActivo && evento.type == sf::Event::MouseMoved && arrastrando)
    {
        const sf::Vector2f actual(static_cast<float>(evento.mouseMove.x),
                                  static_cast<float>(evento.mouseMove.y));
        desplazamiento += actual - ultimoMouse;
        ultimoMouse = actual;
        return true;
    }

    if (arbolActivo && evento.type == sf::Event::MouseWheelScrolled)
    {
        zoom = std::max(0.55f, std::min(zoom * (evento.mouseWheelScroll.delta > 0.0f ? 1.12f : 0.89f),
                                        2.4f));
        return true;
    }

    if (evento.type != sf::Event::MouseButtonPressed ||
        evento.mouseButton.button != sf::Mouse::Left)
        return false;

    const float x = static_cast<float>(evento.mouseButton.x);
    const float y = static_cast<float>(evento.mouseButton.y);
    if (arbolActivo)
    {
        if (x > Config::ANCHO_MUNDO - 48.0f && y < ALTURA_CABECERA_ARBOL)
        {
            arbolActivo = false;
            return true;
        }
        ArbolLinajeGen& arbol = registro.arbol(genMostrado);
        if (x > Config::ANCHO_MUNDO - 145.0f && x < Config::ANCHO_MUNDO - 92.0f &&
            y < ALTURA_CABECERA_ARBOL && arbol.cantidadFundadores() > 1)
        {
            indiceFundador = (indiceFundador + arbol.cantidadFundadores() - 1) %
                             arbol.cantidadFundadores();
            return true;
        }
        if (x > Config::ANCHO_MUNDO - 92.0f && x < Config::ANCHO_MUNDO - 48.0f &&
            y < ALTURA_CABECERA_ARBOL && arbol.cantidadFundadores() > 1)
        {
            indiceFundador = (indiceFundador + 1) % arbol.cantidadFundadores();
            return true;
        }
        arrastrando = true;
        ultimoMouse = sf::Vector2f(x, y);
        return true;
    }

    if (visible && x >= 14.0f && x <= 404.0f && y >= 14.0f && y <= 183.0f)
    {
        if (x >= 370.0f && y <= 43.0f)
        {
            cerrar();
            return true;
        }
        const GenEstatico genes[2] = {genoma.genEstatico1, genoma.genEstatico2};
        const float posicionesX[2] = {42.0f, 96.0f};
        for (int i = 0; i < 2; ++i)
        {
            const float dx = x - posicionesX[i];
            const float dy = y - 150.0f;
            if (genes[i] != GenEstatico::NINGUNO && dx * dx + dy * dy <= 19.0f * 19.0f)
            {
                genMostrado = genes[i];
                indiceFundador = 0;
                zoom = 1.0f;
                desplazamiento = sf::Vector2f(0.0f, 0.0f);
                arbolActivo = registro.arbol(genMostrado).cantidadFundadores() > 0;
                return true;
            }
        }
        return true;
    }
    return false;
}

void PanelGenetica::dibujar(sf::RenderTarget& destino, GestorRecursos& recursos,
                            RegistroGenetico& registro)
{
    if (visible)
        dibujarTarjeta(destino, recursos);
    if (arbolActivo)
        dibujarArbol(destino, recursos, registro);
}

void PanelGenetica::dibujarTarjeta(sf::RenderTarget& destino, GestorRecursos& recursos)
{
    const sf::Vector2f tam(390.0f, 169.0f);
    sf::RectangleShape fondo(tam);
    fondo.setPosition(14.0f, 14.0f);
    fondo.setFillColor(sf::Color(15, 20, 32, 238));
    fondo.setOutlineThickness(2.0f);
    fondo.setOutlineColor(sf::Color(95, 155, 195));
    destino.draw(fondo);

    const std::string estadoTexto = vivo
        ? (estado == EstadoBichito::BuscandoComida ? "Buscando comida" :
           estado == EstadoBichito::BuscandoPareja ? "Buscando pareja" :
           estado == EstadoBichito::Cazando ? "Cazando" : "De paseo")
        : "Fallecido";
    destino.draw(texto("Bichito #" + std::to_string(idSeleccionado) +
                       "   Generacion " + std::to_string(generacion),
                       recursos.fuente(), 15, sf::Color::White, 25.0f, 22.0f));
    sf::Text cerrarTxt = texto("[X]", recursos.fuente(), 15, sf::Color(255, 125, 125),
                               369.0f, 21.0f);
    destino.draw(cerrarTxt);
    destino.draw(texto("Estado: " + estadoTexto, recursos.fuente(), 12,
                       vivo ? sf::Color(155, 230, 170) : sf::Color(245, 145, 145),
                       25.0f, 48.0f));
    destino.draw(texto("Vida: " + std::to_string(static_cast<int>(vida)) + " / " +
                       std::to_string(static_cast<int>(genoma.vidaMax)) +
                       "      Edad: " + std::to_string(static_cast<int>(edad)) + " / " +
                       std::to_string(static_cast<int>(genoma.edadMax)) + " anios",
                       recursos.fuente(), 12, sf::Color(220, 228, 240), 25.0f, 70.0f));
    destino.draw(texto("Velocidad: " + std::to_string(static_cast<int>(genoma.velocidad)) +
                       "      Metabolismo: " + std::to_string(genoma.metabolismo),
                       recursos.fuente(), 12, sf::Color(220, 228, 240), 25.0f, 92.0f));

    const GenEstatico genes[2] = {genoma.genEstatico1, genoma.genEstatico2};
    const float posicionesX[2] = {42.0f, 96.0f};
    const sf::Vector2i mouse = posicionMouse(destino);
    int hover = -1;
    for (int i = 0; i < 2; ++i)
    {
        sf::CircleShape logo(17.0f);
        logo.setOrigin(17.0f, 17.0f);
        logo.setPosition(posicionesX[i], 150.0f);
        logo.setFillColor(colorGen(genes[i]));
        logo.setOutlineThickness(genes[i] == GenEstatico::NINGUNO ? 1.0f : 2.0f);
        logo.setOutlineColor(sf::Color(220, 230, 245));
        destino.draw(logo);
        sf::Text marca = texto(logoGen(genes[i]), recursos.fuente(), 14,
                               sf::Color::White, posicionesX[i] - 5.0f, 140.0f);
        destino.draw(marca);
        const float dx = static_cast<float>(mouse.x) - posicionesX[i];
        const float dy = static_cast<float>(mouse.y) - 150.0f;
        if (genes[i] != GenEstatico::NINGUNO && dx * dx + dy * dy <= 19.0f * 19.0f)
            hover = i;
    }
    if (hover >= 0)
    {
        const GenEstatico gen = genes[hover];
        std::string descripcion = nombreGen(gen);
        if (gen == GenEstatico::TELETRANSPORTARSE) descripcion += ": salto con costo de vida.";
        if (gen == GenEstatico::INMORTAL) descripcion += ": no muere de vejez.";
        if (gen == GenEstatico::VAMPIRISMO) descripcion += ": drena vida en un radio.";
        if (gen == GenEstatico::CANIBALISMO) descripcion += ": caza bichitos.";
        if (gen == GenEstatico::MITOSIS) descripcion += ": clonacion al 90% de vida.";
        if (gen == GenEstatico::INMUNIDAD) descripcion += ": resiste vampirismo.";
        sf::Text tip = texto(descripcion, recursos.fuente(), 11, sf::Color::White,
                             posicionesX[hover] + 23.0f, 139.0f);
        sf::RectangleShape caja(sf::Vector2f(tip.getLocalBounds().width + 12.0f, 24.0f));
        caja.setPosition(posicionesX[hover] + 18.0f, 136.0f);
        caja.setFillColor(sf::Color(10, 12, 20, 245));
        destino.draw(caja);
        destino.draw(tip);
    }
}

void PanelGenetica::dibujarArbol(sf::RenderTarget& destino, GestorRecursos& recursos,
                                RegistroGenetico& registro)
{
    ArbolLinajeGen& arbol = registro.arbol(genMostrado);
    if (arbol.cantidadFundadores() == 0)
    {
        arbolActivo = false;
        return;
    }

    sf::RectangleShape velo(sf::Vector2f(Config::ANCHO_MUNDO, Config::ALTO_MUNDO));
    velo.setFillColor(sf::Color(5, 8, 15, 225));
    destino.draw(velo);

    sf::RectangleShape cabecera(sf::Vector2f(Config::ANCHO_MUNDO,
                                             ALTURA_CABECERA_ARBOL));
    cabecera.setPosition(0.0f, 0.0f);
    cabecera.setFillColor(sf::Color(18, 27, 43, 245));
    destino.draw(cabecera);
    destino.draw(texto("Linaje de " + std::string(nombreGen(genMostrado)),
                       recursos.fuente(), 19, sf::Color(225, 238, 255), 20.0f, 13.0f));
    destino.draw(texto(std::to_string(indiceFundador + 1) + " / " +
                       std::to_string(arbol.cantidadFundadores()) + " origenes",
                       recursos.fuente(), 13, sf::Color(175, 195, 220),
                       Config::ANCHO_MUNDO - 330.0f, 17.0f));
    destino.draw(texto("[<]", recursos.fuente(), 14, sf::Color::White,
                       Config::ANCHO_MUNDO - 143.0f, 17.0f));
    destino.draw(texto("[>]", recursos.fuente(), 14, sf::Color::White,
                       Config::ANCHO_MUNDO - 91.0f, 17.0f));
    destino.draw(texto("[X]", recursos.fuente(), 16, sf::Color(255, 130, 130),
                       Config::ANCHO_MUNDO - 43.0f, 12.0f));

    NodoGenealogico* raiz = arbol.fundador(indiceFundador);
    const float graphTop = 94.0f;
    const float graphBottom = Config::ALTO_MUNDO - 30.0f;
    const float espacioDisponible = graphBottom - graphTop;
    const int totalHojas = std::max(1, raiz->hojas);
    const float alturaTotal =
        static_cast<float>(totalHojas) *
        std::max(26.0f, espacioDisponible / static_cast<float>(totalHojas)) *
        zoom;
    const float leafHeight = alturaTotal / static_cast<float>(totalHojas);
    const float espaciadoEfectivo = leafHeight;
    NodoGenealogico* hovered = nullptr;

    std::function<void(NodoGenealogico*, float, float)> dibujarNodo;
    dibujarNodo = [&](NodoGenealogico* nodo, float top, float bottom)
    {
        const float x = 70.0f + (nodo->profundidad - raiz->profundidad) * 125.0f * zoom +
                        desplazamiento.x;
        const float y = (top + bottom) * 0.5f + desplazamiento.y;
        const float tramoTop = top + desplazamiento.y;
        const float tramoBottom = bottom + desplazamiento.y;
        if (tramoBottom < graphTop || tramoTop > Config::ALTO_MUNDO)
            return;
        if (x > Config::ANCHO_MUNDO)
            return;
        int totalHijosHojas = 0;
        for (int i = 0; i < nodo->hijos.longitud(); ++i)
            totalHijosHojas += hojas(nodo->hijos.obtenerRef(i));
        float cursor = top;
        for (int i = 0; i < nodo->hijos.longitud(); ++i)
        {
            NodoGenealogico* hijo = nodo->hijos.obtenerRef(i);
            const float alto = (bottom - top) *
                static_cast<float>(hojas(hijo)) / static_cast<float>(std::max(1, totalHijosHojas));
            const float childY = cursor + alto * 0.5f + desplazamiento.y;
            const float childX = 70.0f +
                                 (hijo->profundidad - raiz->profundidad) * 125.0f * zoom +
                                 desplazamiento.x;
            sf::Vertex linea[2] = {
                sf::Vertex(sf::Vector2f(x + 10.0f, y), sf::Color(95, 125, 165)),
                sf::Vertex(sf::Vector2f(childX - 10.0f, childY), sf::Color(95, 125, 165))
            };
            destino.draw(linea, 2, sf::Lines);
            dibujarNodo(hijo, cursor, cursor + alto);
            cursor += alto;
        }

        sf::CircleShape circulo(11.0f);
        circulo.setOrigin(11.0f, 11.0f);
        circulo.setPosition(x, y);
        circulo.setFillColor(nodo->idBichito == idSeleccionado ? sf::Color(255, 215, 90) :
                             nodo->estadoVida ? sf::Color(90, 205, 135) :
                                               sf::Color(115, 120, 135));
        circulo.setOutlineThickness(2.0f);
        circulo.setOutlineColor(nodo->idBichito == idSeleccionado ?
                                sf::Color::White : sf::Color(25, 30, 40));
        destino.draw(circulo);
        if (espaciadoEfectivo >= 20.0f)
        {
            sf::Text etiqueta = texto("#" + std::to_string(nodo->idBichito),
                                      recursos.fuente(), 10, sf::Color(225, 230, 240),
                                      x - 14.0f, y + 13.0f);
            destino.draw(etiqueta);
        }
        const sf::Vector2i mouse = posicionMouse(destino);
        const float dx = static_cast<float>(mouse.x) - x;
        const float dy = static_cast<float>(mouse.y) - y;
        if (dx * dx + dy * dy <= 13.0f * 13.0f)
            hovered = nodo;
    };
    dibujarNodo(raiz, graphTop, graphTop + alturaTotal);

    if (hovered != nullptr)
    {
        const GenesVariables& g = hovered->genesVariables;
        const std::string origenMutacion = hovered->mutacionFundadora
            ? " | Mutacion inicial" +
                  (hovered->idProgenitor >= 0
                       ? " (progenitor #" + std::to_string(hovered->idProgenitor) + ")"
                       : "")
            : "";
        const std::string otroProgenitor = hovered->idOtroProgenitor >= 0
            ? " | Otro progenitor #" + std::to_string(hovered->idOtroProgenitor)
            : "";
        const std::string detalles =
            "#" + std::to_string(hovered->idBichito) +
            (hovered->estadoVida ? " vivo" : " fallecido") +
            " | Gen. " + std::to_string(hovered->generacion) +
            origenMutacion +
            otroProgenitor +
            " | VidaMax " + std::to_string(static_cast<int>(g.vidaMax)) +
            " | Vel. " + std::to_string(static_cast<int>(g.velocidad)) +
            " | Metab. " + std::to_string(g.metabolismo) +
            " | EdadMax " + std::to_string(static_cast<int>(g.edadMax));
        sf::Text tip = texto(detalles, recursos.fuente(), 12, sf::Color::White, 20.0f, 665.0f);
        sf::RectangleShape caja(sf::Vector2f(tip.getLocalBounds().width + 18.0f, 30.0f));
        caja.setPosition(12.0f, 660.0f);
        caja.setFillColor(sf::Color(16, 20, 31, 245));
        destino.draw(caja);
        destino.draw(tip);
    }
    destino.draw(texto("Vivos", recursos.fuente(), 11, sf::Color(90, 205, 135), 20.0f, 68.0f));
    destino.draw(texto("Fallecidos", recursos.fuente(), 11, sf::Color(150, 155, 170), 80.0f, 68.0f));
    destino.draw(texto("Arrastra para mover | Rueda para zoom",
                       recursos.fuente(), 11, sf::Color(165, 180, 200), 690.0f, 68.0f));
}
