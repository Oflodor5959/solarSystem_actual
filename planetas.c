#include "planetas.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define DISTANCIA_MIN 50.0f   // ejemplo: 50 millones km
#define DISTANCIA_MAX 6000.0f // ejemplo: 6000 millones km

// Callback para contar resultados
int contar_callback(void* data, int argc, char** argv, char** azColName) {
    *(int*)data = atoi(argv[0]);
    return 0;
}

void agregar_planeta(sqlite3* db) {
    Planeta p;
    char input[64];
    printf("--- AGREGAR PLANETA ---\n");

    printf("Nombre (o SALIR para cancelar): ");
    scanf("%s", input);
    if (strcmp(input, "SALIR") == 0) {
        printf("Operacion cancelada.\n");
        return;
    }
    strcpy(p.nombre, input);

    printf("Radio (km) (o SALIR para cancelar): ");
    scanf("%s", input);
    if (strcmp(input, "SALIR") == 0) {
        printf("Operacion cancelada.\n");
        return;
    }
    if (sscanf(input, "%f", &p.radio_km) != 1) {
        printf("Entrada invalida. Operacion cancelada.\n");
        return;
    }
    if (p.radio_km == -1) {
        printf("Operacion cancelada.\n");
        return;
    }

    printf("Velocidad orbital (km/s) (o SALIR para cancelar): ");
    scanf("%s", input);
    if (strcmp(input, "SALIR") == 0) {
        printf("Operación cancelada.\n");
        return;
    }
    if (sscanf(input, "%f", &p.velocidad_orbital) != 1) {
        printf("Entrada invalida. Operacion cancelada.\n");
        return;
    }
    if (p.velocidad_orbital == -1) {
        printf("Operacion cancelada.\n");
        return;
    }

    do {
        printf("Distancia al Sol (millones km, %.1f - %.1f, o SALIR para cancelar): ", DISTANCIA_MIN, DISTANCIA_MAX);
        scanf("%s", input);
        if (strcmp(input, "SALIR") == 0) {
            printf("Operacion cancelada.\n");
            return;
        }
        if (sscanf(input, "%f", &p.distancia_sol) != 1) {
            printf("Entrada invalida. Operacion cancelada.\n");
            return;
        }
        if (p.distancia_sol == -1) {
            printf("Operacion cancelada.\n");
            return;
        }
        if (p.distancia_sol < DISTANCIA_MIN || p.distancia_sol > DISTANCIA_MAX)
            printf("Distancia fuera de rango.\n");
    } while (p.distancia_sol < DISTANCIA_MIN || p.distancia_sol > DISTANCIA_MAX);

    for (int i = 0; i < MAX_COLORES; i++) {
        printf("Color #%d (ejemplo: RED, BLUE, GREEN, YELLOW, BLACK, WHITE, o SALIR para cancelar): ", i+1);
        scanf("%s", input);
        if (strcmp(input, "SALIR") == 0) {
            printf("Operacion cancelada.\n");
            return;
        }
        strcpy(p.colores[i], input);
    }

    do {
        printf("Numero de lunas (0-%d, o SALIR para cancelar): ", MAX_LUNAS);
        scanf("%s", input);
        if (strcmp(input, "SALIR") == 0) {
            printf("Operación cancelada.\n");
            return;
        }
        if (sscanf(input, "%d", &p.num_lunas) != 1) {
            printf("Entrada inalida. Operacion cancelada.\n");
            return;
        }
        if (p.num_lunas == -1) {
            printf("Operacion cancelada.\n");
            return;
        }
        if (p.num_lunas < 0 || p.num_lunas > MAX_LUNAS)
            printf("Cantidad fuera de rango.\n");
    } while (p.num_lunas < 0 || p.num_lunas > MAX_LUNAS);

    // --- Aquí termina el bloque de entrada de datos ---

    // Validar órbita contra los planetas existentes
    sqlite3_stmt *stmt;
    const char *sql = "SELECT radio_km, distancia_sol, nombre FROM planetas;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        int colision = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            float radio_existente = (float)sqlite3_column_double(stmt, 0);
            float distancia_existente = (float)sqlite3_column_double(stmt, 1);
            const char *nombre_existente = (const char*)sqlite3_column_text(stmt, 2);

            float radio_nuevo = 2.0f + p.radio_km * 0.001f;
            float radio_otro = 2.0f + radio_existente * 0.001f;
            float distancia = fabsf(p.distancia_sol - distancia_existente) * 0.1f;

            if (distancia < (radio_nuevo + radio_otro)) {
                printf("Error: La órbita de '%s' esta demasiado cerca de '%s'.\n", p.nombre, nombre_existente);
                colision = 1;
                break;
            }
        }
        sqlite3_finalize(stmt);
        if (colision) return; // No agrega el planeta si hay colisión
    }

    char sql_insert[512];
    snprintf(sql_insert, sizeof(sql_insert),
        "INSERT INTO planetas (nombre, radio_km, velocidad_orbital, distancia_sol, color1, color2, color3, num_lunas) "
        "VALUES ('%s', %f, %f, %f, '%s', '%s', '%s', %d);",
        p.nombre, p.radio_km, p.velocidad_orbital, p.distancia_sol,
        p.colores[0], p.colores[1], p.colores[2], p.num_lunas);

    if (sqlite3_exec(db, sql_insert, 0, 0, 0) == SQLITE_OK)
        printf("Planeta agregado.\n");
    else
        printf("Error al agregar planeta.\n");
}

