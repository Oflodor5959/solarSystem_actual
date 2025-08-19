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

// Constants
// DISTANCIA_MIN representa la distancia mínima permitida entre un planeta y el Sol en millones de km.
// El valor 160.0f fue elegido para evitar solapamientos y asegurar una separación adecuada en el simulador.
#define DISTANCIA_MIN 160.0f
#define DISTANCIA_MAX 7000.0f
#define INPUT_BUFFER_SIZE 64
#define SQL_BUFFER_SIZE 512

// --------------------------
// DATABASE OPERATIONS
// --------------------------

static void obtener_datos_planeta(sqlite3* db, const char* nombre, Planeta* p) {
    sqlite3_stmt *stmt;
    char sql_get[SQL_BUFFER_SIZE];
    
    snprintf(sql_get, sizeof(sql_get), 
        "SELECT radio_km, velocidad_orbital, distancia_sol, color1, color2, color3, num_lunas "
        "FROM planetas WHERE nombre='%s';", nombre);
        
    if (sqlite3_prepare_v2(db, sql_get, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            p->radio_km = (float)sqlite3_column_double(stmt, 0);
            p->velocidad_orbital = (float)sqlite3_column_double(stmt, 1);
            p->distancia_sol = (float)sqlite3_column_double(stmt, 2);
            strcpy(p->colores[0], (const char*)sqlite3_column_text(stmt, 3));
            strcpy(p->colores[1], (const char*)sqlite3_column_text(stmt, 4));
            strcpy(p->colores[2], (const char*)sqlite3_column_text(stmt, 5));
            p->num_lunas = sqlite3_column_int(stmt, 6);
        }
        sqlite3_finalize(stmt);
    }
}

// --------------------------
// INPUT HANDLING
// --------------------------

int obtener_input(const char* prompt, char* buffer, size_t size) {
    printf("%s", prompt);
    if (fgets(buffer, size, stdin) == NULL) {
        return 0;
    }
    buffer[strcspn(buffer, "\n")] = '\0';
    return 1;
}

int procesar_input_float(const char* input, float* valor) {
    if (stricmp(input, "SALIR") == 0) return 0;
    if (sscanf(input, "%f", valor) != 1) {
        printf("Entrada invalida.\n");
        return -1;
    }
    return 1;
}

int procesar_input_int(const char* input, int* valor) {
    if (stricmp(input, "SALIR") == 0) return 0;
    if (sscanf(input, "%d", valor) != 1) {
        printf("Entrada invalida.\n");
        return -1;
    }
    return 1;
}

// --------------------------
// MAIN CRUD FUNCTIONS
// --------------------------

