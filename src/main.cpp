#include <cstdlib>
#include <ctime>
#include "gui/Aplicacion.h"

int main()
{
    std::srand(static_cast<unsigned int>(std::time(0)));

    // Aplicacion crea la ventana, muestra el menú principal y lleva el bucle del programa.
    // La población y el motor ahora los crea PantallaSimulacion cuando se inicia la simulación.
    Aplicacion app;
    app.ejecutar();

    return 0;
}
