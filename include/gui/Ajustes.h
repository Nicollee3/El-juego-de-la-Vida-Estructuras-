/*
 * @file Ajustes.h
 * @brief Valores que el usuario puede cambiar en la pantalla de Opciones.
 * @details Los guarda la Aplicacion y los lee PantallaSimulacion al iniciar.
 */

#ifndef GUI_AJUSTES_H
#define GUI_AJUSTES_H

struct Ajustes
{
    int agentesCompetencia = 5;    // bichitos del Escenario 1 (competencia por 1 comida)
    float duracionEstacion = 10.0f; // segundos que dura cada estación
};

#endif
