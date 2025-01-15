#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "global.h"
#include <ctype.h>
#include "blocNote.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

FILE *open_file(const char *filename, const char *mode) {
    FILE *file = fopen(filename, mode);
    if (file == NULL) {
        printf("Erreur : Impossible d'ouvrir le fichier %s.\n", filename);
    }
    return file;
}

void display_file_content(FILE *file, char ***lines, size_t *num_lines) {
    char buffer[MAX_LINE_LENGTH];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
        (*num_lines)++;
        *lines = realloc(*lines, *num_lines * sizeof(char *));
        if (*lines == NULL) {
            printf("Erreur d'allocation memoire.\n");
            fclose(file);
            exit(1);
        }
        (*lines)[*num_lines - 1] = strdup(buffer);
    }

    printf("Contenu actuel du fichier :\n");
    for (size_t i = 0; i < *num_lines; i++) {
        printf("%zu > %s\n", i + 1, (*lines)[i]);
    }
}


void delete_file(char *filename) {
    if (remove(filename) == 0) {
        printf("Fichier %s supprime avec succes.\n", filename);
    } else {
        printf("Erreur lors de la suppression du fichier %s.\n", filename);
    }
}


int searchWord(char *filename, char *word) {
    FILE *file = open_file(filename, "r");
    char buffer[MAX_LINE_LENGTH];
    int count = 0;

    rewind(file);
    while (fgets(buffer, MAX_LINE_LENGTH, file)) {
        char *pos = buffer;
        while ((pos = strstr(pos, word)) != NULL) {
            count++;
            pos++;
        }
    }
    return count;
}

// Fonction pour creer recursivement un repertoire
int create_directory_recursively(const char *path) {
    char temp[PATH_MAX];
    char *p = NULL;

    // Copie de la chaîne pour eviter de modifier l'originale
    snprintf(temp, sizeof(temp), "%s", path);

    // Parcourir le chemin et creer chaque niveau
    for (p = temp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (mkdir(temp) != 0 && errno != EEXIST) {
                perror("Erreur lors de la creation du repertoire");
                return -1;
            }
            *p = '/';
        }
    }

    // Creer le dernier niveau
    if (mkdir(temp) != 0 && errno != EEXIST) {
        perror("Erreur lors de la creation du repertoire");
        return -1;
    }

    return 0;
}

void list_files_in_directory(const char *path) {
    struct dirent *entry;
    DIR *directory = opendir(path);

    if (directory == NULL) {
        if (errno == ENOENT) { // Verifier si le repertoire n'existe pas
            printf("Le repertoire '%s' n'existe pas. Creation en cours...\n", path);

            if (create_directory_recursively(path) == 0) {
                printf("Repertoire '%s' cree avec succys.\n", path);
            } else {
                printf("Erreur lors de la creation du repertoire '%s'.\n", path);
                return;
            }

            // Reessayer d'ouvrir le repertoire apres l'avoir cree
            directory = opendir(path);
            if (directory == NULL) {
                perror("Impossible d'ouvrir le repertoire même apres sa creation");
                return;
            }
        } else {
            perror("Impossible d'ouvrir le repertoire");
            return;
        }
    }

    printf("Contenu du repertoire %s :\n", path);
    while ((entry = readdir(directory)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            printf("- %s\n", entry->d_name);
        }
    }
    closedir(directory);
}