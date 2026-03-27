#include "arrakis2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* =====================================================================
 * FUNCIONES PROPORCIONADAS
 * =====================================================================*/

Casa *crear_casa(const char *nombre, int soldados)
{
    Casa *nueva = (Casa *)malloc(sizeof(Casa));
    if (!nueva) {
        fprintf(stderr, "Error: no se pudo asignar memoria.\n");
        exit(EXIT_FAILURE);
    }
    strncpy(nueva->nombre, nombre, MAX_NOMBRE - 1);
    nueva->nombre[MAX_NOMBRE - 1] = '\0';
    nueva->soldados = soldados;
    nueva->especia = 0;
    nueva->siguiente = NULL;
    return nueva;
}

int lanzar_dado(void)
{
    return 1 + rand() % 6;
}

void mostrar_anillo(Casa *cualquiera)
{
    if (!cualquiera) {
        printf("\n  (anillo vacio)\n");
        return;
    }

    Casa *p = cualquiera;
    int i = 1;
    printf("\n  %-4s %-25s %-12s %-10s\n", "#", "Casa", "Soldados", "Especia");
    printf("  -------------------------------------------------------\n");

    do {
        printf("  %-4d %-25s %-12d %-10d\n", i++, p->nombre, p->soldados, p->especia);
        p = p->siguiente;
    } while (p != cualquiera);

    printf("  -------------------------------------------------------\n");
}

void liberar_anillo(Casa *cualquiera)
{
    if (!cualquiera) return;

    Casa *p = cualquiera->siguiente;
    while (p != cualquiera) {
        Casa *tmp = p;
        p = p->siguiente;
        free(tmp);
    }
    free(cualquiera);
}

int contar_casas(Casa *cualquiera)
{
    if (!cualquiera) return 0;

    int n = 0;
    Casa *p = cualquiera;
    do {
        n++;
        p = p->siguiente;
    } while (p != cualquiera);

    return n;
}

/* =====================================================================
 * FUNCIONES IMPLEMENTADAS
 * =====================================================================*/

void insertar_casa(Casa **anillo, Casa *nueva)
{
    if (!*anillo) {
        nueva->siguiente = nueva;
        *anillo = nueva;
        return;
    }

    Casa *ultimo = *anillo;
    while (ultimo->siguiente != *anillo) {
        ultimo = ultimo->siguiente;
    }

    ultimo->siguiente = nueva;
    nueva->siguiente = *anillo;
}

Casa *avanzar_gusano(Casa *actual, int pasos)
{
    for (int i = 0; i < pasos; i++) {
        actual = actual->siguiente;
    }
    return actual;
}

int atacar_asentamiento(Casa **gusano, Casa **anillo)
{
    if (!*gusano) return 0;

    Casa *actual = *gusano;
    actual->soldados -= actual->soldados * DANIO_PORCENTAJE / 100;

    /* Cosecha de especia para los demás */
    Casa *p = *anillo;
    do {
        if (p != actual) p->especia += ESPECIA_COSECHA;
        p = p->siguiente;
    } while (p != *anillo);

    /* Eliminación si soldados < mínimo */
    if (actual->soldados < SOLDADOS_MIN) {
        if (actual->siguiente == actual) {
            free(actual);
            *anillo = NULL;
            *gusano = NULL;
            return 1;
        }

        /* Buscar anterior */
        Casa *anterior = actual;
        while (anterior->siguiente != actual) anterior = anterior->siguiente;

        anterior->siguiente = actual->siguiente;
        if (*anillo == actual) *anillo = actual->siguiente;

        *gusano = actual->siguiente;
        free(actual);
        return 1;
    }

    return 0;
}

int invocar_refuerzos(Casa *actual, Casa **anillo)
{
    if (actual->especia < ESPECIA_REFUERZOS) return 0;

    char nombre_nuevo[MAX_NOMBRE];
    snprintf(nombre_nuevo, MAX_NOMBRE, "%s II", actual->nombre);
    Casa *nuevo = crear_casa(nombre_nuevo, 250);

    /* Buscar anterior */
    Casa *anterior = actual;
    while (anterior->siguiente != actual) anterior = anterior->siguiente;

    anterior->siguiente = nuevo;
    nuevo->siguiente = actual;

    actual->especia = 0;

    return 1;
}

/* =====================================================================
 * MAIN
 * =====================================================================*/

int main(void)
{
    Casa *anillo = NULL;
    Casa *gusano = NULL;
    int turno = 0;
    int max_turnos = 30;

    srand((unsigned)time(NULL));

    printf("==========================================================\n");
    printf("   ARRAKIS KERNEL - El Ciclo de Shai-Hulud\n");
    printf("==========================================================\n");

    insertar_casa(&anillo, crear_casa("Atreides", SOLDADOS_INICIAL));
    insertar_casa(&anillo, crear_casa("Harkonnen", SOLDADOS_INICIAL));
    insertar_casa(&anillo, crear_casa("Fremen", SOLDADOS_INICIAL));
    insertar_casa(&anillo, crear_casa("Corrino", SOLDADOS_INICIAL));
    insertar_casa(&anillo, crear_casa("Ordos", SOLDADOS_INICIAL));
    insertar_casa(&anillo, crear_casa("Tleilaxu", SOLDADOS_INICIAL));

    printf("\n>>> Estado inicial del anillo:");
    mostrar_anillo(anillo);

    gusano = anillo;

    while (contar_casas(anillo) > 1 && turno < max_turnos) {
        turno++;
        printf("\n--- TURNO %d ---\n", turno);

        int dado = lanzar_dado();
        gusano = avanzar_gusano(gusano, dado);
        printf("  Dado: %d -> Shai-Hulud se detiene en: %s\n", dado, gusano->nombre);

        if (atacar_asentamiento(&gusano, &anillo)) {
            printf("  >> Una casa ha sido consumida por el desierto!\n");
        }

        if (contar_casas(anillo) <= 1) break;

        Casa *p = anillo;
        do {
            if (invocar_refuerzos(p, &anillo)) {
                printf("  >> %s invoca Refuerzos de la Cofradia Espacial!\n", p->nombre);
            }
            p = p->siguiente;
        } while (p != anillo);

        printf("\n  Estado del anillo (%d casas):", contar_casas(anillo));
        mostrar_anillo(anillo);
    }

    printf("\n==========================================================\n");
    if (contar_casas(anillo) == 1)
        printf("  VICTORIA: %s domina Arrakis!\n", anillo->nombre);
    else
        printf("  Fin de la simulacion tras %d turnos. Casas sobrevivientes: %d\n", max_turnos, contar_casas(anillo));
    printf("==========================================================\n");

    liberar_anillo(anillo);

    return 0;
}
