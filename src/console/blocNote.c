#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "global.h"
#include <ctype.h>
#include "blocNote.h"

void display_editor_instructions() {
    printf("\nFonctionnement de l'editeur :\n");
    printf("1. Ouvrez un fichier existant ou creez un nouveau fichier pour commencer.\n");
    printf("2. Une fois le fichier ouvert, vous pouvez :\n");
    printf("   - Ajouter des lignes en tapant du texte a la fin.\n");
    printf("   - Modifier une ligne existante en tapant \\m suivi du numero de la ligne.\n");
    printf("   - Inserer une ligne apres une ligne specifique en tapant '\\i' suivi du numero de la ligne.\n");
    printf("   - Terminer l'edition en appuyant sur Entree sur une ligne vide.\n");
    printf("3. Enregistrez vos modifications et quittez pour mettre a jour le fichier.\n UwU");
    printf("NB: Le nombre maximal de caractère par ligne est de 256 faite donc attention a ne pas les dépassées");
}

void display_file_content_no_vars(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Le fichier %s n'existe pas.\n", filename);
        return;
    }

    printf("\nContenu du fichier \"%s\" 0o0:\n", filename);
    printf("-------------------------------------------------\n");

    char buffer[MAX_LINE_LENGTH];
    size_t line_number = 1;

    while (fgets(buffer, sizeof(buffer), file)) {
        // Supprimer le saut de ligne s'il existe
        buffer[strcspn(buffer, "\n")] = '\0';
        printf("%zu: %s\n", line_number++, buffer);
    }

    printf("-------------------------------------------------\n");

    fclose(file);
}

// Fonction pour modifier une ligne existante
void edit_line(char ***lines, size_t *num_lines) {
    printf("Entrez le numero de la ligne a modifier : ");
    int line_to_edit;
    if (scanf("%d", &line_to_edit) != 1 || line_to_edit < 1 || (size_t)line_to_edit > *num_lines) {
        printf("Numero de ligne invalide.-_-\n");
        return;
    }

    getchar(); // Consommer le '\n' restant après le numéro
    printf("Ligne actuelle : %s\n", (*lines)[line_to_edit - 1]);
    printf("Entrez le nouveau texte (laisser vide pour supprimer la ligne) : ");
    char buffer[MAX_LINE_LENGTH];
    
    // Lire l'entrée utilisateur
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        printf("Erreur lors de la lecture.\n");
        return;
    }
    
    // Supprimer le saut de ligne
    buffer[strcspn(buffer, "\n")] = '\0';

    if (strlen(buffer) == 0) {
        // Suppression de la ligne
        free((*lines)[line_to_edit - 1]); // Libérer la mémoire de la ligne à supprimer
        for (size_t i = line_to_edit - 1; i < *num_lines - 1; i++) {
            (*lines)[i] = (*lines)[i + 1]; // Décaler les lignes suivantes
        }
        (*num_lines)--; // Réduire le nombre de lignes
        *lines = realloc(*lines, (*num_lines) * sizeof(char *)); // Réallouer la mémoire
        if (*lines == NULL && *num_lines > 0) {
            printf("Erreur d'allocation memoire.\n");
            exit(1);
        }
        printf("Ligne %d supprimee avec succes. WoW !\n", line_to_edit);
    } else {
        // Modification de la ligne
        free((*lines)[line_to_edit - 1]); // Libérer l'ancienne ligne
        (*lines)[line_to_edit - 1] = strdup(buffer); // Copier le nouveau contenu
        if ((*lines)[line_to_edit - 1] == NULL) {
            printf("Erreur d'allocation memoire.\n");
            exit(1);
        }
        printf("Ligne %d modifiee avec succes. ¤_¤ \n", line_to_edit);
    }
}

