
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
    #ifdef _WIN32
    system("cls"); Sleep(400);
    #else
    system("clear"); usleep(400000);
    #endif
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
        #ifdef _WIN32
        Sleep(1000);
        #else
        usleep(1000000);
        #endif
        continue;
    }

    if (opcion == 0) {
        printf("\nSaliendo...\n");
        #ifdef _WIN32
        Sleep(800); system("cls");
        #else
        usleep(800000); system("clear");
        #endif
        break;
    }
    if (opcion < 0 || opcion > 5) {
        printf("\nOpcion invalida. Debe ingresar un numero entre 0 y 5.\n");
        #ifdef _WIN32
        Sleep(3000);
        #else
        usleep(3000000);
        #endif
        continue;
    }

    printf("\nCargando...\n");
    #ifdef _WIN32
    Sleep(500);
    #else
    usleep(500000);
    #endif

    switch(opcion) {
        case 1: agregar_planeta(db); break;
        case 2: editar_planeta(db); break;
        case 3: eliminar_planeta(db); break;
        case 4: listar_planetas(db); break;
        case 5: iniciar_simulacion(db); break;
    }
} while(1);
}

// Compile in CMD with:
// gcc main.c dashboard.c planetas.c simulador.c source.c initialialization.c -o solar.exe -lsqlite3 -lraylib -lopengl32 -lgdi32 -lwinmm
// Run with:
// solar.exe