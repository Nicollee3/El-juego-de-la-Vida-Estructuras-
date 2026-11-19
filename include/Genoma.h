#ifndef GENOMA_H
#define GENOMA_H

/**
 * @enum GenEstatico
 * @brief Catálogo completo de los 7 genes estáticos definidos por diseño.
 * Afectan al bichito más cercano sin preferencias familiares.
 */
enum class GenEstatico {
    NINGUNO,
    INMUNIDAD,         // Bloquea vampirismo, canibalismo y enfermedad.
    CANIBALISMO,       // Busca bichitos en vez de comida al tener hambre.
    VAMPIRISMO,        // Roba vida de bichitos cercanos.
    TELETRANSPORTARSE, // Salto espacial en el mapa hacia su objetivo.
    INMORTAL,          // No muere por vejez.
    ENFERMEDAD,        // Drena vida propia y del contagiado cercano.
    MITOSIS            // Permite clonación sin pareja.
};

/**
 * @struct Genoma
 * @brief Almacena los genes estáticos y variables de un individuo.
 */
struct Genoma {
    GenEstatico genEstatico1;
    GenEstatico genEstatico2;
    
    float velocidad;   // Gen variable (en píxeles/segundo, base 120.0f)
    float metabolismo; // Gen variable acoplado: qué tan rápido se gasta la vida

    /**
     * @brief Constructor por defecto. Acopla el metabolismo a la velocidad.
     */
    Genoma(float vel = 120.0f, GenEstatico g1 = GenEstatico::NINGUNO, GenEstatico g2 = GenEstatico::NINGUNO) 
        : genEstatico1(g1), genEstatico2(g2)
    {
        // Garantizamos que la velocidad base nunca sea menor a 30 px/s para evitar que tiemblen sin avanzar
        velocidad = (vel < 30.0f) ? 120.0f : vel;
        // Metabolismo acoplado equilibrado (a 120 px/s da 12.0f -> gasta ~0.6 de vida/seg)
        metabolismo = velocidad * 0.10f; 
    }
};

#endif // GENOMA_H