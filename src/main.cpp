#include <iostream>
#include <cstdlib>
#include <ctime>
#include "PilaDinamica.h"
#include "MotorGrafico.h"

int main()
{
    std::srand(static_cast<unsigned int>(std::time(0)));

    std::cout << "========================================\n";
    std::cout << "  SIMULADOR DE VIDA 2D - MOTOR GRAFICO \n";
    std::cout << "========================================\n";
    std::cout << "Controles:\n";
    std::cout << " [1] Cargar Escenario Competencia (N agentes por 1 comida)\n";
    std::cout << " [2] Cargar Escenario Reproduccion (2 agentes cercanos)\n";
    std::cout << " [Click Izquierdo] Generar comida en la posicion del cursor\n";
    std::cout << "========================================\n\n";

    PilaDinamica poblacion;

    // Crear el motor grafico de 1000x700
    MotorGrafico motor(poblacion, 1000, 700);

    // Iniciar con el Escenario 1 por defecto (5 agentes compitiendo)
    motor.cargarEscenarioCompetencia(5);

    // Iniciar el bucle principal de la simulacion
    motor.ejecutar();

    return 0;
}