void agregar_planeta(sqlite3* db) {
    Planeta p;
    char input[INPUT_BUFFER_SIZE];
    #ifdef _WIN32
    system("cls");
    #else
    system("clear");
    #endif
    printf("Cargando formulario de nuevo planeta...");
    #ifdef _WIN32
        Sleep(400); system("cls");
    #else
        usleep(400000); system("clear");
    #endif
    
    printf("\r");
    printf("\n====================================\n");
    printf("   AGREGAR NUEVO PLANETA\n");
    printf("====================================\n");

    do {
        printf("Nombre (o [SALIR] para cancelar): ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Error de entrada.\n");
            return;
        }
        input[strcspn(input, "\n")] = 0;
        if (stricmp(input, "SALIR") == 0) {
            printf("Operacion cancelada.\n");
            return;
        }
        if (strlen(input) == 0) {
            printf("El nombre no puede estar vacio. Intente nuevamente.\n");
            continue;
        }
        // Validar que el nombre no exista ya en la base de datos
        char sql_check[SQL_BUFFER_SIZE];
        snprintf(sql_check, sizeof(sql_check), "SELECT 1 FROM planetas WHERE nombre='%s';", input);
        sqlite3_stmt *stmt;
        int existe = 0;
        if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, NULL) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                existe = 1;
            }
            sqlite3_finalize(stmt);
        }
        if (existe) {
            printf("Ya existe un planeta con ese nombre. Intente otro nombre.\n");
            continue;
        }
        break;
    } while (1);
    strcpy(p.nombre, input);

    do {
        obtener_input("Radio (km) (2000 - 15000, o [SALIR] para cancelar): ", input, sizeof(input));
        if (stricmp(input, "SALIR") == 0) {
            printf("Operacion cancelada.\n");
            Sleep(800);
            return;
        }
        if (strlen(input) == 0) {
            printf("El campo no puede estar vacio. Intente nuevamente.\n");
            continue;
        }
        int es_numero = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if ((input[i] < '0' || input[i] > '9') && input[i] != '.') {
                es_numero = 0;
                break;
            }
        }
        if (!es_numero) {
            printf("Solo se permiten numeros. Intente nuevamente.\n");
            continue;
        }
        if (procesar_input_float(input, &p.radio_km) <= 0) {
            printf("Operacion cancelada.\n");
            return;
        }
        if (p.radio_km < 2000.0f || p.radio_km > 15000.0f) {
            printf("Radio fuera de rango realista. Intente nuevamente.\n");
        }
    } while (p.radio_km < 2000.0f || p.radio_km > 15000.0f);

    do {    //velocidad orbital
        obtener_input("Velocidad orbital (km/s) (o [SALIR] para cancelar): ", input, sizeof(input));
        if (stricmp(input, "SALIR") == 0) {
            printf("Operacion cancelada.\n");
            Sleep(800);
            return;
        }
        if (strlen(input) == 0) {
            printf("El campo no puede estar vacio. Intente nuevamente.\n");
            continue;
        }
        int es_numero = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if ((input[i] < '0' || input[i] > '9') && input[i] != '.') {
                es_numero = 0;
                break;
            }
        }
        if (!es_numero) {
            printf("Solo se permiten numeros. Intente nuevamente.\n");
            continue;
        }
        if (procesar_input_float(input, &p.velocidad_orbital) <= 0) {
            printf("Operacion cancelada.\n");
            return;
        }
        break;
    } while (1);

    float distancia_minima = DISTANCIA_MIN + (p.radio_km * 0.007f / 0.1f);

    do {    //  distancia al Sol
        printf("Distancia al Sol (millones km, %.1f - %.1f, o [SALIR] para cancelar): ", distancia_minima, DISTANCIA_MAX);      
        obtener_input("", input, sizeof(input));
        
        if (stricmp(input, "SALIR") == 0) {
            printf("Operacion cancelada.\n");
            Sleep(800);
            return;
        }

        if (strlen(input) == 0) {
            printf("El campo no puede estar vacio. Intente nuevamente.\n");
            continue;
        }
        int es_numero = 1;
        for (int i = 0; input[i] != '\0'; i++) {
            if ((input[i] < '0' || input[i] > '9') && input[i] != '.') {
                es_numero = 0;
                break;
            }
        }
        if (!es_numero) {
            printf("Solo se permiten numeros. Intente nuevamente.\n");
            continue;
        }
        int resultado = procesar_input_float(input, &p.distancia_sol);
        if (resultado <= 0) {
            printf("Operacion cancelada.\n");
            return;
        }
        if (p.distancia_sol < distancia_minima || p.distancia_sol > DISTANCIA_MAX) {
            printf("Distancia fuera de rango. Intente nuevamente.\n");
        }
    } while (p.distancia_sol < distancia_minima || p.distancia_sol > DISTANCIA_MAX);
    
    const char* nombres_colores[] = {"principal", "secundario", "terciario"};
        printf("Colores validos: RED, GREEN, BLUE, YELLOW, WHITE, BLACK, GRAY, BROWN, ORANGE, PURPLE\n");
    for (int i = 0; i < MAX_COLORES; i++) {
    do {
        printf("Ingrese el color %s (o [SALIR] para cancelar): ", nombres_colores[i]);
        obtener_input("", input, sizeof(input));
        if (stricmp(input, "SALIR") == 0) {
            printf("Operacion cancelada.\n");
            return;
        }
        if (!es_color_valido(input)) {
            printf("Color invalido. Intente nuevamente.\n");
        } else {
            strcpy(p.colores[i], input);
            break;
        }
    } while (1);
    }

    do {
        printf("Numero de lunas (0-%d, o [SALIR] para cancelar): ", MAX_LUNAS);
        obtener_input("", input, sizeof(input));
        if (stricmp(input, "SALIR") == 0) {
            printf("Operacion cancelada.\n");
            return;
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
        int resultado = procesar_input_int(input, &p.num_lunas);
        if (resultado <= 0) {
            printf("Operacion cancelada.\n");
            return;
        }
        if (p.num_lunas < 0 || p.num_lunas > MAX_LUNAS) {
            printf("Numero fuera de rango. Intente nuevamente.\n");
            continue;
        }
    } while (p.num_lunas < 0 || p.num_lunas > MAX_LUNAS);

    if (validar_orbita(db, &p, "")) {
        return;
    }

    char sql_insert[SQL_BUFFER_SIZE];
    snprintf(sql_insert, sizeof(sql_insert),
        "INSERT INTO planetas (nombre, radio_km, velocidad_orbital, distancia_sol, color1, color2, color3, num_lunas) "
        "VALUES ('%s', %f, %f, %f, '%s', '%s', '%s', %d);",
        p.nombre, p.radio_km, p.velocidad_orbital, p.distancia_sol,
        p.colores[0], p.colores[1], p.colores[2], p.num_lunas);

    if (sqlite3_exec(db, sql_insert, 0, 0, 0) == SQLITE_OK) {
        printf("\nProcesando...");
        #ifdef _WIN32
            Sleep(800);
        #else
            usleep(800000);
        #endif
        system("cls");
        printf("Planeta '%s' agregado con exito.\n", p.nombre);
        #ifdef _WIN32
            Sleep(1000);
        #else
            usleep(1000000);
        #endif    
    } else {
        printf("\nProcesando...");
        #ifdef _WIN32
            Sleep(600);
        #else
            usleep(600000);
        #endif
        printf("\rError al agregar planeta.\n");
        #ifdef _WIN32
            Sleep(600);
        #else
            usleep(600000);
        #endif
    }
}

