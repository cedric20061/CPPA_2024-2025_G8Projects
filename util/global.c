#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "global.h"

int displayMenu(char *menu[], int count) {
    // Afficher les options du menu
    for (int i = 0; i < count; i++) {
        printf("%d. %s\n", i + 1, menu[i]);
    }

    int choice;
    while (1) {
        choice = getSafeInt("Votre choix: "); // Demande un choix à l'utilisateur
        if (choice >= 1 && choice <= count) { // Vérifie si le choix est dans l'intervalle valide
            break; // Si valide, sortir de la boucle
        } else {
            printf("Choix invalide. Veuillez entrer un nombre entre 1 et %d.\n", count);
        }
    }
    return choice;
}

int getSafeInt(char *prompt) {
    int value = 0;
    char buffer[100];

    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            printf("Erreur de lecture. Essayez à nouveau.\n");
            continue;
        }
        
        buffer[strcspn(buffer, "\n")] = '\0';

        if (sscanf(buffer, "%d", &value) == 1) {
            return value;
        } else {
            printf("Entree invalide. Veuillez entrer un entier valide.\n");
        }
    }
}
