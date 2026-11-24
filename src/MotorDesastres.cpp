#include "MotorDesastres.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace
{
    constexpr double PROBABILIDAD_DESASTRE_POR_SEGUNDO =
        0.00001; // Promedio: uno cada 100 000 s simulados.

    double uniformeAltaPrecision()
    {
        const double base = static_cast<double>(RAND_MAX) + 1.0;
        const double valor = static_cast<double>(std::rand()) * base + std::rand();
        return valor / (base * base);
    }

    bool debeOcurrirDesastreEspontaneo(float dt)
    {
        if (dt <= 0.0f)
            return false;

        const double probabilidad = 1.0 -
            std::exp(-PROBABILIDAD_DESASTRE_POR_SEGUNDO * static_cast<double>(dt));
        return uniformeAltaPrecision() < probabilidad;
    }
}

MotorDesastres::MotorDesastres()
    : extractorPerfil(perfilPorDefecto),
      umbralSobrepoblacion(UMBRAL_SOBREPOBLACION),
      objetivoEquilibrio(OBJETIVO_EQUILIBRIO)
{}

void MotorDesastres::setExtractorPerfil(ExtractorPerfil extractor)
{
    if (extractor)
    {
        extractorPerfil = std::move(extractor);
    }
    else
    {
        extractorPerfil = perfilPorDefecto;
    }
}

void MotorDesastres::setLimitesSobrepoblacion(int umbralMaximo, int objetivo)
{
    if (umbralMaximo < 1)
        umbralMaximo = 1;
    if (objetivo < 0)
        objetivo = 0;
    if (objetivo >= umbralMaximo)
        objetivo = std::max(0, umbralMaximo - 1);

    umbralSobrepoblacion = umbralMaximo;
    objetivoEquilibrio = objetivo;
}

void MotorDesastres::reiniciar()
{
    reaper.vaciar();
    ultimoResultado = ResultadoDesastre{};
}

PerfilReaper MotorDesastres::perfilPorDefecto(const Bichito &bichito)
{
    PerfilReaper perfil;
    perfil.id = bichito.getId();
    perfil.vida = bichito.getVida();
    perfil.edad = bichito.getEdadActual();
    perfil.edadMaxima = bichito.getEdadMaxima();
    perfil.inmortal = bichito.tieneGen(GenEstatico::INMORTAL);
    perfil.enfermo = false;
    perfil.inmune = bichito.tieneGen(GenEstatico::INMUNIDAD);
    return perfil;
}

int MotorDesastres::impactoDe(TipoDesastre tipo)
{
    switch (tipo)
    {
    case TipoDesastre::Meteoritos:
        return BAJAS_METEORITOS;
    case TipoDesastre::Diluvio:
        return BAJAS_DILUVIO;
    case TipoDesastre::BombaNuclear:
        return BAJAS_BOMBA_NUCLEAR;
    case TipoDesastre::PatriaMilagro:
        return BAJAS_PATRIA_MILAGRO;
    case TipoDesastre::Sobrepoblacion:
    case TipoDesastre::Ninguno:
    default:
        return 0;
    }
}

std::string MotorDesastres::nombreDe(TipoDesastre tipo)
{
    switch (tipo)
    {
    case TipoDesastre::Sobrepoblacion:
        return "Sobrepoblacion";
    case TipoDesastre::Meteoritos:
        return "Meteoritos";
    case TipoDesastre::Diluvio:
        return "Diluvio";
    case TipoDesastre::BombaNuclear:
        return "Bomba Nuclear";
    case TipoDesastre::PatriaMilagro:
        return "Patria Milagro";
    case TipoDesastre::Ninguno:
    default:
        return "Ninguno";
    }
}

TipoDesastre MotorDesastres::elegirDesastreAleatorio(int poblacionActual)
{
    const int r = std::rand() % 100;

    // Si la colonia aún es pequeña (< 80), evitar extinguirla con los eventos masivos.
    if (poblacionActual < POBLACION_DESASTRES_MAYORES)
    {
        return (r < 55) ? TipoDesastre::Meteoritos : TipoDesastre::Diluvio;
    }

    // Selección aleatoria ponderada entre los 4 desastres catastróficos:
    // Meteoritos (35%), Diluvio (30%), Bomba Nuclear (20%), Patria Milagro (15%).
    if (r < 35)
        return TipoDesastre::Meteoritos;
    if (r < 65)
        return TipoDesastre::Diluvio;
    if (r < 85)
        return TipoDesastre::BombaNuclear;
    return TipoDesastre::PatriaMilagro;
}

