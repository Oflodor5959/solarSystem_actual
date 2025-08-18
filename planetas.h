// Datos del archivo
// Autor: [Rodolfo Beltre]
// Matricula: [2025-0444]
// Fecha: [16/8/2025]
// Descripción: [CRUD simulador de sistemas solares]
// planetas.h

#ifndef PLANETAS_H
#define PLANETAS_H

#include <sqlite3.h>
#include <raylib.h>

#define MAX_COLORES 3
#define MAX_LUNAS 8

typedef struct {
    Vector3 secundarias[124];
    Vector3 terciarias[32];
} ManchasPlaneta;

typedef struct {
    char nombre[32];
    float radio_km;
    float velocidad_orbital;
    float distancia_sol;
    char colores[MAX_COLORES][16]; // Ahora guarda el nombre del color
    int num_lunas;
} Planeta;

void agregar_planeta(sqlite3* db);
void editar_planeta(sqlite3* db);
void eliminar_planeta(sqlite3* db);
void listar_planetas(sqlite3* db);

#endif

// Compile with:
// gcc main.c dashboard.c planetas.c simulador.c source.c initialialization.c -o solar.exe -lsqlite3 -lraylib -lopengl32 -lgdi32 -lwinmm
// Run with:
// solar.exe