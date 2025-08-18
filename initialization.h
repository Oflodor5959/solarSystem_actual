#ifndef INITIALIZATION_H
#define INITIALIZATION_H

#include <sqlite3.h>
#include "planetas.h"

// Prototipos de funciones de inicialización
void cargar_planetas(sqlite3* db, Planeta planetas[], int *n);
void generar_manchas(ManchasPlaneta manchas[], Planeta planetas[], int n);
void inicializar_estrellas(float estrellas_x[], int estrellas_y[], int ancho, int alto);
void dibujar_orbita(Vector3 centro, float radio, Color color);

#endif