// Datos del archivo
// Autor: [Rodolfo Beltre]
// Matricula: [2025-0444]
// Fecha: [16/8/2025]
// Descripción: [CRUD simulador de sistemas solares]
// simulador.h

#ifndef SIMULADOR_H
#define SIMULADOR_H

#include <sqlite3.h>

void iniciar_simulacion(sqlite3* db);
void pedir_color(char* color);
int es_color_valido(const char* color);

#endif