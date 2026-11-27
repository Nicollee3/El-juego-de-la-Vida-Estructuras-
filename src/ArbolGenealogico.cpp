#include "ArbolGenealogico.h"
#include <cassert>
#include <iostream>

ArbolLinajeGen::ArbolLinajeGen(GenEstatico genRastreado)
    : gen(genRastreado) {}

ArbolLinajeGen::~ArbolLinajeGen()
{
    limpiar();
}

void ArbolLinajeGen::limpiar()
{
    for (int i = 0; i < fundadores.longitud(); ++i)
    {
        NodoGenealogico* raiz = fundadores.obtenerRef(i);
        if (raiz != nullptr)
            destruirRama(raiz);
    }
    fundadores.vaciar();
    nodos.vaciar();
}

void ArbolLinajeGen::destruirRama(NodoGenealogico* nodo)
{
    if (nodo == nullptr)
        return;
    for (int i = 0; i < nodo->hijos.longitud(); ++i)
        destruirRama(nodo->hijos.obtenerRef(i));
    delete nodo;
}

void ArbolLinajeGen::registrarPortador(int id, int idPadre, int idMadre,
                                       int generacion, bool porMitosis,
                                       const GenesVariables& genes,
                                       bool permitirFundadorAdicional)
{
#ifndef NDEBUG
    if (nodos.contiene(id))
        std::cerr << "Aviso: ID de bichito ya presente en el arbol genealogico: "
                  << id << std::endl;
#endif
    if (nodos.contiene(id))
        return;

    NodoGenealogico* padre = nullptr;
    NodoGenealogico* madre = nullptr;
    if (idPadre >= 0)
        nodos.buscar(idPadre, padre);
    if (idMadre >= 0)
        nodos.buscar(idMadre, madre);

    if (padre == nullptr)
        padre = madre;
    else if (madre == padre)
        madre = nullptr;

    if (padre == nullptr && (estaBloqueado() || permitirFundadorAdicional))
        std::cerr << "Aviso: bichito #" << id
                  << " no encontro padre en el arbol y se registra como raiz adicional."
                  << std::endl;

    const int idPadreRama = padre == nullptr ? -1 : padre->idBichito;
    const int idOtroProgenitor =
        madre != nullptr && madre != padre ? madre->idBichito : -1;
    const int idProgenitor = padre == nullptr
        ? (idPadre >= 0 ? idPadre : idMadre)
        : -1;
    const bool mutacionFundadora =
        padre == nullptr && !estaBloqueado() && !permitirFundadorAdicional;
    const int profundidadNodo =
        padre == nullptr ? 0 : padre->profundidad + 1;
    NodoGenealogico* nodo = new NodoGenealogico(
        id, idPadreRama, idProgenitor, idOtroProgenitor,
        generacion, profundidadNodo, porMitosis,
        mutacionFundadora, genes);
    nodos.insertar(id, nodo);
    if (padre != nullptr)
    {
        padre->hijos.generar(nodo);
        nodo->ramaPadre = padre;
        if (padre->hijos.longitud() > 1)
        {
            for (NodoGenealogico* ancestro = padre; ancestro != nullptr;
                 ancestro = ancestro->ramaPadre)
                ++ancestro->hojas;
        }
    }
    else
        fundadores.generar(nodo);
}

void ArbolLinajeGen::registrarMuerte(int id)
{
    NodoGenealogico* nodo = nullptr;
    if (nodos.buscar(id, nodo) && nodo != nullptr)
        nodo->estadoVida = false;
}

bool ArbolLinajeGen::buscar(int id, NodoGenealogico*& nodo) const
{
    return nodos.buscar(id, nodo);
}

RegistroGenetico::RegistroGenetico()
    : inmunidad(GenEstatico::INMUNIDAD),
      canibalismo(GenEstatico::CANIBALISMO),
      vampirismo(GenEstatico::VAMPIRISMO),
      teletransporte(GenEstatico::TELETRANSPORTARSE),
      inmortal(GenEstatico::INMORTAL),
      mitosis(GenEstatico::MITOSIS) {}

ArbolLinajeGen& RegistroGenetico::arbol(GenEstatico gen)
{
    switch (gen)
    {
    case GenEstatico::INMUNIDAD: return inmunidad;
    case GenEstatico::CANIBALISMO: return canibalismo;
    case GenEstatico::VAMPIRISMO: return vampirismo;
    case GenEstatico::TELETRANSPORTARSE: return teletransporte;
    case GenEstatico::INMORTAL: return inmortal;
    case GenEstatico::MITOSIS: return mitosis;
    case GenEstatico::NINGUNO: default: return inmunidad;
    }
}

const ArbolLinajeGen& RegistroGenetico::arbol(GenEstatico gen) const
{
    return const_cast<RegistroGenetico*>(this)->arbol(gen);
}

bool RegistroGenetico::puedeMutar(GenEstatico gen) const
{
    return gen != GenEstatico::NINGUNO && !arbol(gen).estaBloqueado();
}

void RegistroGenetico::registrarBichito(int id, int idPadre, int idMadre,
                                        int generacion, bool porMitosis,
                                        const Genoma& genoma,
                                        bool permitirFundadorAdicional)
{
    const GenEstatico genes[] = {genoma.genEstatico1, genoma.genEstatico2};
    for (int i = 0; i < 2; ++i)
    {
        if (genes[i] != GenEstatico::NINGUNO &&
            (i == 0 || genes[i] != genes[0]))
        {
            arbol(genes[i]).registrarPortador(id, idPadre, idMadre,
                                             generacion, porMitosis,
                                             genoma.variables(),
                                             permitirFundadorAdicional);
        }
    }
}

void RegistroGenetico::registrarMuerte(int id)
{
    inmunidad.registrarMuerte(id);
    canibalismo.registrarMuerte(id);
    vampirismo.registrarMuerte(id);
    teletransporte.registrarMuerte(id);
    inmortal.registrarMuerte(id);
    mitosis.registrarMuerte(id);
}

void RegistroGenetico::limpiar()
{
    inmunidad.limpiar();
    canibalismo.limpiar();
    vampirismo.limpiar();
    teletransporte.limpiar();
    inmortal.limpiar();
    mitosis.limpiar();
}
