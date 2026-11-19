/*
 * @file RTree.h
 * @brief Implementación de un Árbol de Rectángulos (R-Tree) para búsqueda espacial 2D.
 * @details Indexa las posiciones de las comidas mediante Rectángulos Mínimos Delimitadores (MBR),
 *          permitiendo descartar ramas enteras del mapa y encontrar alimentos cercanos en O(log N).
 */

#ifndef RTREE_H
#define RTREE_H

#include <algorithm>
#include <cmath>
#include <vector>

/*
 * @struct Rectangulo
 * @brief Define un Rectángulo Mínimo Delimitador (MBR - Minimum Bounding Rectangle).
 * @details Encierra una o varias comidas en el plano 2D y provee operaciones geométricas
 *          de intersección, expansión y cálculo de área.
 */
struct Rectangulo
{
    float minX, minY;
    float maxX, maxY;

    Rectangulo() : minX(0.0f), minY(0.0f), maxX(0.0f), maxY(0.0f) {}
    Rectangulo(float _minX, float _minY, float _maxX, float _maxY)
        : minX(_minX), minY(_minY), maxX(_maxX), maxY(_maxY) {}

    /*
     * @brief Verifica si este rectángulo se cruza con otro (por ejemplo, el área de visión).
     */
    bool intersecta(const Rectangulo &otro) const
    {
        return !(otro.minX > maxX || otro.maxX < minX ||
                 otro.minY > maxY || otro.maxY < minY);
    }

    /*
     * @brief Expande los límites de este rectángulo para abarcar completamente a otro.
     */
    void expandir(const Rectangulo &otro)
    {
        minX = std::min(minX, otro.minX);
        minY = std::min(minY, otro.minY);
        maxX = std::max(maxX, otro.maxX);
        maxY = std::max(maxY, otro.maxY);
    }

    /*
     * @brief Calcula el área geométrica actual del rectángulo.
     */
    float area() const
    {
        return (maxX - minX) * (maxY - minY);
    }

    /*
     * @brief Calcula cuánto crecería el área si se incluye otro rectángulo (heurística de inserción).
     */
    float aumentoDeArea(const Rectangulo &otro) const
    {
        const float nMinX = std::min(minX, otro.minX);
        const float nMinY = std::min(minY, otro.minY);
        const float nMaxX = std::max(maxX, otro.maxX);
        const float nMaxY = std::max(maxY, otro.maxY);
        return ((nMaxX - nMinX) * (nMaxY - nMinY)) - area();
    }
};

/*
 * @struct EntradaComidaRTree
 * @brief Representa el registro de una unidad de comida dentro de un nodo hoja del R-Tree.
 */
struct EntradaComidaRTree
{
    int idComida;
    float x, y;
    Rectangulo mbr;
};

/*
 * @class NodoRTree
 * @brief Nodo interno o de hoja perteneciente a la jerarquía del R-Tree.
 * @details Si es hoja, almacena entradas de comida; si es nodo interno, almacena punteros
 *          a sub-rectángulos hijos y mantiene actualizado su MBR envolvente.
 */
class NodoRTree
{
public:
    bool esHoja;
    Rectangulo mbr;
    std::vector<EntradaComidaRTree> entradas;
    std::vector<NodoRTree *> hijos;

    explicit NodoRTree(bool _esHoja) : esHoja(_esHoja) {}

    ~NodoRTree()
    {
        for (NodoRTree *hijo : hijos)
        {
            delete hijo;
        }
    }

    /*
     * @brief Recalcula el Rectángulo Mínimo Delimitador ajustándolo a sus entradas o hijos actuales.
     */
    void recalcularMBR();
};

/*
 * @class RTree
 * @brief Estructura espacial dinámica para indexar y consultar comida cercana en tiempo O(log N).
 * @details Agrupa comidas próximas mediante cajas envolventes (MBR) y las divide según
 *          el eje de mayor dispersión cuando un nodo supera su capacidad máxima.
 */
class RTree
{
private:
    NodoRTree *raiz;
    int maxEntradas;

    /*
     * @brief Inserta recursivamente una entrada eligiendo el subárbol con menor expansión de área.
     */
    NodoRTree *insertarRecursivo(NodoRTree *nodo, const EntradaComidaRTree &entrada);

    /*
     * @brief Divide un nodo desbordado en dos mitades ordenadas por el eje de mayor dispersión.
     */
    NodoRTree *dividirNodo(NodoRTree *nodo);

    /*
     * @brief Recorre recursivamente las ramas que intersectan el área de búsqueda del bichito.
     */
    void buscarEnRangoRecursivo(NodoRTree *nodo, const Rectangulo &areaBusqueda,
                                std::vector<int> &idsResultados) const;

public:
    explicit RTree(int _maxEntradas = 4);
    ~RTree();

    RTree(const RTree &) = delete;
    RTree &operator=(const RTree &) = delete;

    /*
     * @brief Libera todos los nodos actuales y reinicia el árbol con una raíz hoja vacía.
     */
    void limpiar();

    /*
     * @brief Registra una comida con su identificador y coordenadas (x, y) dentro del R-Tree.
     */
    void insertar(int idComida, float x, float y);

    /*
     * @brief Consulta los IDs de las comidas ubicadas dentro del radio de visión de un bichito.
     */
    std::vector<int> buscarCercanas(float x, float y, float radioVision) const;
};

#endif