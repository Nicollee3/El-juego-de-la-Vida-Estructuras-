#ifndef GENOMA_H
#define GENOMA_H

/**
 * @enum GenEstatico
 * @brief Catálogo completo de los 7 genes estáticos definidos por diseño.
 * Afectan al bichito más cercano sin preferencias familiares.
 */
enum class GenEstatico {
    NINGUNO,
    INMUNIDAD,       // Bloquea vampirismo y enfermedad.
    CANIBALISMO,     // Busca bichitos en vez de comida al tener hambre.
    VAMPIRISMO,      // Roba vida de bichitos cercanos.
    TELETRANSPORTARSE, // Salto espacial en el mapa.
    INMORTAL,        // No muere por edad/desgaste natural.
    ENFERMEDAD,      // Drena vida propia y del contagiado cercano.
    MITOSIS          // Permite clonación sin pareja.
};

/**
 * @struct Genoma
 * @brief Almacena los genes estáticos y variables de un individuo.
 */
struct Genoma {
    GenEstatico genEstatico1;
    GenEstatico genEstatico2;
    
    float velocidad;   // Gen variable
    float metabolismo; // Gen variable acoplado: qué tan rápido se gasta la vida

    /**
     * @brief Constructor por defecto. Acopla el metabolismo a la velocidad.
     */
    Genoma(float vel = 2.0f, GenEstatico g1 = GenEstatico::NINGUNO, GenEstatico g2 = GenEstatico::NINGUNO) 
        : genEstatico1(g1), genEstatico2(g2), velocidad(vel) 
    {
        // El metabolismo es proporcional a la velocidad (ej. cuesta el doble de la velocidad)
        metabolismo = velocidad * 2.5f; 
    }
};

#endif // GENOMA_H