/*
 * @file BICHITO.H
 * @brief Representa a cada bichito dentro del juego de la vida.
 * @details Tiene atributos públicos de identificación y de cuánta vida tiene.
 * @author Karoll Ramírez
 * @OCTUBRE 2026
 */


#ifndef BICHITO_H
#define BICHITO_H

class Bichito
{
public:

    int id;
    int vida;
    Bichito()
    {
        id = 0;
        vida = 30;
    }
    Bichito(int _id, int _vida)
    {
        id = _id;
        vida = _vida;
    }
};

#endif