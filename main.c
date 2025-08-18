// Datos del archivo
// Autor: [Rodolfo Beltre]
// Matricula: [2025-0444]
// Fecha: [16/8/2025]
// Descripción: [CRUD simulador de sistemas solares]
// main.c

#include <sqlite3.h>
#include "dashboard.h"
#include <stdio.h>
#include "planetas.h"

int main() {
    sqlite3 *db;
    if (sqlite3_open("planetas.db", &db) != SQLITE_OK) {
        printf("No se pudo abrir la base de datos.\n");
        return 1;
    }

    // Crear tabla si no existe
    const char *sql = "CREATE TABLE IF NOT EXISTS planetas("
        "nombre TEXT PRIMARY KEY,"
        "radio_km REAL,"
        "velocidad_orbital REAL,"
        "distancia_sol REAL,"
        "color1 TEXT,"
        "color2 TEXT,"
        "color3 TEXT,"
        "num_lunas INTEGER);";
    sqlite3_exec(db, sql, 0, 0, 0);

    // Mostrar dashboard en terminal
    show_dashboard(db);

    sqlite3_close(db);
    return 0;
}

// Compile in CMD with:
// gcc main.c dashboard.c planetas.c simulador.c source.c initialialization.c -o solar.exe -lsqlite3 -lraylib -lopengl32 -lgdi32 -lwinmm
// Run with:
// solar.exe
//```