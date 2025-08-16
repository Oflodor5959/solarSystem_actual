#include <sqlite3.h>
#include "dashboard.h"
#include <stdio.h>
#include "planetas.h"

void mostrar_tabla_planetas(Planeta planetas[], int n) {
    printf(" _______________________________________________________________________________________\n");
    printf("| %-15s | %-10s | %-15s | %-10s | %-10s |\n", "Nombre", "Radio (km)", "Distancia (M km)", "Lunas", "Color");
    printf("|-----------------|------------|-----------------|------------|------------|\n");
    for (int i = 0; i < n; i++) {
        printf("| %-15s | %-10.0f | %-15.0f | %-10d | %-10s |\n",
            planetas[i].nombre,
            planetas[i].radio_km,
            planetas[i].distancia_sol,
            planetas[i].num_lunas,
            planetas[i].colores[0]
        );
    }
    printf("|_______________________________________________________________________________________|\n");
}

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