void editar_planeta(sqlite3* db) {
    char nombre[32];
    char input[INPUT_BUFFER_SIZE];

    system("clear"); system("cls");
    printf("Cargando formulario de edicion...");
    #ifdef _WIN32
        Sleep(600); system("cls");
    #else
        usleep(600000); system("clear");
    #endif
     
    printf("\r");
    printf("\n====================================\n");
    printf("   EDITAR PLANETA\n");
    printf("====================================\n");
    // Mostrar lista de planetas disponibles
    sqlite3_stmt *stmt_list;
    const char *sql_list = "SELECT nombre FROM planetas;";
    if (sqlite3_prepare_v2(db, sql_list, -1, &stmt_list, NULL) == SQLITE_OK) {
        printf("Planetas disponibles:\n");
        while (sqlite3_step(stmt_list) == SQLITE_ROW) {
            printf(" - %s\n", sqlite3_column_text(stmt_list, 0));
        }
        sqlite3_finalize(stmt_list);
    }
    printf("\n");
    printf("Presione 'Enter' para continuar...");
    int c;
    while ((c = getchar()) != '\n' && c != EOF)// Limpia el buffer de entrada
    ; 
        // Borra la línea del mensaje en la terminal
        printf("\033[F");
        printf("\r%*s\r", 40, "");
    do {
        obtener_input("Nombre del planeta a editar (o [SALIR] para cancelar): ", nombre, sizeof(nombre));
        if (stricmp(nombre, "SALIR") == 0) {
            printf("Operacion cancelada.\n");
            return;
        }
        if (strlen(nombre) == 0) {
            printf("El nombre no puede estar vacio. Intente nuevamente.\n");
            continue;
        }
        // Validar que el planeta exista en la base de datos
        char sql_check[SQL_BUFFER_SIZE];
        snprintf(sql_check, sizeof(sql_check), "SELECT 1 FROM planetas WHERE nombre='%s';", nombre);
        sqlite3_stmt *stmt;
        int existe = 0;
        if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, NULL) == SQLITE_OK) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                existe = 1;
            }
            sqlite3_finalize(stmt);
        }
        if (!existe) {
            printf("No existe un planeta con ese nombre. Intente nuevamente.\n");
            continue;
        }
        break;
    } while (1);

    char sql_check[SQL_BUFFER_SIZE];
    snprintf(sql_check, sizeof(sql_check), 
        "SELECT 1 FROM planetas WHERE nombre='%s';", nombre);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql_check, -1, &stmt, NULL) != SQLITE_OK || sqlite3_step(stmt) != SQLITE_ROW) {
        printf("Planeta no encontrado.\n");
        sqlite3_finalize(stmt);
        return;
    }
    sqlite3_finalize(stmt);
    Planeta p;
    strcpy(p.nombre, nombre);
    obtener_datos_planeta(db, nombre, &p);

    int opcion = -1;
    do {
        mostrar_menu_edicion(&p);
        obtener_input("", input, sizeof(input));
        if (strlen(input) == 0) {
            printf("La entrada no puede estar vacia. Intente nuevamente.\n");
            #ifdef _WIN32
            Sleep(1500);
            #else
            usleep(1500000);
            #endif
            continue;
        }
        int resultado = procesar_input_int(input, &opcion);
        if (resultado == -1) {
            printf("Solo se permiten numeros enteros del 0 al 7. Intente nuevamente.\n");
            #ifdef _WIN32
            Sleep(1500);
            #else
            usleep(1500000);
            #endif
            continue;
        }
        if (opcion < 0 || opcion > 7) {
            printf("Solo se permiten opciones del 0 al 7. Intente nuevamente.\n");
            #ifdef _WIN32
            Sleep(1500);
            #else
            usleep(1500000);
            #endif
            continue;
        }
        if (opcion == 0 && resultado == 1) break;
        editar_campo(&p, opcion);
    } while (1);

    if (validar_orbita(db, &p, p.nombre)) {
        return;
    }

    char sql_update[SQL_BUFFER_SIZE];
    snprintf(sql_update, sizeof(sql_update),
        "UPDATE planetas SET radio_km=%f, velocidad_orbital=%f, distancia_sol=%f, "
        "color1='%s', color2='%s', color3='%s', num_lunas=%d WHERE nombre='%s';",
        p.radio_km, p.velocidad_orbital, p.distancia_sol,
        p.colores[0], p.colores[1], p.colores[2], p.num_lunas, p.nombre);

    if (sqlite3_exec(db, sql_update, 0, 0, 0) == SQLITE_OK) {
        printf("\nProcesando...");
        #ifdef _WIN32
            Sleep(500);
        #else
            usleep(500000);
        #endif
        system("clear"); system("cls");
        printf("\rPlaneta actualizado exitosamente.\n");
        #ifdef _WIN32
            Sleep(1000);
        #else
            usleep(1000000);
        #endif
    } else {
        printf("\nProcesando...");
        #ifdef _WIN32
            Sleep(5000);
        #else
            usleep(5000000);
        #endif
        printf("\rError al actualizar planeta.\n");
        
    }
}

