#include <iostream>
#include <iostream>
#include <cstdlib> // Para rand()
#include <ctime>   // Para time(), osea la semilla.
#include "BICHITO.H"
#include "PILA_DINAMICA.H"
#include "COMIDA.H"
#include "COLAS_CON_PRIORIDAD.H"

int main()
{
    // Inicializar la semilla aleatoria basada en el tiempo actual
    std::srand(std::time(0));

    std::cout << "SIMULACION DE BICHITOS - DEMOSTRACION\n";
    PilaDinamica poblacion; // INICIALIZAR LA PILA
    std::cout << "Pila dinamica inicializada correctamente.\n";
    std::cout << "-> Capacidad inicial y longitud actual: " << poblacion.longitud() << "\n\n";

    // GENERACION AUTOMATICA DE BICHITOS

    int cantidad_a_generar = 100;
    std::cout << "--- 1. GENERANDO " << cantidad_a_generar << " BICHITOS ALEATORIAMENTE ---\n";

    for (int i = 1; i <= cantidad_a_generar; i++)
    {
        int vida_aleatoria = std::rand() % 101; // Genera un número de 0 a 100
        poblacion.generar(Bichito(i, vida_aleatoria));
    }
    std::cout << "Poblacion generada con exito. Total actual: " << poblacion.longitud() << " bichitos.\n";
    std::cout << "Listado de la poblacion inicial:\n";
    for (int i = 0; i < poblacion.longitud(); i++)
    {
        Bichito b = poblacion.obtener(i);
        std::cout << "  [Indice " << i << "] -> ID: " << b.id << " | Vida: " << b.vida << "\n";
    }
    std::cout << "\n";

    // 4. MATAR UN BICHITO
    {
        std::cout << "--- 3. SIMULANDO MUERTE ---\n";
        int indice_a_eliminar = 3;
        Bichito que_morira = poblacion.obtener(indice_a_eliminar);
        std::cout << "El bichito en el indice " << indice_a_eliminar << " (ID: " << que_morira.id << ", Vida: " << que_morira.vida << ") ha muerto.\n";
        poblacion.morir(indice_a_eliminar);

        std::cout << "Poblacion tras la eliminacion (" << poblacion.longitud() << " bichitos):\n";
        for (int i = 0; i < poblacion.longitud(); i++)
        {
            Bichito b = poblacion.obtener(i);
            std::cout << "  [Indice " << i << "] -> ID: " << b.id << " | Vida: " << b.vida << "\n";
        }
        std::cout << "\n";
    }

    std::cout << "\n==================================================\n";
    std::cout << "===       DEMOSTRACION FINALIZADA CON EXITO    ===\n";
    std::cout << "==================================================\n";

    return 0;
}
