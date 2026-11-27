#ifndef ARBOL_GENEALOGICO_H
#define ARBOL_GENEALOGICO_H

#include "ArregloDinamico.h"
#include "Diccionario.h"
#include "Genoma.h"

struct NodoGenealogico
{
    int idBichito;
    int idPadreRama;
    int idProgenitor;
    int idOtroProgenitor;
    int generacion;
    int profundidad;
    bool estadoVida;
    bool porMitosis;
    bool mutacionFundadora;
    GenesVariables genesVariables;
    NodoGenealogico* ramaPadre;
    int hojas;
    ArregloDinamico<NodoGenealogico*> hijos;

    NodoGenealogico(int id, int padreRama, int progenitor, int otroProgenitor,
                    int gen, int profundidadNodo, bool mitosis,
                    bool origenMutacion, const GenesVariables& genes)
        : idBichito(id), idPadreRama(padreRama), idProgenitor(progenitor),
          idOtroProgenitor(otroProgenitor), generacion(gen),
          profundidad(profundidadNodo), estadoVida(true), porMitosis(mitosis),
          mutacionFundadora(origenMutacion), genesVariables(genes),
          ramaPadre(nullptr), hojas(1), hijos(2) {}
};

class ArbolLinajeGen
{
public:
    explicit ArbolLinajeGen(GenEstatico gen);
    ~ArbolLinajeGen();
    ArbolLinajeGen(const ArbolLinajeGen&) = delete;
    ArbolLinajeGen& operator=(const ArbolLinajeGen&) = delete;

    void registrarPortador(int id, int idPadre, int idMadre, int generacion,
                           bool porMitosis, const GenesVariables& genes,
                           bool permitirFundadorAdicional = false);
    void registrarMuerte(int id);
    void limpiar();
    bool buscar(int id, NodoGenealogico*& nodo) const;
    int cantidadFundadores() const { return fundadores.longitud(); }
    NodoGenealogico* fundador(int indice) { return fundadores.obtenerRef(indice); }
    int cantidadPortadores() const { return nodos.getElementos(); }
    GenEstatico genRastreado() const { return gen; }
    // Se mantiene bloqueada la mutacion mientras exista algun fundador,
    // incluso si todos sus portadores estan muertos.
    bool estaBloqueado() const { return fundadores.longitud() > 0; }

private:
    static void destruirRama(NodoGenealogico* nodo);
    GenEstatico gen;
    ArregloDinamico<NodoGenealogico*> fundadores;
    Diccionario<NodoGenealogico*> nodos;
};

class RegistroGenetico
{
public:
    RegistroGenetico();
    ArbolLinajeGen& arbol(GenEstatico gen);
    const ArbolLinajeGen& arbol(GenEstatico gen) const;
    void registrarBichito(int id, int idPadre, int idMadre, int generacion,
                          bool porMitosis, const Genoma& genoma,
                          bool permitirFundadorAdicional = false);
    bool puedeMutar(GenEstatico gen) const;
    void registrarMuerte(int id);
    void limpiar();

private:
    ArbolLinajeGen inmunidad;
    ArbolLinajeGen canibalismo;
    ArbolLinajeGen vampirismo;
    ArbolLinajeGen teletransporte;
    ArbolLinajeGen inmortal;
    ArbolLinajeGen mitosis;
    ArbolLinajeGen* buscarArbol(GenEstatico gen);
    const ArbolLinajeGen* buscarArbol(GenEstatico gen) const;
};

#endif
