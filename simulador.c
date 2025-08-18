// Datos del archivo
// Autor: [Rodolfo Beltre]
// Matricula: [2025-0444]
// Fecha: [16/8/2025]
// Descripción: [CRUD simulador de sistemas solares]
// simulador.c

#include "simulador.h"
#include <raylib.h>
#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include "planetas.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include "initialization.h"

#define MAX_PLANETAS 16
#define NUM_ESTRELLAS 750
#define MAX_LUNAS 8

bool pausado = false;
bool mostrar_orbitas = true;

// --------------------------
// UTILITY FUNCTIONS
// --------------------------

int solo_letras(const char* s) {
    for (int i = 0; s[i]; i++) {
        if (s[i] < 'A' || s[i] > 'Z') return 0;
    }
    return 1;
}

Color nombre_a_color(const char *nombre) {
    if (strcmp(nombre, "RED") == 0) return RED;
    if (strcmp(nombre, "BLUE") == 0) return BLUE;
    if (strcmp(nombre, "GREEN") == 0) return GREEN;
    if (strcmp(nombre, "YELLOW") == 0) return YELLOW;
    if (strcmp(nombre, "ORANGE") == 0) return ORANGE;
    if (strcmp(nombre, "PURPLE") == 0) return PURPLE;
    if (strcmp(nombre, "BROWN") == 0) return BROWN;
    if (strcmp(nombre, "GRAY") == 0) return GRAY;
    if (strcmp(nombre, "WHITE") == 0) return WHITE;
    if (strcmp(nombre, "BLACK") == 0) return BLACK;
    return RAYWHITE;
}

void pedir_color(char* color) {
    char input[32];
    const char* colores[] = {"RED", "BLUE", "GREEN", "YELLOW", "ORANGE", 
                            "PURPLE", "BROWN", "GRAY", "WHITE", "BLACK"};
    int n = 10;
    int valido;
    do {
        printf("Color (RED, BLUE, GREEN, YELLOW, ORANGE, PURPLE, BROWN, GRAY, WHITE, BLACK): ");
        scanf("%s", input);
        valido = 0;
        for (int i = 0; i < n; i++) {
            if (strcmp(input, colores[i]) == 0) {
                valido = 1;
                break;
            }
        }
        if (!valido) printf("Color invalido. Intenta de nuevo.\n");
    } while (!valido);
    strcpy(color, input);
}

int es_color_valido(const char* color) {
    const char* colores[] = {"RED", "BLUE", "GREEN", "YELLOW", "ORANGE", 
                            "PURPLE", "BROWN", "GRAY", "WHITE", "BLACK"};
    int n = 10;
    for (int i = 0; i < n; i++) {
        if (strcmp(color, colores[i]) == 0) return 1;
    }
    return 0;
}

