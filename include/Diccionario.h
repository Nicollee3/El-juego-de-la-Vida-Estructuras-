/*
 * @file Diccionario.h
 * @brief Diccionario hash genérico que asocia IDs enteros con valores.
 * @details Usa direccionamiento abierto, marcas de borrado y rehashing
 *          automático para mantener las operaciones O(1) en promedio.
 */

#ifndef DICCIONARIO_H
#define DICCIONARIO_H

template <typename T>
class Diccionario
{
private:
    enum Estado { LIBRE, OCUPADO, BORRADO };

    struct Entrada
    {
        int id;
        T valor;
        Estado estado;

        Entrada() : id(-1), valor(), estado(LIBRE) {}
    };

    Entrada *tabla;
    int capacidad;
    int elementosActivos;
    int tombstones;

    int funcionHash(int id, int cap) const
    {
        unsigned int hash = static_cast<unsigned int>(id);
        hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
        hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
        hash = (hash >> 16) ^ hash;
        return static_cast<int>(hash % static_cast<unsigned int>(cap));
    }

    void insertarInterno(int id, const T &valor)
    {
        int indice = funcionHash(id, capacidad);
        while (tabla[indice].estado == OCUPADO)
        {
            indice = (indice + 1) % capacidad;
        }

        tabla[indice].id = id;
        tabla[indice].valor = valor;
        tabla[indice].estado = OCUPADO;
        elementosActivos++;
    }

    void rehash()
    {
        const int antiguaCapacidad = capacidad;
        Entrada *antiguaTabla = tabla;

        capacidad = antiguaCapacidad * 2 + 1;
        tabla = new Entrada[capacidad];
        elementosActivos = 0;
        tombstones = 0;

        for (int i = 0; i < antiguaCapacidad; ++i)
        {
            if (antiguaTabla[i].estado == OCUPADO)
            {
                insertarInterno(antiguaTabla[i].id, antiguaTabla[i].valor);
            }
        }

        delete[] antiguaTabla;
    }

public:
    explicit Diccionario(int capacidadInicial = 503)
        : tabla(nullptr),
          capacidad(capacidadInicial > 0 ? capacidadInicial : 503),
          elementosActivos(0),
          tombstones(0)
    {
        tabla = new Entrada[capacidad];
    }

    ~Diccionario()
    {
        delete[] tabla;
    }

    Diccionario(const Diccionario &) = delete;
    Diccionario &operator=(const Diccionario &) = delete;

    void insertar(int id, const T &valor)
    {
        int indice = funcionHash(id, capacidad);
        int primerTombstone = -1;

        while (tabla[indice].estado != LIBRE)
        {
            if (tabla[indice].estado == OCUPADO && tabla[indice].id == id)
            {
                tabla[indice].valor = valor;
                return;
            }

            if (tabla[indice].estado == BORRADO && primerTombstone == -1)
            {
                primerTombstone = indice;
            }
            indice = (indice + 1) % capacidad;
        }

        if (static_cast<float>(elementosActivos + tombstones + 1) / capacidad > 0.7f)
        {
            rehash();
            indice = funcionHash(id, capacidad);
            while (tabla[indice].estado == OCUPADO)
            {
                indice = (indice + 1) % capacidad;
            }
            primerTombstone = -1;
        }

        const int indiceInsercion = primerTombstone != -1 ? primerTombstone : indice;
        if (tabla[indiceInsercion].estado == BORRADO)
        {
            tombstones--;
        }

        tabla[indiceInsercion].id = id;
        tabla[indiceInsercion].valor = valor;
        tabla[indiceInsercion].estado = OCUPADO;
        elementosActivos++;
    }

    bool buscar(int id, T &salida) const
    {
        int indice = funcionHash(id, capacidad);
        const int inicio = indice;

        while (tabla[indice].estado != LIBRE)
        {
            if (tabla[indice].estado == OCUPADO && tabla[indice].id == id)
            {
                salida = tabla[indice].valor;
                return true;
            }
            indice = (indice + 1) % capacidad;
            if (indice == inicio)
            {
                break;
            }
        }
        return false;
    }

    bool contiene(int id) const
    {
        T valor;
        return buscar(id, valor);
    }

    void eliminar(int id)
    {
        int indice = funcionHash(id, capacidad);
        const int inicio = indice;

        while (tabla[indice].estado != LIBRE)
        {
            if (tabla[indice].estado == OCUPADO && tabla[indice].id == id)
            {
                tabla[indice].estado = BORRADO;
                tabla[indice].id = -1;
                tabla[indice].valor = T();
                elementosActivos--;
                tombstones++;
                return;
            }
            indice = (indice + 1) % capacidad;
            if (indice == inicio)
            {
                break;
            }
        }
    }

    void vaciar()
    {
        delete[] tabla;
        tabla = new Entrada[capacidad];
        elementosActivos = 0;
        tombstones = 0;
    }

    int getCapacidad() const { return capacidad; }
    int getElementos() const { return elementosActivos; }
};

#endif
