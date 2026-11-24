/*
 * @file MotorDesastres.h
 * @brief Motor de Desastres Naturales y "The Reaper" (Gestor de Mortandad).
 * @details Define el catálogo de desastres, la lógica de filtrado con ColaPrioridad
 *          y la activación manual, aleatoria poco frecuente o por sobrepoblación
 *          de forma desacoplada de
 *          los atributos genéticos de la Parte 1.
 */

#ifndef MOTOR_DESASTRES_H
#define MOTOR_DESASTRES_H

#include <array>
#include <functional>
#include <string>
#include "ArregloDinamico.h"
#include "Bichito.h"
#include "ColaConPrioridad.h"

/*
 * @enum TipoDesastre
 * @brief Catálogo de eventos catastróficos que activan a The Reaper.
 */
enum class TipoDesastre
{
    Ninguno,
    Sobrepoblacion,
    Meteoritos,
    Diluvio,
    BombaNuclear,
    PatriaMilagro
};

/*
 * @struct ResultadoDesastre
 * @brief Resumen del último evento ejecutado por The Reaper.
 */
struct ResultadoDesastre
{
    TipoDesastre tipo = TipoDesastre::Ninguno;
    std::string nombre = "Ninguno";
    int bajasObjetivo = 0;
    int bajasReales = 0;
    std::array<int, ColaPrioridad::NUM_NIVELES> bajasPorNivel = {0, 0, 0};
};

/*
 * @class MotorDesastres
 * @brief Gestiona el catálogo de desastres, la cola de prioridad de The Reaper
 *        y el disparo aleatorio poco frecuente o por sobrepoblación.
 */
class MotorDesastres
{
public:
    using ExtractorPerfil = std::function<PerfilReaper(const Bichito &)>;
    using CallbackEliminar = std::function<void(int idBichito)>;

    // Impacto en número de bichitos de cada desastre del catálogo
    static constexpr int BAJAS_METEORITOS = 20;
    static constexpr int BAJAS_DILUVIO = 30;
    static constexpr int BAJAS_BOMBA_NUCLEAR = 100;
    static constexpr int BAJAS_PATRIA_MILAGRO = 60;

    // Control de sobrepoblación y desastres aleatorios poco frecuentes
    static constexpr int UMBRAL_SOBREPOBLACION = 160;
    static constexpr int OBJETIVO_EQUILIBRIO = 140;
    static constexpr int POBLACION_MINIMA_DESASTRE = 15;
    static constexpr int POBLACION_DESASTRES_MAYORES = 80;

    MotorDesastres();

    /*
     * @brief Permite inyectar un extractor personalizado para conectar la Parte 1 (genética/edad)
     *        sin modificar esta clase.
     */
    void setExtractorPerfil(ExtractorPerfil extractor);

    /*
     * @brief Configura el umbral de sobrepoblación y la población objetivo tras equilibrar.
     */
    void setLimitesSobrepoblacion(int umbralMaximo, int objetivoEquilibrio);

    /*
     * @brief Reinicia el registro del último desastre.
     */
    void reiniciar();

    /*
     * @brief Verifica la sobrepoblación y una probabilidad muy baja de desastre espontáneo.
     * @return true si ocurrió un desastre en este paso.
     */
    bool actualizar(float dt, ArregloDinamico<Bichito> &poblacion,
                    const CallbackEliminar &eliminarPorId);

    /*
     * @brief Ejecuta inmediatamente un desastre específico usando The Reaper.
     * @param forzarSiSobrepoblacion Si es true y el desastre es Sobrepoblacion,
     *        actúa incluso si la población no supera el umbral (elimina 25% de los más viejos).
     */
    ResultadoDesastre ejecutarDesastre(TipoDesastre tipo, ArregloDinamico<Bichito> &poblacion,
                                       const CallbackEliminar &eliminarPorId,
                                       bool forzarSiSobrepoblacion = false);

    /*
     * @brief Elige un desastre natural aleatorio del catálogo según el tamaño de la población.
     */
    static TipoDesastre elegirDesastreAleatorio(int poblacionActual = POBLACION_DESASTRES_MAYORES);

    /*
     * @brief Devuelve la cantidad de bajas configurada para un tipo de desastre.
     */
    static int impactoDe(TipoDesastre tipo);

    /*
     * @brief Nombre legible del desastre.
     */
    static std::string nombreDe(TipoDesastre tipo);

    /*
     * @brief Extractor por defecto (modo desacoplado mientras se integra la Parte 1).
     */
    static PerfilReaper perfilPorDefecto(const Bichito &bichito);

    const ResultadoDesastre &getUltimoResultado() const { return ultimoResultado; }
    std::string getResumenUltimoDesastre() const;

private:
    ColaPrioridad reaper;
    ExtractorPerfil extractorPerfil;
    ResultadoDesastre ultimoResultado;

    int umbralSobrepoblacion;
    int objetivoEquilibrio;

    void poblarColaReaper(ArregloDinamico<Bichito> &poblacion, bool priorizarMasViejos);
};

#endif