static inline Vector3 Vector3Subtract(Vector3 v1, Vector3 v2) {
    return (Vector3){v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
}

static inline Vector3 Vector3Scale(Vector3 v, float scale) {
    return (Vector3){v.x * scale, v.y * scale, v.z * scale};
}

static inline Vector3 Vector3Add(Vector3 v1, Vector3 v2) {
    return (Vector3){v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
}

Vector3 rotar_horizontal(Vector3 pos, float ang) {
    float x = pos.x * cosf(ang) - pos.z * sinf(ang);
    float z = pos.x * sinf(ang) + pos.z * cosf(ang);
    return (Vector3){x, pos.y, z};
}

// --------------------------
// DRAWING FUNCTIONS
// --------------------------

void dibujar_orbita(Vector3 centro, float radio, Color color);

// --------------------------
// INITIALIZATION FUNCTIONS
// --------------------------

void cargar_planetas(sqlite3* db, Planeta planetas[], int *n);    

void generar_manchas(ManchasPlaneta manchas[], Planeta planetas[], int n);

void inicializar_estrellas(float estrellas_x[], int estrellas_y[], int ancho, int alto);

// --------------------------
// MAIN SIMULATION FUNCTION
// --------------------------

float calcular_distancia_luna(float radio, int l, int num_lunas) {
    float divisor = (num_lunas > 1) ? (float)(num_lunas - 1) : 1.0f;
    return radio * (2.5f + l * (1.5f / divisor));
}

void iniciar_simulacion(sqlite3* db) {
    Planeta planetas[MAX_PLANETAS];
    ManchasPlaneta manchas[MAX_PLANETAS];
    int n = 0;
    
    InitAudioDevice();
    Music musica = LoadMusicStream("soundspace.wav");
    if (musica.ctxData == NULL) {
        printf("No se pudo cargar el archivo de audio.\n");
    }
    PlayMusicStream(musica);
    SetMusicVolume(musica, 0.5f);
    
    cargar_planetas(db, planetas, &n);
    generar_manchas(manchas, planetas, n);
    
    InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "Sistema Solar");
    ToggleFullscreen();
    SetExitKey(0);
    
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 50.0f, 120.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    
    float tiempo = 0.0f;
    float cam_angle_x = 0.0f;
    float cam_angle_y = 10.0f * PI / 180.0f;
    float cam_distance = 500.0f;
    bool dragging = false;
    int last_mouse_x = 0, last_mouse_y = 0;
    int planeta_seleccionado = -1;
    
    int ancho = GetScreenWidth();
    int alto = GetScreenHeight();
    float estrellas_x[NUM_ESTRELLAS];
    int estrellas_y[NUM_ESTRELLAS];
    inicializar_estrellas(estrellas_x, estrellas_y, ancho, alto);
    
    float lunas_phi[MAX_PLANETAS][MAX_LUNAS];
    float lunas_velocidad[MAX_PLANETAS][MAX_LUNAS];
    for (int i = 0; i < n; i++) {
        for (int l = 0; l < planetas[i].num_lunas; l++) {
            lunas_phi[i][l] = ((float)rand() / RAND_MAX) * PI;
            lunas_velocidad[i][l] = 0.2f + ((float)rand() / RAND_MAX) * 0.8f;
        }
    }
    
    SetTargetFPS(60);
    
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_O)) {
            mostrar_orbitas = !mostrar_orbitas;
        }

        if (IsKeyPressed(KEY_Q)) break;
        if (IsKeyPressed(KEY_SPACE)) pausado = !pausado;
        
        UpdateMusicStream(musica);
        
        if (!pausado) {
            tiempo += GetFrameTime();
        }
        
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            dragging = true;
            last_mouse_x = GetMouseX();
            last_mouse_y = GetMouseY();
        }
        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            dragging = false;
        }
        
        if (dragging) {
            int dx = GetMouseX() - last_mouse_x;
            int dy = GetMouseY() - last_mouse_y;
            cam_angle_x -= dx * 0.004f;
            cam_angle_y += dy * 0.004f;
            last_mouse_x = GetMouseX();
            last_mouse_y = GetMouseY();
        }
        
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            cam_distance -= wheel * 5.0f;
            if (cam_distance < 50.0f) cam_distance = 50.0f;
            if (cam_distance > 6000.0f) cam_distance = 6000.0f;
        }
        
        Vector3 posiciones[MAX_PLANETAS];
        float radios[MAX_PLANETAS];
        
        for (int i = 0; i < n; i++) {
            float ang = tiempo * planetas[i].velocidad_orbital * 0.01f;
            posiciones[i] = (Vector3){
                cosf(ang) * planetas[i].distancia_sol * 0.1f,
                0,
                sinf(ang) * planetas[i].distancia_sol * 0.1f
            };
            radios[i] = 2.0f + planetas[i].radio_km * 0.001f;
        }
        
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mouse = GetMousePosition();
            for (int i = 0; i < n; i++) {
                Vector2 pos2d = GetWorldToScreen(posiciones[i], camera);
                float radio_pantalla = radios[i] * 4.0f;
                if (CheckCollisionPointCircle(mouse, pos2d, radio_pantalla)) {
                    planeta_seleccionado = i;
                    cam_distance = 160.0f;
                    break;
                }
            }
        }
        
        if (IsKeyPressed(KEY_ESCAPE)) {
            planeta_seleccionado = -1;
            cam_angle_x = 0.0f;
            cam_angle_y = 30.0f * PI / 180.0f;
            cam_distance = 500.0f;
            camera.target = (Vector3){0, 0, 0};
        }
        
        Vector3 centro = (planeta_seleccionado >= 0) ? 
                         posiciones[planeta_seleccionado] : 
                         (Vector3){0, 0, 0};
        camera.target = centro;
        
        camera.position.x = centro.x + sinf(cam_angle_x) * cosf(cam_angle_y) * cam_distance;
        camera.position.y = centro.y + sinf(cam_angle_y) * cam_distance;
        camera.position.z = centro.z + cosf(cam_angle_x) * cosf(cam_angle_y) * cam_distance;
        
        BeginDrawing();
        ClearBackground((Color){10, 10, 30, 255});
        
        if (pausado) {
            DrawText("Simulacion pausada (SPACE para continuar)", 10, 90, 20, RED);
        }
        
        for (int s = 0; s < NUM_ESTRELLAS; s++) {
            estrellas_x[s] += 0.05f;
            if (estrellas_x[s] > ancho) {
                estrellas_x[s] = 0;
                estrellas_y[s] = rand() % alto;
            }
            DrawPixel((int)estrellas_x[s], estrellas_y[s], RAYWHITE);
        }
        
        BeginMode3D(camera);
        
        DrawSphere((Vector3){0, 0, 0}, 40.0f, YELLOW);
        Color brillo = (Color){255, 255, 100, 65};
        DrawSphereEx((Vector3){0, 0, 0}, 44.0f, 32, 32, brillo);
        
        for (int i = 0; i < n; i++) {
            float ang = tiempo * planetas[i].velocidad_orbital * 0.01f;
            float x = cosf(ang) * planetas[i].distancia_sol * 0.1f;
            float z = sinf(ang) * planetas[i].distancia_sol * 0.1f;
            float radio = 2.0f + planetas[i].radio_km * 0.001f;
            float rotacion_eje = tiempo * planetas[i].velocidad_orbital * 0.05f;
            
            if (mostrar_orbitas && planetas[i].distancia_sol > 0.0f) {
            dibujar_orbita((Vector3){0, 0, 0}, planetas[i].distancia_sol * 0.1f, GRAY);
            }
            
            Color color_principal = nombre_a_color(planetas[i].colores[0]);
            DrawSphereEx((Vector3){x, 0, z}, radio, 16, 16, color_principal);
            
            Color color_secundario = nombre_a_color(planetas[i].colores[1]);
            for (int m = 0; m < 124; m++) {
                Vector3 rel = rotar_horizontal(manchas[i].secundarias[m], rotacion_eje);
                Vector3 pos = { x + rel.x, rel.y, z + rel.z };
                DrawSphere(pos, radio * 0.08f, color_secundario);
            }
            
            Color color_terciario = nombre_a_color(planetas[i].colores[2]);
            for (int m = 0; m < 32; m++) {
                Vector3 rel = rotar_horizontal(manchas[i].terciarias[m], rotacion_eje);
                Vector3 pos = { x + rel.x, rel.y, z + rel.z };
                DrawSphere(pos, radio * 0.06f, color_terciario);
            }
            
            for (int l = 0; l < planetas[i].num_lunas; l++) {
                float velocidad_luna = lunas_velocidad[i][l];
                float theta = tiempo * velocidad_luna + l * (2 * PI / planetas[i].num_lunas);
                float phi = lunas_phi[i][l];
                float distancia_luna = calcular_distancia_luna(radio, l, planetas[i].num_lunas);
                
                Vector3 luna_pos = {
                    x + cosf(theta) * distancia_luna,
                    sinf(phi) * radio * 1.f,
                    z + sinf(theta) * distancia_luna
                };
                DrawSphere(luna_pos, radio * 0.1f, GRAY);
            }
        }
        
        EndMode3D();
        
        if (planeta_seleccionado >= 0) {
            int panel_ancho = 300;
            int panel_alto = 150;
            int panel_x = ancho - panel_ancho - 20;
            int panel_y = 100;
            
            DrawRectangle(panel_x - 2, panel_y - 2, panel_ancho + 4, panel_alto + 4, WHITE);
            DrawRectangle(panel_x, panel_y, panel_ancho, panel_alto, Fade(BLACK, 0.7f));
            
            DrawText("[Presione ESC para salir de la visualización]", 
                   ancho - MeasureText("[Presione ESC para salir de la visualización]", 18) - 20, 
                   panel_y + panel_alto + 20, 18, RED);
            
            DrawText(TextFormat("Nombre: %s", planetas[planeta_seleccionado].nombre), 
                   panel_x + 10, panel_y + 10, 18, RAYWHITE);
            DrawText(TextFormat("Radio: %.0f km", planetas[planeta_seleccionado].radio_km), 
                   panel_x + 10, panel_y + 30, 18, RAYWHITE);
            DrawText(TextFormat("Distancia al Sol: %.0f Millones km", planetas[planeta_seleccionado].distancia_sol), 
                   panel_x + 10, panel_y + 50, 18, RAYWHITE);
            DrawText(TextFormat("Lunas: %d", planetas[planeta_seleccionado].num_lunas), 
                   panel_x + 10, panel_y + 70, 18, RAYWHITE);
            DrawText(TextFormat("Velocidad orbital: %.2f km/s", planetas[planeta_seleccionado].velocidad_orbital), 
                   panel_x + 10, panel_y + 90, 18, RAYWHITE);
        }
        
        DrawText("Controles: Space-bar para pausar, Mouse para rotar, rueda para zoom", 10, 10, 20, RAYWHITE);
        DrawText("Presione O para mostrar/ocultar orbitas", 10, 30, 20, RAYWHITE);
        DrawText("Presione Q para salir de la simulacion", 10, 50, 20, RAYWHITE);
        
        EndDrawing();
    }
    
    UnloadMusicStream(musica);
    CloseAudioDevice();
    CloseWindow();
}

// Compile with:
// gcc main.c dashboard.c planetas.c simulador.c source.c -o solar.exe -lsqlite3 -lraylib -lopengl32 -lgdi32 -lwinmm
// Run with:
// solar.exe