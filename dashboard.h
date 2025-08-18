#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <sqlite3.h>

void show_dashboard(sqlite3* db);

#endif

// Compile in CMD with:
// gcc main.c dashboard.c planetas.c simulador.c source.c initialialization.c -o solar.exe -lsqlite3 -lraylib -lopengl32 -lgdi32 -lwinmm
// Run with:
// solar.exe