void MotorDesastres::poblarColaReaper(ArregloDinamico<Bichito> &poblacion, bool priorizarMasViejos)
{
    reaper.vaciar();

    if (!priorizarMasViejos)
    {
        for (int i = 0; i < poblacion.longitud(); ++i)
        {
            const Bichito &b = poblacion.obtenerRef(i);
            if (b.estaCondenado())
                continue;

            reaper.insertar(extractorPerfil(b));
        }
        return;
    }

    // En sobrepoblación, ordenamos de mayor a menor edad antes de insertar en la cola
    // para que dentro de cada nivel de prioridad salgan primero los más viejos.
    std::vector<PerfilReaper> perfiles;
    perfiles.reserve(static_cast<std::size_t>(poblacion.longitud()));

    for (int i = 0; i < poblacion.longitud(); ++i)
    {
        const Bichito &b = poblacion.obtenerRef(i);
        if (b.estaCondenado())
            continue;

        perfiles.push_back(extractorPerfil(b));
    }

    std::stable_sort(perfiles.begin(), perfiles.end(),
                     [](const PerfilReaper &a, const PerfilReaper &b)
                     {
                         if (a.edad != b.edad)
                             return a.edad > b.edad;
                         return a.vida < b.vida;
                     });

    for (const PerfilReaper &perfil : perfiles)
    {
        reaper.insertar(perfil);
    }
}

ResultadoDesastre MotorDesastres::ejecutarDesastre(TipoDesastre tipo,
                                                   ArregloDinamico<Bichito> &poblacion,
                                                   const CallbackEliminar &eliminarPorId,
                                                   bool forzarSiSobrepoblacion)
{
    ResultadoDesastre resultado;
    resultado.tipo = tipo;
    resultado.nombre = nombreDe(tipo);

    if (tipo == TipoDesastre::Ninguno || poblacion.longitud() == 0)
        return resultado;

    int cantidadObjetivo = 0;
    bool priorizarMasViejos = false;

    if (tipo == TipoDesastre::Sobrepoblacion)
    {
        if (poblacion.longitud() > umbralSobrepoblacion)
        {
            cantidadObjetivo = poblacion.longitud() - objetivoEquilibrio;
        }
        else if (forzarSiSobrepoblacion)
        {
            cantidadObjetivo = std::max(1, poblacion.longitud() / 4);
        }
        else
        {
            return resultado;
        }
        priorizarMasViejos = true;
    }
    else
    {
        cantidadObjetivo = impactoDe(tipo);
    }

    resultado.bajasObjetivo = cantidadObjetivo;

    // 1. Clasificar toda la población viva en la Cola con Prioridad de The Reaper.
    poblarColaReaper(poblacion, priorizarMasViejos);

    // 2. Extraer y eliminar hasta cantidadObjetivo bichitos en estricto orden de prioridad.
    int idVictima = -1;
    int nivelVictima = 0;
    while (resultado.bajasReales < cantidadObjetivo &&
           reaper.eliminar_primero(idVictima, &nivelVictima))
    {
        if (eliminarPorId)
        {
            eliminarPorId(idVictima);
        }
        ++resultado.bajasReales;
        if (nivelVictima >= 0 && nivelVictima < ColaPrioridad::NUM_NIVELES)
        {
            ++resultado.bajasPorNivel[static_cast<std::size_t>(nivelVictima)];
        }
    }

    reaper.vaciar();
    ultimoResultado = resultado;

    std::cout << "[LA PARCA] Desastre: " << resultado.nombre
              << " | Eliminados: " << resultado.bajasReales
              << "/" << resultado.bajasObjetivo
              << " (N1:" << resultado.bajasPorNivel[0]
              << ", N2:" << resultado.bajasPorNivel[1]
              << ", N3:" << resultado.bajasPorNivel[2] << ")\n";

    return resultado;
}

bool MotorDesastres::actualizar(float dt, ArregloDinamico<Bichito> &poblacion,
                                const CallbackEliminar &eliminarPorId)
{
    // 1. Prioridad absoluta: controlar sobrepoblación si se supera el límite.
    if (poblacion.longitud() > umbralSobrepoblacion)
    {
        ejecutarDesastre(TipoDesastre::Sobrepoblacion, poblacion, eliminarPorId);
        return true;
    }

    // Evitar desastres espontáneos antes de que exista una colonia suficiente.
    if (poblacion.longitud() < POBLACION_MINIMA_DESASTRE)
        return false;

    if (debeOcurrirDesastreEspontaneo(dt))
    {
        const TipoDesastre elegido = elegirDesastreAleatorio(poblacion.longitud());
        ejecutarDesastre(elegido, poblacion, eliminarPorId);
        return true;
    }

    return false;
}

std::string MotorDesastres::getResumenUltimoDesastre() const
{
    if (ultimoResultado.tipo == TipoDesastre::Ninguno)
        return "Ninguno";

    return ultimoResultado.nombre + " (-" + std::to_string(ultimoResultado.bajasReales) + ")";
}