// Fonction pour inserer une nouvelle ligne
void insert_line(char ***lines, size_t *num_lines) {
    printf("Entrez le numero de la ligne apres laquelle inserer : ");
    int line_to_insert_after;
    if (scanf("%d", &line_to_insert_after) != 1 || line_to_insert_after < 0 || (size_t)line_to_insert_after > *num_lines) {
        printf("Numero de ligne invalide.-_- \n");
    } else {
        getchar(); // Consommer le '\n' restant
        printf("Entrez le texte a inserer Ow^: ");
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

int confirm(char *options[]) {
    int choice = displayMenu(options,2);
    switch (choice) {
            case 1:
            return 1;
            break;
            case 2:
                printf("OK\n");
            return 0;
            break;
            default:
                printf("Choix invalide. Reessayez.\n");
        }
}

void dynamic_text_editing(const char *filename) {
    // Création d'un fichier temporaire
    char temp_filename[] = "../../data/blocNote/tempfile.txt";
    FILE *temp_file = open_file(temp_filename, "w+");
    if (temp_file == NULL) {
        fprintf(stderr, "Erreur: impossible de creer le fichier temporaire.\n");
        return;
    }
    FILE *file = open_file(filename, "r+");
    if (file == NULL) {
        file = open_file(filename, "w+");
        return;
    }
    if (file == NULL) {
        fprintf(stderr, "Erreur: impossible d'ouvrir le fichier source.\n");
        fclose(temp_file);
        remove(temp_filename);
        return;
    }

    char **lines = NULL;
    size_t num_lines = 0;
    display_file_content(file, &lines, &num_lines);

    printf("\nEntrez du texte pour completer, modifier ou inserer une ligne (ligne vide pour terminer) *vv*:\n");

    char buffer[MAX_LINE_LENGTH];
    while (1) {
        printf("%zu > ", num_lines + 1);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            break;
        }

        buffer[strcspn(buffer, "\n")] = '\0'; // Suppression du saut de ligne

        if (strlen(buffer) == 0) {
            break;
        }

        if (buffer[0] == '\\') {
            if (buffer[1] == 'm') {
                edit_line(&lines, &num_lines); // Modifier une ligne existante
            } else if (buffer[1] == 'i') {
                insert_line(&lines, &num_lines); // Insérer une nouvelle ligne
            }
        } else {
            add_new_line(&lines, &num_lines, buffer); // Ajouter une ligne à la fin
        }
    }
    fclose(file);
    char *confirm_option[] = {"Confirmer les modifications","Annuler toutes les modifications"};
    int conf = confirm(confirm_option);
    if(conf == 1){
    // Sauvegarde des modifications dans le fichier temporaire
    save_lines_to_file(temp_filename, temp_file, lines, num_lines);
    fclose(temp_file);

    // Remplacement du fichier original par le fichier temporaire
    if (remove(filename) != 0) {
        fprintf(stderr, "Erreur: impossible de supprimer le fichier original.\n");
        remove(temp_filename); // Nettoyage en cas de problème
        return;
    }
    if (rename(temp_filename, filename) != 0) {
        fprintf(stderr, "Erreur: impossible de renommer le fichier temporaire.\n");
        remove(temp_filename); // Nettoyage en cas de problème
        return;
    }
    printf("Fichier mis a jour avec succes. UwU \n");
    }else{
        fclose(temp_file);
        remove(temp_filename);
        printf("Toutes les modifications sont annulees o_o !");
    }
}

void menu_in_file(char *buffer, char *filename){
    printf("\n\n{}====================(|O-M-O|)==================={}\n");
    display_file_content_no_vars(filename);
    printf("\n");
    char *menu[] = {"Nombre d'occurrences d'un mot", "Modification du fichier", "Retourner au menu principal"};
    int choice2 = displayMenu(menu, 3);
    char buffer1[MAX_LINE_LENGTH];
    switch (choice2) {
        case 1:
            printf("Entrez le mot a rechercher o_ç : ");
            fgets(buffer1, sizeof(buffer1), stdin);
            buffer1[strcspn(buffer1, "\n")] = '\0';
            int count = searchWord(filename, buffer1);
            printf("Le mot %s a ete trouve %d fois dans le fichier %s. [O]_[O]\n", buffer1, count, filename);
            menu_in_file( buffer, filename);
            break;
        case 2:
            dynamic_text_editing(filename);
            menu_in_file( buffer, filename);
            break;
        case 3:
        break;
        default:
    printf("Choix invalide. Veuillez reessayer. o_ç\n");
    }
}

void mainMenu(const char *path, char *filename){
    printf("\n\n{}====================(|O-M-O|)==================={}\n");
    printf("\nMenu principal $_$:\n");
    char *mainMenuOptions[] = {
        "Ouvrir un fichier existant",
        "Creer un nouveau fichier",
        "Supprimer un fichier",
        "Afficher le fonctionnement de l'editeur",
        "Quitter"
    };
    char *leaveMenuOption[] ={"Quitter", "Ne pas quitter"};
    int choice = displayMenu(mainMenuOptions, 5);
    char buffer[MAX_LINE_LENGTH];
    switch (choice) {
        case 1:
        printf("\n");
            list_files_in_directory(path);
            printf("Entrez le nom du fichier a ouvrir [0]_[0]: ");
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = '\0';
            strcat(filename, buffer);
            strcat(filename, ".txt");
            menu_in_file( buffer, filename);
            strcpy(filename, "../../data/blocNote/");
            mainMenu(path,filename);
            break;

        case 2:
            printf("Entrez le nom du nouveau fichier [o]_[o]: ");
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strcspn(buffer, "\n")] = '\0';
            strcat(filename, buffer);
            strcat(filename, ".txt");
            dynamic_text_editing(filename);
            strcpy(filename, "../../data/blocNote/");
            mainMenu(path,filename);
            break;

        case 3:
            printf("Entrez le nom du fichier a supprimer : ");
            fgets(filename, sizeof(filename), stdin);
            filename[strcspn(filename, "\n")] = '\0';
            strcat(filename, ".txt");
            char *confirm_option[] = {"Confirmer la suppression","Annuler la suppression"};
            if(confirm(confirm_option)==1){
            delete_file(filename);
            }else{
                printf("Suppression annulee, retour au menu principal §_§");
            }
            strcpy(filename, "../../data/blocNote/");
            mainMenu(path,filename);
            break;

        case 4:
            display_editor_instructions();
            strcpy(filename, "../../data/blocNote/");
            mainMenu(path,filename);
            break;

        case 5:
        if(confirm(leaveMenuOption)==1){
            printf("Au revoir, cher utilisateur ^o^ !\n");
            exit(0);
            }else{
            strcpy(filename, "../../data/blocNote/");
                mainMenu(path,filename);
            }

        default:
            printf("Choix invalide. Veuillez reessayer. o_ç\n");
    }
}

int main() {
    const char *path = "../../data/blocNote";

    while (1) {
        char filename[MAX_LINE_LENGTH] = "../../data/blocNote/";
        mainMenu(path,filename);
    }
}
