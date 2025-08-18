// Datos del archivo
// Autor: [Rodolfo Beltre]
// Matricula: [2025-0444]
// Fecha: [16/8/2025]
// Descripción: [CRUD simulador de sistemas solares]
// planetas.c

#include <raylib.h>
#include "planetas.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "simulador.h"
#include "source.h"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOMINMAX
#define NOUSER
#include <windows.h>
#endif

#define DISTANCIA_MAX 6000.0f
#define INPUT_BUFFER_SIZE 64
#define SQL_BUFFER_SIZE 512

int validar_orbita(sqlite3* db, const Planeta* p, const char* excluir_nombre) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT radio_km, distancia_sol, nombre FROM planetas WHERE nombre != ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }
    
    sqlite3_bind_text(stmt, 1, excluir_nombre, -1, SQLITE_STATIC);
    int colision = 0;
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        float radio_existente = (float)sqlite3_column_double(stmt, 0);
        float distancia_existente = (float)sqlite3_column_double(stmt, 1);
        const char *nombre_existente = (const char*)sqlite3_column_text(stmt, 2);

        float radio_nuevo = 2.0f + p->radio_km * 0.001f;
        float radio_otro = 2.0f + radio_existente * 0.001f;
        float distancia = fabsf(p->distancia_sol - distancia_existente) * 0.1f;

        if (distancia < (radio_nuevo + radio_otro)) {
            printf("\nError: La orbita de '%s' esta demasiado cerca de '%s'.\n", 
                  p->nombre, nombre_existente);
            colision = 1;
            break;
        }
    }
    
    sqlite3_finalize(stmt);
    return colision;
}

void mostrar_menu_edicion(const Planeta* p) {

    system("cls"); system("clear"); 
    printf("Cargando menu de edicion...");
#ifdef _WIN32
    Sleep(600);
#else
    usleep(600000);
#endif
    system("cls"); system("clear");
    printf("\r");
    printf("\n====================================\n");
    printf("   MENU EDICION DE PLANETA\n");
    printf("====================================\n");
    printf("1. Radio (actual: %.1f km)\n", p->radio_km);
    printf("2. Velocidad orbital (actual: %.2f km/s)\n", p->velocidad_orbital);
    printf("3. Distancia al Sol (actual: %.1f millones km)\n", p->distancia_sol);
    printf("4. Color principal (actual: %s)\n", p->colores[0]);
    printf("5. Color secundario (actual: %s)\n", p->colores[1]);
    printf("6. Color terciario (actual: %s)\n", p->colores[2]);
    printf("7. Numero de lunas (actual: %d)\n", p->num_lunas);
    printf("0. Guardar y salir\n");
    printf("====================================\n");
    printf("Opcion: ");
}

void editar_campo(Planeta* p, int opcion) {
    char input[INPUT_BUFFER_SIZE];
    float distancia_minima;
    
    switch (opcion) {
        case 1:
            obtener_input("Nuevo radio (km, o SALIR para cancelar): ", input, sizeof(input));
            if (procesar_input_float(input, &p->radio_km) <= 0) break;
            break;
            
        case 2:
            obtener_input("Nueva velocidad orbital (km/s, o SALIR para cancelar): ", input, sizeof(input));
            if (procesar_input_float(input, &p->velocidad_orbital) <= 0) break;
            break;
            
        case 3:
            distancia_minima = DISTANCIA_MIN + (p->radio_km * 0.007f / 0.1f);
            do {
                printf("Nueva distancia al Sol (millones km, %.1f - %.1f, o SALIR para cancelar): ", 
                      distancia_minima, DISTANCIA_MAX);
                obtener_input("", input, sizeof(input));
                
                int resultado = procesar_input_float(input, &p->distancia_sol);
                if (resultado <= 0) break;
                
                if (p->distancia_sol < distancia_minima || p->distancia_sol > DISTANCIA_MAX) {
                    printf("Distancia fuera de rango.\n");
                }
            } while (p->distancia_sol < distancia_minima || p->distancia_sol > DISTANCIA_MAX);
            break;
            
        case 4:
    obtener_input("Nuevo color principal (o SALIR para cancelar): ", input, sizeof(input));
    if (strcmp(input, "SALIR") == 0) {
        printf("Operacion cancelada.\n");
        break;
    }
    if (!es_color_valido(input)) {
        printf("Color invalido. Solo se permiten colores validos.\n");
        break;
    }
    strcpy(p->colores[0], input);
    break;

case 5:
    obtener_input("Nuevo color secundario (o SALIR para cancelar): ", input, sizeof(input));
    if (strcmp(input, "SALIR") == 0) {
        printf("Operacion cancelada.\n");
        break;
    }
    if (!es_color_valido(input)) {
        printf("Color invalido. Solo se permiten colores validos.\n");
        break;
    }
    strcpy(p->colores[1], input);
    break;

case 6:
    obtener_input("Nuevo color terciario (o SALIR para cancelar): ", input, sizeof(input));
    if (strcmp(input, "SALIR") == 0) {
        printf("Operacion cancelada.\n");
        break;
    }
    if (!es_color_valido(input)) {
        printf("Color invalido. Solo se permiten colores validos.\n");
        break;
    }
    strcpy(p->colores[2], input);
case 7:
         do {
            obtener_input("Nuevo numero de lunas (0-8, o [SALIR] para cancelar): ", input, sizeof(input));
                if (strcmp(input, "SALIR") == 0) {
                printf("Operacion cancelada.\n");
                break;
            }
            if (strlen(input) == 0) {
                printf("El campo no puede estar vacio. Intente nuevamente.\n");
                continue;
            }
            int es_numero = 1;
               for (int i = 0; input[i] != '\0'; i++) {
                if (input[i] < '0' || input[i] > '9') {
                    es_numero = 0;
                    break;
                   }
            }
            if (!es_numero) {
                printf("Solo se permiten numeros enteros. Intente nuevamente.\n");
                continue;
            }
            int resultado = procesar_input_int(input, &p->num_lunas);
            if (resultado <= 0) {
                printf("Operacion cancelada.\n");
                break;
            }
            if (p->num_lunas < 0 || p->num_lunas > MAX_LUNAS) {
                printf("Numero fuera de rango. Intente nuevamente.\n");
                continue;
            }
            break;
        } while (1);
        break;

case 0:
    break;
    }
}

// Compile with:
// gcc main.c dashboard.c planetas.c simulador.c source.c initialialization.c -o solar.exe -lsqlite3 -lraylib -lopengl32 -lgdi32 -lwinmm
// Run with:
// solar.exe