#include "ArbolGenealogico.h"

ArbolLinajeGen::ArbolLinajeGen(GenEstatico genRastreado)
    : gen(genRastreado) {}

ArbolLinajeGen::~ArbolLinajeGen()
{
    limpiar();
}

void ArbolLinajeGen::limpiar()
{
    for (int i = 0; i < fundadores.longitud(); ++i)
        destruirRama(fundadores.obtenerRef(i));
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
    if (nodos.contiene(id))
        return;

    NodoGenealogico* padre = nullptr;
    if (idPadre >= 0)
        nodos.buscar(idPadre, padre);
    if (padre == nullptr && idMadre >= 0)
        nodos.buscar(idMadre, padre);

    if (padre == nullptr && estaBloqueado() && !permitirFundadorAdicional)
        return;

    const int idPadreRama = padre == nullptr ? -1 : padre->idBichito;
    const int idProgenitor = padre == nullptr
        ? (idPadre >= 0 ? idPadre : idMadre)
        : -1;
    NodoGenealogico* nodo = new NodoGenealogico(
        id, idPadreRama, idProgenitor, generacion, porMitosis,
        padre == nullptr, genes);
    nodos.insertar(id, nodo);
    if (padre != nullptr)
        padre->hijos.generar(nodo);
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
