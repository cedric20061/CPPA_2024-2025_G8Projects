#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "game.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

Levels gamesLevels[] = {
    {"Facile (Nombre entre 0 et 50, essais illimites)", 50, 0},
    {"Moyen (Nombre entre 0 et 100, 10 essais maximum)", 100, 10},
    {"Difficile (Nombre entre 0 et 500, 10 essais maximum)", 500, 10},
    {"Expert (Nombre entre 0 et 1000, 15 essais maximum)", 1000, 15},
}; 

void cleanString(char *str) {
    char *ptr = str;
    // Supprimer les espaces au debut
    while (*ptr == ' ' || *ptr == '\n' || *ptr == '\r') ptr++;
    if (ptr != str) memmove(str, ptr, strlen(ptr) + 1);
    
    // Supprimer les espaces à la fin
    ptr = str + strlen(str) - 1;
    while (ptr >= str && (*ptr == ' ' || *ptr == '\n' || *ptr == '\r')) {
        *ptr = '\0';
        ptr--;
    }
}

int generate_random_number(int min, int max) {
    return min + rand() % (max - min + 1);
}

User *loadUsers(int *userCount) {
    FILE *file = fopen(SCORE_FILE, "r");

    if (!file) {
        // Tenter de créer le dossier si le fichier ne s'ouvre pas
        if (errno == ENOENT) { // Vérifie si le chemin n'existe pas
            if (mkdir("../../data") == -1 && errno != EEXIST) {
                printf("Erreur : impossible de créer le dossier data.\n");
                *userCount = 0;
                return NULL;
            }
        }

        // Créer le fichier après avoir tenté de créer le dossier
        file = fopen(SCORE_FILE, "w");
        if (!file) {
            printf("Erreur : impossible de créer le fichier game.txt.\n");
            *userCount = 0;
            return NULL;
        }

        fclose(file);
        *userCount = 0;
        return NULL;
    }

    char buffer[BUFFER_SIZE];
    User *users = NULL;
    int count = 0;

    while (fgets(buffer, BUFFER_SIZE, file)) {
        cleanString(buffer);
        if (strlen(buffer) > 0) {
            users = realloc(users, (count + 1) * sizeof(User));
            if (!users) {
                fclose(file);
                *userCount = 0;
                return NULL;
            }

            strncpy(users[count].name, buffer, MAX_NAME_LENGTH - 1);
            users[count].name[MAX_NAME_LENGTH - 1] = '\0';

            for (int i = 0; i < 4; i++) {
                if (fgets(buffer, BUFFER_SIZE, file)) {
                    sscanf(buffer, "%d %d", &users[count].bestScores[i], &users[count].worstScores[i]);
                } else {
                    users[count].bestScores[i] = 0;
                    users[count].worstScores[i] = 0;
                }
            }

            count++;
        }
    }

    fclose(file);
    *userCount = count;
    return users;
}

void saveUsers(User users[], int count) {
    FILE *file = fopen(SCORE_FILE, "w");
    if (!file) {
        printf("Erreur lors de l'enregistrement des utilisateurs.\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        fprintf(file, "%s\n", users[i].name);

        for (int j = 0; j < 4; j++) {
            fprintf(file, "%d %d\n", users[i].bestScores[j], users[i].worstScores[j]);
        }
    }

    fclose(file);
}


int addUser(User **users, int *userCount, char *name) {
    *users = loadUsers(userCount);
    for (int i = 0; i < *userCount; i++) {
        if (strcmp((*users)[i].name, name) == 0) {
            printf("Utilisateur '%s' existe dejà.\n", name);
            return 0;
        }
    }

    User newUser;
    strcpy(newUser.name, name);

    // Initialiser les tableaux à zero
    memset(newUser.bestScores, 0, sizeof(newUser.bestScores));
    memset(newUser.worstScores, 0, sizeof(newUser.worstScores));

    // Reallouer la memoire pour ajouter un utilisateur
    *users = realloc(*users, (*userCount + 1) * sizeof(User));
    if (*users == NULL) {
        perror("Erreur de memoire");
        return 0;
    }

    (*users)[*userCount] = newUser;
    (*userCount)++;
    saveUsers(*users, *userCount);
    return 1;
}

void loadScores(User *user) {
    for (int i = 0; i < 4; i++) {
        user->bestScores[i] = 0;
        user->worstScores[i] = 0;
    }

    FILE *file = fopen(SCORE_FILE, "r");
    if (!file) return;

    char buffer[BUFFER_SIZE];
    char name[MAX_NAME_LENGTH];

    while (fgets(buffer, BUFFER_SIZE, file)) {
        cleanString(buffer);
        if (sscanf(buffer, "%s", name) == 1) {
            if (strcmp(name, user->name) == 0) {
                for (int i = 0; i < 4; i++) {
                    if (fgets(buffer, BUFFER_SIZE, file)) {
                        sscanf(buffer, "%d %d", &user->bestScores[i], &user->worstScores[i]);
                    }
                }
                break;
            } else {
                for (int i = 0; i < 4; i++) {
                    fgets(buffer, BUFFER_SIZE, file);
                }
            }
        }
    }

    fclose(file);
}

void saveScores(User *user) {
    FILE *file = fopen(SCORE_FILE, "r");
    FILE *temp = fopen(TEMP_FILE, "w");
    
    if (!temp) {
        if (file) fclose(file);
        printf("Erreur lors de la creation du fichier temporaire.\n");
        return;
    }

    int found = 0;
    char buffer[BUFFER_SIZE];
    char name[MAX_NAME_LENGTH];

    if (file) {
        while (fgets(buffer, BUFFER_SIZE, file)) {
            cleanString(buffer);
            if (sscanf(buffer, "%s", name) == 1) {
                if (strcmp(name, user->name) == 0) {
                    found = 1;
                    fprintf(temp, "%s\n", user->name);
                    for (int i = 0; i < 4; i++) {
                        fprintf(temp, "%d %d\n", user->bestScores[i], user->worstScores[i]);
                    }
                    // Sauter les 4 lignes de scores dans le fichier source
                    for (int i = 0; i < 4; i++) {
                        fgets(buffer, BUFFER_SIZE, file);
                    }
                } else {
                    // Copier le nom et les 4 lignes de scores
                    fprintf(temp, "%s\n", name);
                    for (int i = 0; i < 4; i++) {
                        if (fgets(buffer, BUFFER_SIZE, file)) {
                            fprintf(temp, "%s", buffer);
                        }
                    }
                }
            }
        }
        fclose(file);
    }

    if (!found) {
        fprintf(temp, "%s\n", user->name);
        for (int i = 0; i < 4; i++) {
            fprintf(temp, "%d %d\n", user->bestScores[i], user->worstScores[i]);
        }
    }

    fclose(temp);
    remove(SCORE_FILE);
    rename(TEMP_FILE, SCORE_FILE);
}

void updateScores(User *user, int level, int score) {
    int idx = level - 1;
    if (user->bestScores[idx] == 0 || score < user->bestScores[idx]) {
        user->bestScores[idx] = score;
    }
    if (score > user->worstScores[idx]) {
        user->worstScores[idx] = score;
    }
}
