#ifndef SOURCE_H
#define SOURCE_H

#include <sqlite3.h>
#include "planetas.h"

void mostrar_menu_edicion(const Planeta* p);
void editar_campo(Planeta* p, int opcion);
int validar_orbita(sqlite3* db, const Planeta* p, const char* excluir_nombre);
int obtener_input(const char* prompt, char* buffer, size_t size);
int procesar_input_float(const char* input, float* valor);
int procesar_input_int(const char* input, int* valor);

#define DISTANCIA_MIN 160.0f
#define DISTANCIA_MAX 6000.0f
#define INPUT_BUFFER_SIZE 64
#define SQL_BUFFER_SIZE 512

#endif // SOURCE_H