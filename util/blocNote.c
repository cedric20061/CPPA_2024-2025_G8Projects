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

#ifdef _WIN32
#include <direct.h>
#define mkdir(path) _mkdir(path)
#endif

int create_directory_recursively(const char *path) {
    char temp[PATH_MAX];
    char *p = NULL;

    snprintf(temp, sizeof(temp), "%s", path);

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

    if (mkdir(temp) != 0 && errno != EEXIST) {
        perror("Erreur lors de la creation du repertoire");
        return -1;
    }

    return 0;
}

FILE *open_file(const char *filename, const char *mode) {
    // Extraire le chemin du répertoire
    FILE *file = fopen(filename, mode);
    if (file == NULL){
        fclose(file);
        char directory[1024];
        snprintf(directory, sizeof(directory), "%s", filename);
        char *last_slash = strrchr(directory, '/');

        if (last_slash != NULL) {
            *last_slash = '\0';

            // Créer les répertoires si nécessaire
            if (create_directory_recursively(directory) != 0) {
                printf("Erreur : Impossible de créer les répertoires pour %s.\n", directory);
                return NULL;
            }
        }
            // Ouvrir le fichier
        FILE *file = fopen(filename, mode);
        return file;
    }
    


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

void list_files_in_directory(const char *path) {
    struct dirent *entry;
    DIR *directory = opendir(path);

    if (directory == NULL) {
        if (errno == ENOENT) { // Verifier si le repertoire n'existe pas

            if (create_directory_recursively(path) != 0) {
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