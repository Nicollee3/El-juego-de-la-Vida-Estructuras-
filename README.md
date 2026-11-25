# El-juego-de-la-Vida-Estructuras-

## Compilar y ejecutar en VS Code

Abre la carpeta del proyecto en VS Code e instala la extensión **CMake Tools**
si VS Code la recomienda. El proyecto se configura automáticamente con el
preset `default` y genera los archivos en `out/build`, una ruta relativa que
sigue funcionando aunque cambie el nombre o la ubicación de la carpeta.

- `Ctrl+Shift+B` configura y compila el proyecto.
- Para ejecutar, selecciona `ElJuegoDeLaVida` como objetivo de lanzamiento si
  aún no está seleccionado y usa el botón triangular de CMake Tools en la barra
  de estado.

## Genes estáticos y mutaciones

- Los fundadores empiezan sin genes estáticos.
- Cada descendiente tiene un `5%` de probabilidad de mutar a un gen estático que
  todavía no haya aparecido en la simulación.
- El primer portador de cada gen inicia su linaje en el árbol genealógico y
  bloquea nuevas mutaciones independientes de ese gen. Sus descendientes sí
  pueden heredarlo.

## Las 3 formas en las que mueren los bichitos

Cada vez que un bichito muere (por cualquiera de las 3 causas) se elimina del `ArregloDinamico` en **O(1)** mediante *swap-and-pop* y se actualiza su índice en el `Diccionario` (`indicesBichitos`).

1. **Muerte por Hambre (`vida <= 0`):**
   - La vida se desgasta según la estación actual y se recupera comiendo.
   - Al llegar a `0`, el bichito muere, deja comida (cadáver) y sale del arreglo en O(1).
   - Aplica para **todos** los bichitos, **incluidos los inmortales** (es la única forma en que un inmortal puede morir).

2. **Muerte por Vejez (`edadActual >= edadMaxima`) — Min-Heap (`ColaPrioridadVejez`):**
   - Cada bichito tiene `edadActual` (avanza a `2.5 años/s`) y `edadMaxima` predestinada por su linaje (`20` a `400` años; los hijos heredan el promedio de sus padres con `±15%` de variación).
   - Al nacer un bichito mortal se inserta en una **Cola con Prioridad (Min-Heap)** ordenada por el instante exacto en que cumplirá su `edadMaxima` (`O(log n)`).
   - Cada frame se revisa la raíz en **O(1)**: si ya cumplió su edad máxima, se extrae (`O(log n)`), deja comida y muere en O(1) con *swap-and-pop*. Los inmortales no entran a esta cola.

3. **Muerte por Desastres Naturales y Sobrepoblación (La Parca) — `ColaPrioridad` de 3 niveles:**
   - Al dispararse un desastre (aleatorio cada `45–80 s`, manual desde la barra lateral o automático por sobrepoblación `> 160`), se clasifica la población mortal en 3 niveles fijos (**O(1)**):
     - **Nivel 1 (mueren primero):** poca vida (`< 25`), enfermos o viejitos (`>= 75%` de su `edadMaxima`).
     - **Nivel 2 (intermedio):** jóvenes y adultos sanos.
     - **Nivel 3 (mueren al final):** inmunes y recién nacidos (`edad <= 10`).
   - Impacto por evento: **Meteoritos** (`20`), **Lluvia Ácida** (`30`), **Erupción Volcánica** (`60`), **Tormenta Solar** (`100`) y **Sobrepoblación automática** (`> 160` reduce a `140` priorizando a los más viejos). Los inmortales están exentos.
