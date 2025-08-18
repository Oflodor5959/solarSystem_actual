
#include <raylib.h>
#include "dashboard.h"
#include "planetas.h"
#include "simulador.h"
#include <stdio.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOMINMAX
#define NOUSER
#include <windows.h>
#endif

void show_dashboard(sqlite3* db) {
    int opcion;
char input[16];
do {
    system("cls");
    printf("====================================\n");
    printf("      PANEL SISTEMA SOLAR\n");
    printf("====================================\n");
    printf("[1] Agregar planeta\n");
    printf("[2] Editar planeta\n");
    printf("[3] Eliminar planeta\n");
    printf("[4] Listar planetas\n");
    printf("[5] Iniciar simulacion\n");
    printf("[0] Salir\n");
    printf("====================================\n");
    printf("Opcion: ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = 0;

    // Validar que la entrada sea solo números
    int es_numero = 1;
    for (int i = 0; input[i] != '\0'; i++) {
        if (input[i] < '0' || input[i] > '9') {
            es_numero = 0;
            break;
        }
    }

    if (!es_numero || sscanf(input, "%d", &opcion) != 1) {
        printf("\nOpcion invalida. Debe ingresar solo numeros.\n");
        Sleep(1000);
        continue;
    }

    if (opcion == 0) {
        printf("\nSaliendo...\n");
        Sleep(800);
        system("cls");
        break;
    }
    if (opcion < 0 || opcion > 5) {
        printf("\nOpcion invalida. Debe ingresar un numero entre 0 y 5.\n");
        Sleep(3000);
        continue;
    }

    printf("\nCargando...\n");
    Sleep(500);

    switch(opcion) {
        case 1: agregar_planeta(db); break;
        case 2: editar_planeta(db); break;
        case 3: eliminar_planeta(db); break;
        case 4: listar_planetas(db); break;
        case 5: iniciar_simulacion(db); break;
    }
} while(1);
}