void editar_planeta(sqlite3* db) {
    char nombre[32];
    printf("--- EDITAR PLANETA ---\n");
    printf("Nombre del planeta a editar: "); scanf("%s", nombre);

    // Verificar si el planeta existe
    char sql_check[128];
    snprintf(sql_check, sizeof(sql_check), "SELECT COUNT(*) FROM planetas WHERE nombre='%s';", nombre);
    int count = 0;
    sqlite3_exec(db, sql_check, contar_callback, &count, 0);

    if (count == 0) {
        printf("Planeta no encontrado.\n");
        return;
    }

    // Obtener datos actuales
    Planeta p;
    strcpy(p.nombre, nombre);
    sqlite3_stmt *stmt;
    char sql_get[256];
    snprintf(sql_get, sizeof(sql_get), "SELECT radio_km, velocidad_orbital, distancia_sol, color1, color2, color3, num_lunas FROM planetas WHERE nombre='%s';", nombre);
    if (sqlite3_prepare_v2(db, sql_get, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            p.radio_km = (float)sqlite3_column_double(stmt, 0);
            p.velocidad_orbital = (float)sqlite3_column_double(stmt, 1);
            p.distancia_sol = (float)sqlite3_column_double(stmt, 2);
            strcpy(p.colores[0], (const char*)sqlite3_column_text(stmt, 3));
            strcpy(p.colores[1], (const char*)sqlite3_column_text(stmt, 4));
            strcpy(p.colores[2], (const char*)sqlite3_column_text(stmt, 5));
            p.num_lunas = sqlite3_column_int(stmt, 6);
        }
        sqlite3_finalize(stmt);
    }

    int opcion;
    char input[64];
    do {
        printf("\n¿Que deseas editar?\n");
        printf("1. Radio (actual: %.1f km)\n", p.radio_km);
        printf("2. Velocidad orbital (actual: %.2f km/s)\n", p.velocidad_orbital);
        printf("3. Distancia al Sol (actual: %.1f millones km)\n", p.distancia_sol);
        printf("4. Color principal (actual: %s)\n", p.colores[0]);
        printf("5. Color secundario (actual: %s)\n", p.colores[1]);
        printf("6. Color terciario (actual: %s)\n", p.colores[2]);
        printf("7. Numero de lunas (actual: %d)\n", p.num_lunas);
        printf("0. Guardar y salir\n");
        printf("Opcion: ");
        scanf("%s", input);
        if (sscanf(input, "%d", &opcion) != 1) {
            printf("Entrada invalida.\n");
            continue;
        }

        switch (opcion) {
            case 1:
                printf("Nuevo radio (km, o SALIR para cancelar): ");
                scanf("%s", input);
                if (strcmp(input, "SALIR") == 0) break;
                if (sscanf(input, "%f", &p.radio_km) != 1) {
                    printf("Entrada invalida.\n");
                }
                break;
            case 2:
                printf("Nueva velocidad orbital (km/s, o SALIR para cancelar): ");
                scanf("%s", input);
                if (strcmp(input, "SALIR") == 0) break;
                if (sscanf(input, "%f", &p.velocidad_orbital) != 1) {
                    printf("Entrada invalida.\n");
                }
                break;
            case 3:
                do {
                    printf("Nueva distancia al Sol (millones km, %.1f - %.1f, o SALIR para cancelar): ", DISTANCIA_MIN, DISTANCIA_MAX);
                    scanf("%s", input);
                    if (strcmp(input, "SALIR") == 0) break;
                    if (sscanf(input, "%f", &p.distancia_sol) != 1) {
                        printf("Entrada invalida.\n");
                        continue;
                    }
                    if (p.distancia_sol < DISTANCIA_MIN || p.distancia_sol > DISTANCIA_MAX)
                        printf("Distancia fuera de rango.\n");
                } while (p.distancia_sol < DISTANCIA_MIN || p.distancia_sol > DISTANCIA_MAX);
                break;
            case 4:
                printf("Nuevo color principal (o SALIR para cancelar): ");
                scanf("%s", input);
                if (strcmp(input, "SALIR") == 0) break;
                strcpy(p.colores[0], input);
                break;
            case 5:
                printf("Nuevo color secundario (o SALIR para cancelar): ");
                scanf("%s", input);
                if (strcmp(input, "SALIR") == 0) break;
                strcpy(p.colores[1], input);
                break;
            case 6:
                printf("Nuevo color terciario (o SALIR para cancelar): ");
                scanf("%s", input);
                if (strcmp(input, "SALIR") == 0) break;
                strcpy(p.colores[2], input);
                break;
            case 7:
                do {
                    printf("Nuevo numero de lunas (0-%d, o SALIR para cancelar): ", MAX_LUNAS);
                    scanf("%s", input);
                    if (strcmp(input, "SALIR") == 0) break;
                    if (sscanf(input, "%d", &p.num_lunas) != 1) {
                        printf("Entrada invalida.\n");
                        continue;
                    }
                    if (p.num_lunas < 0 || p.num_lunas > MAX_LUNAS)
                        printf("Cantidad fuera de rango.\n");
                } while (p.num_lunas < 0 || p.num_lunas > MAX_LUNAS);
                break;
            case 0:
                break;
            default:
                printf("Opcion invalida.\n");
        }
    } while (opcion != 0);

    // Validar órbita contra los planetas existentes (excepto el que se está editando)
    const char *sql = "SELECT radio_km, distancia_sol, nombre FROM planetas WHERE nombre != ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, p.nombre, -1, SQLITE_STATIC);
        int colision = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            float radio_existente = (float)sqlite3_column_double(stmt, 0);
            float distancia_existente = (float)sqlite3_column_double(stmt, 1);
            const char *nombre_existente = (const char*)sqlite3_column_text(stmt, 2);

            float radio_nuevo = 2.0f + p.radio_km * 0.001f;
            float radio_otro = 2.0f + radio_existente * 0.001f;
            float distancia = fabsf(p.distancia_sol - distancia_existente) * 0.1f;

            if (distancia < (radio_nuevo + radio_otro)) {
                printf("Error: La orbita de '%s' esta demasiado cerca de '%s'.\n", p.nombre, nombre_existente);
                colision = 1;
                break;
            }
        }
        sqlite3_finalize(stmt);
        if (colision) return; // No actualiza el planeta si hay colisión
    }

    char sql_update[512];
    snprintf(sql_update, sizeof(sql_update),
        "UPDATE planetas SET radio_km=%f, velocidad_orbital=%f, distancia_sol=%f, "
        "color1='%s', color2='%s', color3='%s', num_lunas=%d WHERE nombre='%s';",
        p.radio_km, p.velocidad_orbital, p.distancia_sol,
        p.colores[0], p.colores[1], p.colores[2], p.num_lunas, p.nombre);

    if (sqlite3_exec(db, sql_update, 0, 0, 0) == SQLITE_OK)
        printf("Planeta actualizado.\n");
    else
        printf("Error al actualizar planeta.\n");
}

