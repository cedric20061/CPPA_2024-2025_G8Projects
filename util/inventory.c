#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "inventory.h"

// Fonction pour creer un repertoire recursivement
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

// Charge l'inventaire
int load_data(const char *filename, Product **inventory, int *count) {
    char directory[PATH_MAX];
    strncpy(directory, filename, PATH_MAX);

    // Supprimer le nom du fichier pour obtenir le chemin du repertoire
    char *last_slash = strrchr(directory, '/');
    if (last_slash) {
        *last_slash = '\0';

        // Creer le repertoire si necessaire
        if (create_directory_recursively(directory) != 0) {
            fprintf(stderr, "Erreur lors de la creation du repertoire pour \"%s\".\n", filename);
            return -1;
        }
    }

    FILE *file = fopen(filename, "r");

    if (!file) {
        file = fopen(filename, "w");
        if (!file) {
            perror("Erreur de creation du fichier");
            return -1;
        }
        fclose(file);
        printf("Fichier \"%s\" cree avec succes.\n", filename);
        *count = 0;
        return 0;
    }

    char line[256];
    *count = 0;

    while (fgets(line, sizeof(line), file)) {
        *inventory = realloc(*inventory, sizeof(Product) * (*count + 1));
        if (*inventory == NULL) {
            perror("Erreur de memoire lors du realloc");
            fclose(file);
            return -1;
        }

        if (sscanf(line, "%d,%49[^,],%49[^,],%d,%d",
                   &(*inventory)[*count].id,
                   (*inventory)[*count].name,
                   (*inventory)[*count].category,
                   &(*inventory)[*count].price,
                   &(*inventory)[*count].stock) == 5) {
            (*count)++;
        } else {
            printf("Ligne ignoree (format invalide) : %s", line);
        }
    }

    fclose(file);
    return 0;
}

//Sauvegarde l'inventaire
void save_data(const char *filename, Product *inventory, int count) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Erreur d'ouverture du fichier pour ecriture");
        return;
    }

    for (int i = 0; i < count; i++) {
        if (fprintf(file, "%d,%s,%s,%d,%d\n",
                    inventory[i].id,
                    inventory[i].name,
                    inventory[i].category,
                    inventory[i].price,
                    inventory[i].stock) < 0) {
            perror("Erreur d'ecriture dans le fichier");
            fclose(file);
            return;
        }
    }
    fclose(file);
    printf("Donnees sauvegardees avec succes dans %s.\n", filename);
}


int addProduct(Product productToAdd, Product **inventory, int *count) {
    productToAdd.id = *count? (*inventory)[*count - 1].id + 1 : 0;

    *inventory = realloc(*inventory, sizeof(Product) * (*count + 1));
    if (*inventory == NULL) {
        perror("Erreur de memoire lors du realloc");
        return -1;
    }

    (*inventory)[*count] = productToAdd;
    (*count)++;
    printf("Produit ajoute avec succes !\n\n");
    save_data(INVENTORY_FILE, *(inventory), *(count));
    return 1;
}

int findIndex(Product *inventory, int count, int id) {
    for (int i = 0; i < count; i++) {
        if (inventory[i].id == id) {
            return i;
        }
    }
    return -1;
}

int deleteProduct(int id, Product **inventory, int *count) {
    int productIndex = findIndex(*inventory, *count, id);
    if (productIndex != -1) {
        // Deplacer les elements apres l'element à supprimer vers la gauche
        for (int i = productIndex; i < *count - 1; i++) {
            (*inventory)[i] = (*inventory)[i + 1];
        }

        // Reallocation de la memoire apres la suppression
        *inventory = realloc(*inventory, sizeof(Product) * (*count - 1));
        if (*inventory == NULL && *count - 1 > 0) {
            perror("Erreur de memoire lors du realloc");
            return -1;
        }

        (*count)--; // Decremente le compteur de produits
        save_data(INVENTORY_FILE, *(inventory), *(count));
        printf("Produit supprime avec succes !\n");
        return 1;
    } else {
        printf("Produit introuvable.\n\n");
        return 0;
    }
}

int updateProduct(int id, Product updatedProduct, Product **inventory, int *count) {
    
    int productIndex = findIndex(*inventory, *count, id);
    if (productIndex != -1) {
        updatedProduct.id = id;
        (*inventory)[productIndex] = updatedProduct;
        save_data(INVENTORY_FILE, *(inventory), *(count));
        printf("Produit mis a jour avec succes.\n\n");
        return 1;
    } else {
        printf("Produit introuvable.\n\n");
        return 0;
    }
}

char *toLowerCase(const char *str) {
    // Allouer de la memoire pour la nouvelle chaîne
    char *lowerStr = malloc(strlen(str) + 1);
    if (!lowerStr) {
        perror("Erreur d'allocation memoire");
        exit(EXIT_FAILURE);
    }

    // Parcourir la chaîne d'entree et convertir en minuscules
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            lowerStr[i] = str[i] + ('a' - 'A');
        } else {
            lowerStr[i] = str[i];
        }
    }

    // Ajouter le caractere de fin de chaîne
    lowerStr[strlen(str)] = '\0';

    return lowerStr;
}


Product *searchProducts(Product *inventory, int count, int choice, const char *filter, int *foundCount) {
    // Initialiser la liste des produits trouves
    Product *filteredProducts = malloc(sizeof(Product));
    if (!filteredProducts) {
        printf("Erreur de memoire.\n");
        *foundCount = 0;
        return NULL;
    }

    *foundCount = 0;

    for (int i = 0; i < count; i++) {
        if ((choice == 1 && strncmp(toLowerCase(inventory[i].name), toLowerCase(filter), strlen(filter)) == 0) ||
            (choice == 2 && strncmp(toLowerCase(inventory[i].category), toLowerCase(filter), strlen(filter)) == 0)) {
            filteredProducts = realloc(filteredProducts, sizeof(Product) * (*foundCount + 1));
            if (!filteredProducts) {
                printf("Erreur de memoire.\n");
                *foundCount = 0;
                return NULL;
            }
            filteredProducts[*foundCount] = inventory[i];
            (*foundCount)++;
        }
    }

    return filteredProducts;
}
