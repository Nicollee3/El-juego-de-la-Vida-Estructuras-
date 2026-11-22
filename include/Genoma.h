#ifndef GENOMA_H
#define GENOMA_H

enum class GenEstatico
{
    NINGUNO,
    INMUNIDAD,
    CANIBALISMO,
    VAMPIRISMO,
    TELETRANSPORTARSE,
    INMORTAL,
    MITOSIS
};

struct GenesVariables
{
    float vidaMax;
    float velocidad;
    float metabolismo;
    float edadMax;

    GenesVariables(float vida = 100.0f, float vel = 120.0f,
                   float metab = 1.0f, float edad = 90.0f)
        : vidaMax(vida), velocidad(vel), metabolismo(metab), edadMax(edad) {}
};

struct Genoma
{
    GenEstatico genEstatico1;
    GenEstatico genEstatico2;
    float vidaMax;
    float velocidad;
    float metabolismo;
    float edadMax;

    Genoma(float vel = 120.0f, GenEstatico g1 = GenEstatico::NINGUNO,
           GenEstatico g2 = GenEstatico::NINGUNO)
        : genEstatico1(g1), genEstatico2(g1 != GenEstatico::NINGUNO && g1 == g2
                                           ? GenEstatico::NINGUNO : g2),
          vidaMax(100.0f),
          velocidad(vel > 0.0f ? vel : 120.0f), metabolismo(1.0f), edadMax(90.0f) {}

    Genoma(const GenesVariables& variables, GenEstatico g1, GenEstatico g2)
        : genEstatico1(g1), genEstatico2(g1 != GenEstatico::NINGUNO && g1 == g2
                                           ? GenEstatico::NINGUNO : g2),
          vidaMax(variables.vidaMax),
          velocidad(variables.velocidad), metabolismo(variables.metabolismo),
          edadMax(variables.edadMax) {}

    GenesVariables variables() const
    {
        return GenesVariables(vidaMax, velocidad, metabolismo, edadMax);
    }
};

#endif
