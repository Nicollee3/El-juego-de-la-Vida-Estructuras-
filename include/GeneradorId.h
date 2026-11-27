#ifndef GENERADOR_ID_H
#define GENERADOR_ID_H

class GeneradorId
{
public:
    static int siguiente()
    {
        return contador++;
    }

    static void reservar(int id)
    {
        if (id >= contador)
            contador = id + 1;
    }

    static void reiniciar()
    {
        contador = 1;
    }

private:
    static int contador;
};

inline int GeneradorId::contador = 1;

#endif
