#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "global.h"
#include <ctype.h>
#include "blocNote.h"

// Affiche le fontionnement de l'éditeur
void display_editor_instructions() {
    printf("\nFonctionnement de l'editeur :\n");
    printf("1. Ouvrez un fichier existant ou creez un nouveau fichier pour commencer.\n");
    printf("2. Une fois le fichier ouvert, vous pouvez :\n");
    printf("   - Ajouter des lignes en tapant du texte à la fin.\n");
    printf("   - Modifier une ligne existante en tapant '\\m' suivi du numero de la ligne.\n");
    printf("   - Inserer une ligne après une ligne specifique en tapant '\\i' suivi du numero de la ligne.\n");
    printf("   - Terminer l'edition en appuyant sur Entree sur une ligne vide.\n");
    printf("3. Enregistrez vos modifications et quittez pour mettre à jour le fichier.\n");
    printf("NB: Le nombre maximal de caractère par ligne est de 256. Faites attention a ne pas exeder cette limite.");
}

// Fonction pour modifier une ligne existante
void edit_line(char **lines, size_t num_lines) {
    printf("Entrez le numero de la ligne à modifier : ");
    int line_to_edit;
    if (scanf("%d", &line_to_edit) != 1 || line_to_edit < 1 || (size_t)line_to_edit > num_lines) {
        printf("Numero de ligne invalide.\n");
    } else {
        getchar();
        printf("Ligne actuelle : %s\n", lines[line_to_edit - 1]);
        printf("Entrez le nouveau texte : ");
        char buffer[MAX_LINE_LENGTH];
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            buffer[strcspn(buffer, "\n")] = '\0';
            free(lines[line_to_edit - 1]);
            lines[line_to_edit - 1] = strdup(buffer);
        }
    }
}

// Fonction pour inserer une nouvelle ligne
void insert_line(char ***lines, size_t *num_lines) {
    printf("Entrez le numero de la ligne après laquelle inserer : ");
    int line_to_insert_after;
    if (scanf("%d", &line_to_insert_after) != 1 || line_to_insert_after < 0 || (size_t)line_to_insert_after > *num_lines) {
        printf("Numero de ligne invalide.\n");
    } else {
        getchar();
        printf("Entrez le texte à inserer : ");
        char buffer[MAX_LINE_LENGTH];
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            buffer[strcspn(buffer, "\n")] = '\0';
            (*num_lines)++;
            *lines = realloc(*lines, (*num_lines) * sizeof(char *));
            if (*lines == NULL) {
                printf("Erreur d'allocation memoire.\n");
                exit(1);
            }
            for (size_t i = *num_lines - 1; i > (size_t)line_to_insert_after; i--) {
                (*lines)[i] = (*lines)[i - 1];
            }
            (*lines)[line_to_insert_after] = strdup(buffer);
        }
    }
}

// Fonction pour ajouter une nouvelle ligne à la fin
void add_new_line(char ***lines, size_t *num_lines, const char *buffer) {
    (*num_lines)++;
    *lines = realloc(*lines, (*num_lines) * sizeof(char *));
    if (*lines == NULL) {
        printf("Erreur d'allocation memoire.\n");
        exit(1);
    }
    (*lines)[*num_lines - 1] = strdup(buffer);
}

// Fonction pour sauvegarder les lignes dans le fichier
void save_lines_to_file(const char *filename, FILE *file, char **lines, size_t num_lines) {
    freopen(filename, "w", file); // Reouvrir pour ecriture
    rewind(file); // Revenir au debut du fichier
    for (size_t i = 0; i < num_lines; i++) {
        fprintf(file, "%s\n", lines[i]);
        free(lines[i]);
    }
    free(lines);
}

void dynamic_text_editing(const char *filename) {
    FILE *file = open_file(filename, "r+");
    if (file == NULL) {
        return;
    }

    char **lines = NULL;
    size_t num_lines = 0;
    display_file_content(file, &lines, &num_lines);

    printf("\nEntrez du texte pour completer, modifier ou inserer une ligne (ligne vide pour terminer) :\n");

    char buffer[MAX_LINE_LENGTH];
    while (1) {
        printf("%zu > ", num_lines + 1);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;
        }

        buffer[strcspn(buffer, "\n")] = '\0';

        if (strlen(buffer) == 0) {
            break;
        }

        if (buffer[0] == '\\') {
            if (buffer[1] == 'm') {
                edit_line(lines, num_lines);
            } else if (buffer[1] == 'i') {
                insert_line(&lines, &num_lines);
            }
        } else {
            add_new_line(&lines, &num_lines, buffer);
        }
    }

    save_lines_to_file(filename, file, lines, num_lines);
    fclose(file);
    printf("Fichier mis à jour avec succès.\n");
}


int main() {
    const char *path = "../../data/blocNote";

    while (1) {
        char filename[MAX_LINE_LENGTH] = "../../data/blocNote/";
        printf("\nMenu principal:\n");
        char *mainMenuOptions[] = {
            "Ouvrir un fichier existant",
            "Creer un nouveau fichier",
            "Supprimer un fichier",
            "Afficher le fonctionnement de l'editeur",
            "Quitter"
        };

        int choice = displayMenu(mainMenuOptions, 5);
        char buffer[MAX_LINE_LENGTH];
        switch (choice) {
            case 1:
                list_files_in_directory(path);
                printf("Entrez le nom du fichier à ouvrir : ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = '\0';
                strcat(filename, buffer);
                strcat(filename, ".txt");
                char *menu[] = {"Nombre d'occurrences d'un mot", "Modification du fichier"};
                int choice2 = displayMenu(menu, 2);
                switch (choice2) {
                    case 1:
                        printf("Entrez le mot à rechercher : ");
                        fgets(buffer, sizeof(buffer), stdin);
                        buffer[strcspn(buffer, "\n")] = '\0';
                        int count = searchWord(filename, buffer);
                        printf("Le mot %s a ete trouve %d fois dans le fichier %s.\n", buffer, count, filename);
                        break;
                    case 2:
                        dynamic_text_editing(filename);
                        break;
                }
                break;

            case 2:
                printf("Entrez le nom du nouveau fichier : ");
                fgets(buffer, sizeof(buffer), stdin);
                buffer[strcspn(buffer, "\n")] = '\0';
                strcat(filename, buffer);
                strcat(filename, ".txt");
                dynamic_text_editing(filename);
                break;

            case 3:
                printf("Entrez le nom du fichier à supprimer : ");
                fgets(filename, sizeof(filename), stdin);
                filename[strcspn(filename, "\n")] = '\0';
                strcat(filename, ".txt");
                delete_file(filename);
                break;

            case 4:
                display_editor_instructions();
                break;

            case 5:
                printf("Au revoir !\n");
                return 0;

            default:
                printf("Choix invalide. Veuillez reessayer.\n");
        }
    }
}