void eliminar_planeta(sqlite3* db) {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
    printf("Cargando formulario de eliminacion...");
    #ifdef _WIN32
        Sleep(600); system("cls");
    #else
        usleep(600000); system("clear");
    #endif
     
    printf("\r");
    printf("\n====================================\n");
    printf("   ELIMINAR PLANETA\n");
    printf("====================================\n");
    
    sqlite3_stmt *stmt;
    const char *sql = "SELECT nombre FROM planetas;";
                     
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        printf("Planetas disponibles:\n");
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            printf(" - %s\n", sqlite3_column_text(stmt, 0));
        }
        sqlite3_finalize(stmt); 
    }  printf("presione ENTER para seleccionar un planeta");

    char nombre[32];
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
    ; 
    printf("\033[F");
    printf("\r%*s\r", 44, "");
    int existe = 0;
    do {
        obtener_input("Nombre del planeta a eliminar (o [SALIR] para cancelar): ", nombre, sizeof(nombre));
        if (stricmp(nombre, "SALIR") == 0) {
            printf("Operacion cancelada.\n");
            return;
        }
        if (strlen(nombre) == 0) {
            printf("El nombre no puede estar vacio. Intente nuevamente.\n");
            continue;
        }
        
        char sql_check[SQL_BUFFER_SIZE];
        snprintf(sql_check, sizeof(sql_check), "SELECT 1 FROM planetas WHERE nombre='%s';", nombre);
        sqlite3_stmt *stmt_check;
        existe = 0;
        if (sqlite3_prepare_v2(db, sql_check, -1, &stmt_check, NULL) == SQLITE_OK) {
            if (sqlite3_step(stmt_check) == SQLITE_ROW) {
                existe = 1;
            }
            sqlite3_finalize(stmt_check);
        }
        if (!existe) {
            printf("No existe un planeta con ese nombre. Intente nuevamente.\n");
            continue;
        }
        break;
    } while (1);

    char sql_del[SQL_BUFFER_SIZE];
    snprintf(sql_del, sizeof(sql_del), "DELETE FROM planetas WHERE nombre='%s';", nombre);

    if (sqlite3_exec(db, sql_del, 0, 0, 0) == SQLITE_OK) {
        printf("\nProcesando...");
        #ifdef _WIN32
            Sleep(600);
        #else
            usleep(600000);
        #endif
        printf("\rPlaneta eliminado exitosamente.\n");
        #ifdef _WIN32
            Sleep(1000);
        #else
            usleep(1000000);
        #endif
    } else {
        printf("\nProcesando...");
        #ifdef _WIN32
            Sleep(500);
        #else
            usleep(500000);
        #endif
        printf("\rError al eliminar planeta.\n");
        #ifdef _WIN32
            Sleep(800);
        #else
            usleep(800000);
        #endif
    }
}

