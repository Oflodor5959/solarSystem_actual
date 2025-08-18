// Datos del archivo
// Autor: [Rodolfo Beltre]
// Matricula: [2025-0444]
// Fecha: [16/8/2025]
// Descripción: [CRUD simulador de sistemas solares]
// simulador.c

#include <raylib.h>
#include "simulador.h"
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include "planetas.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>

#define MAX_PLANETAS 16
#define NUM_ESTRELLAS 750
#define MAX_LUNAS 8

// --------------------------
// INITIALIZATION FUNCTIONS
// --------------------------
void cargar_planetas(sqlite3* db, Planeta planetas[], int *n) {
    *n = 0;
    sqlite3_stmt *stmt;
    const char *sql = "SELECT nombre, radio_km, velocidad_orbital, distancia_sol, "
                     "color1, color2, color3, num_lunas FROM planetas;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW && *n < MAX_PLANETAS) {
            strcpy(planetas[*n].nombre, (const char*)sqlite3_column_text(stmt, 0));
            planetas[*n].radio_km = (float)sqlite3_column_double(stmt, 1);
            planetas[*n].velocidad_orbital = (float)sqlite3_column_double(stmt, 2);
            planetas[*n].distancia_sol = (float)sqlite3_column_double(stmt, 3);
            strcpy(planetas[*n].colores[0], (const char*)sqlite3_column_text(stmt, 4));
            strcpy(planetas[*n].colores[1], (const char*)sqlite3_column_text(stmt, 5));
            strcpy(planetas[*n].colores[2], (const char*)sqlite3_column_text(stmt, 6));
            planetas[*n].num_lunas = sqlite3_column_int(stmt, 7);
            (*n)++;
        }
        void mostrar_tabla_planetas(Planeta planetas[], int n);
        sqlite3_finalize(stmt);
    }
}

void generar_manchas(ManchasPlaneta manchas[], Planeta planetas[], int n) {
    srand((unsigned int)time(NULL));
    
    for (int i = 0; i < n; i++) {
        float radio = 2.0f + planetas[i].radio_km * 0.001f;
        
        for (int m = 0; m < 124; m++) {
            float theta = ((float)rand() / RAND_MAX) * 2 * PI;
            float phi = ((float)rand() / RAND_MAX) * PI;
            float factor = 0.93f + ((float)rand() / RAND_MAX) * 0.04f;
            
            manchas[i].secundarias[m].x = cosf(theta) * sinf(phi) * radio * factor;
            manchas[i].secundarias[m].y = sinf(theta) * sinf(phi) * radio * factor;
            manchas[i].secundarias[m].z = cosf(phi) * radio * factor;
        }
        
        for (int m = 0; m < 32; m++) {
            float theta = ((float)rand() / RAND_MAX) * 2 * PI;
            float phi = ((float)rand() / RAND_MAX) * PI;
            float factor = 0.93f + ((float)rand() / RAND_MAX) * 0.04f;
            
            manchas[i].terciarias[m].x = cosf(theta) * sinf(phi) * radio * factor;
            manchas[i].terciarias[m].y = sinf(theta) * sinf(phi) * radio * factor;
            manchas[i].terciarias[m].z = cosf(phi) * radio * factor;
        }
    }
}

void inicializar_estrellas(float estrellas_x[], int estrellas_y[], int ancho, int alto) {
    srand(42);
    for (int s = 0; s < NUM_ESTRELLAS; s++) {
        estrellas_x[s] = rand() % ancho;
        estrellas_y[s] = rand() % alto;
    }
}

// --------------------------
// DRAWING FUNCTIONS
// --------------------------
void dibujar_orbita(Vector3 centro, float radio, Color color) {
    const int segmentos = 64;
    for (int i = 0; i < segmentos; i++) {
        float ang1 = (2 * PI * i) / segmentos;
        float ang2 = (2 * PI * (i + 1)) / segmentos;
        Vector3 p1 = {
            centro.x + cosf(ang1) * radio,
            centro.y,
            centro.z + sinf(ang1) * radio
        };
        Vector3 p2 = {
            centro.x + cosf(ang2) * radio,
            centro.y,
            centro.z + sinf(ang2) * radio
        };
        DrawLine3D(p1, p2, color);
    }
}

// Compile with:
// gcc main.c dashboard.c planetas.c simulador.c source.c initialialization.c -o solar.exe -lsqlite3 -lraylib -lopengl32 -lgdi32 -lwinmm
// Run with:
// solar.exe