void eliminar_planeta(sqlite3* db) {
    printf("--- ELIMINAR PLANETA ---\n");

    // Mostrar lista de planetas
    sqlite3_stmt *stmt;
    const char *sql = "SELECT nombre FROM planetas;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        printf("Planetas disponibles:\n");
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            printf(" - %s\n", sqlite3_column_text(stmt, 0));
        }
        sqlite3_finalize(stmt);
    }

    char nombre[32];
    printf("Escribe el nombre del planeta a eliminar (o SALIR para cancelar): ");
    scanf("%s", nombre);

    if (strcmp(nombre, "SALIR") == 0) {
        printf("Operación cancelada.\n");
        return;
    }

    char sql_del[128];
    snprintf(sql_del, sizeof(sql_del), "DELETE FROM planetas WHERE nombre='%s';", nombre);

    if (sqlite3_exec(db, sql_del, 0, 0, 0) == SQLITE_OK)
        printf("Planeta eliminado.\n");
    else
        printf("Error al eliminar planeta.\n");
}

void listar_planetas(sqlite3* db) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT nombre, radio_km, velocidad_orbital, distancia_sol, color1, color2, color3, num_lunas FROM planetas;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        printf("\n--- LISTA DE PLANETAS ---\n");
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("Nombre: %s | Radio: %.1f km | Vel. Orbital: %.2f km/s | Distancia: %.1f Mkm | Colores: %s %s %s | Lunas: %d\n",
                sqlite3_column_text(stmt, 0),
                sqlite3_column_double(stmt, 1),
                sqlite3_column_double(stmt, 2),
                sqlite3_column_double(stmt, 3),
                sqlite3_column_text(stmt, 4),
                sqlite3_column_text(stmt, 5),
                sqlite3_column_text(stmt, 6),
                sqlite3_column_int(stmt, 7));
        }
        sqlite3_finalize(stmt);
    }
}