void listar_planetas(sqlite3* db) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT nombre, radio_km, velocidad_orbital, distancia_sol, "
                     "color1, color2, color3, num_lunas FROM planetas;";
    
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
    printf("Cargando lista de planetas...");
    #ifdef _WIN32
        Sleep(400); system("cls");
    #else
        usleep(400000); system("clear");
    #endif
    printf("\r");
    printf("\n====================================\n");
    printf("        LISTA DE PLANETAS\n");
    printf("====================================\n");
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        int hay_planetas = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            hay_planetas = 1;
            printf("------------------------------------\n");
            printf("Nombre: %s\n", sqlite3_column_text(stmt, 0));
            printf("Radio: %.1f km\n", sqlite3_column_double(stmt, 1));
            printf("Velocidad orbital: %.2f km/s\n", sqlite3_column_double(stmt, 2));
            printf("Distancia al Sol: %.1f Mkm\n", sqlite3_column_double(stmt, 3));
            printf("Colores: %s, %s, %s\n", 
                  sqlite3_column_text(stmt, 4), 
                  sqlite3_column_text(stmt, 5), 
                  sqlite3_column_text(stmt, 6));
            printf("Lunas: %d\n", sqlite3_column_int(stmt, 7));
        }
        if (!hay_planetas) {
            printf("No hay planetas registrados.\n");
        }
        sqlite3_finalize(stmt);
    }
    printf("====================================\n");
    printf("\n");
    printf("Presione ENTER para continuar...");
    getchar();
}

// Compile in CMD with:
// gcc main.c dashboard.c planetas.c simulador.c source.c initialialization.c -o solar.exe -lsqlite3 -lraylib -lopengl32 -lgdi32 -lwinmm
// Run with:
// solar.exe

