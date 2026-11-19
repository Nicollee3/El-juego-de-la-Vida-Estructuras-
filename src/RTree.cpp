/*
 * @file RTree.cpp
 * @brief Implementación de los métodos de NodoRTree y RTree para búsqueda espacial.
 * @details Contiene las rutinas de inserción balanceada por menor aumento de área,
 *          división de nodos desbordados y poda de ramas durante consultas de rango.
 */

#include "RTree.h"

void NodoRTree::recalcularMBR()
{
    if (esHoja && !entradas.empty())
    {
        mbr = entradas[0].mbr;
        for (std::size_t i = 1; i < entradas.size(); ++i)
        {
            mbr.expandir(entradas[i].mbr);
        }
    }
    else if (!esHoja && !hijos.empty())
    {
        mbr = hijos[0]->mbr;
        for (std::size_t i = 1; i < hijos.size(); ++i)
        {
            mbr.expandir(hijos[i]->mbr);
        }
    }
}

RTree::RTree(int _maxEntradas)
    : raiz(nullptr),
    maxEntradas(_maxEntradas > 1 ? _maxEntradas : 4)
{
    raiz = new NodoRTree(true);
}

RTree::~RTree()
{
    delete raiz;
}

void RTree::limpiar()
{
    delete raiz;
    raiz = new NodoRTree(true);
}

void RTree::insertar(int idComida, float x, float y)
{
    EntradaComidaRTree nuevaEntrada{idComida, x, y, Rectangulo(x, y, x, y)};
    NodoRTree *nodoDividido = insertarRecursivo(raiz, nuevaEntrada);

    // Si la raíz se desbordó y se dividió, se crea una nueva raíz superior
    if (nodoDividido != nullptr)
    {
        NodoRTree *nuevaRaiz = new NodoRTree(false);
        nuevaRaiz->hijos.push_back(raiz);
        nuevaRaiz->hijos.push_back(nodoDividido);
        nuevaRaiz->recalcularMBR();
        raiz = nuevaRaiz;
    }
}

NodoRTree *RTree::insertarRecursivo(NodoRTree *nodo, const EntradaComidaRTree &entrada)
{
    if (nodo->esHoja)
    {
        nodo->entradas.push_back(entrada);
        nodo->recalcularMBR();
        if (static_cast<int>(nodo->entradas.size()) > maxEntradas)
        {
            return dividirNodo(nodo);
        }
        return nullptr;
    }

    // Elegir el subárbol cuyo MBR requiera la menor expansión de área
    std::size_t mejorHijo = 0;
    float menorAumento = nodo->hijos[0]->mbr.aumentoDeArea(entrada.mbr);

    for (std::size_t i = 1; i < nodo->hijos.size(); ++i)
    {
        const float aumento = nodo->hijos[i]->mbr.aumentoDeArea(entrada.mbr);
        if (aumento < menorAumento)
        {
            menorAumento = aumento;
            mejorHijo = i;
        }
    }

    NodoRTree *nuevoHermano = insertarRecursivo(nodo->hijos[mejorHijo], entrada);
    nodo->recalcularMBR();

    if (nuevoHermano != nullptr)
    {
        nodo->hijos.push_back(nuevoHermano);
        nodo->recalcularMBR();
        if (static_cast<int>(nodo->hijos.size()) > maxEntradas)
        {
            return dividirNodo(nodo);
        }
    }

    return nullptr;
}

NodoRTree *RTree::dividirNodo(NodoRTree *nodo)
{
    NodoRTree *nuevoNodo = new NodoRTree(nodo->esHoja);
    const bool dividirPorX = (nodo->mbr.maxX - nodo->mbr.minX) >= (nodo->mbr.maxY - nodo->mbr.minY);

    if (nodo->esHoja)
    {
        std::sort(nodo->entradas.begin(), nodo->entradas.end(),
                  [dividirPorX](const EntradaComidaRTree &a, const EntradaComidaRTree &b)
                  {
                      return dividirPorX ? (a.x < b.x) : (a.y < b.y);
                  });

        const std::size_t mitad = nodo->entradas.size() / 2;
        nuevoNodo->entradas.assign(nodo->entradas.begin() + mitad, nodo->entradas.end());
        nodo->entradas.resize(mitad);
    }
    else
    {
        std::sort(nodo->hijos.begin(), nodo->hijos.end(),
                  [dividirPorX](const NodoRTree *a, const NodoRTree *b)
                    {
                    return dividirPorX ? (a->mbr.minX < b->mbr.minX) : (a->mbr.minY < b->mbr.minY);
                    });

        const std::size_t mitad = nodo->hijos.size() / 2;
        nuevoNodo->hijos.assign(nodo->hijos.begin() + mitad, nodo->hijos.end());
        nodo->hijos.resize(mitad);
    }

    nodo->recalcularMBR();
    nuevoNodo->recalcularMBR();
    return nuevoNodo;
}

std::vector<int> RTree::buscarCercanas(float x, float y, float radioVision) const
{
    std::vector<int> resultados;
    const Rectangulo areaVision(x - radioVision, y - radioVision, x + radioVision, y + radioVision);
    buscarEnRangoRecursivo(raiz, areaVision, resultados);
    return resultados;
}

void RTree::buscarEnRangoRecursivo(NodoRTree *nodo, const Rectangulo &areaBusqueda,
                                std::vector<int> &idsResultados) const
{
    // Poda espacial: si el rectángulo del nodo no toca el área de visión, se descarta toda la rama
    if (nodo == nullptr || !nodo->mbr.intersecta(areaBusqueda))
    {
        return;
    }

    if (nodo->esHoja)
    {
        for (const auto &entrada : nodo->entradas)
        {
            if (areaBusqueda.intersecta(entrada.mbr))
            {
                idsResultados.push_back(entrada.idComida);
            }
        }
    }
    else
    {
        for (NodoRTree *hijo : nodo->hijos)
        {
            buscarEnRangoRecursivo(hijo, areaBusqueda, idsResultados);
        }
    }
}