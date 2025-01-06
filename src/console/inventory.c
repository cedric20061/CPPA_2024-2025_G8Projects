#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inventory.h"
#include "global.h"

// Afficher tous les produits
void listAllProducts(Product *inventory, int count) {
    printf("----- Liste de tous les produits -----\n");
    printf("| %-5s | %-20s | %-15s | %-10s | %-10s |\n", "N", "Nom", "Categorie", "Prix", "Stock");
    printf("--------------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        printf("| %-5d | %-20s | %-15s | %-10d | %-10d |\n",
               i+1, inventory[i].name, inventory[i].category,
               inventory[i].price, inventory[i].stock);
    }
    printf("--------------------------------------------------------------\n\n");
}

// Recupere les caracteristique d'un produit
void getProductDetails(Product *product) {
    printf("Saisissez le nom : ");
    fgets(product->name, sizeof(product->name), stdin);
    product->name[strcspn(product->name, "\n")] = '\0';
    printf("Saisissez la categorie : ");
    fgets(product->category, sizeof(product->category), stdin);
    product->category[strcspn(product->category, "\n")] = '\0';
    printf("Saisissez le prix : ");
    scanf("%d", &product->price);
    printf("Saisissez le stock : ");
    scanf("%d", &product->stock);
    while (getchar() != '\n' && getchar() != EOF);
}

// Effectue les actions principales sur un produit
void productActions (Product **inventory, Product *filteredProducts, int *foundCount, int *count) {
    printf("Voulez-vous modifier ou supprimer un produit ?\n");
    char *options[] = {"Modifier", "Supprimer", "Aucun"};
    int action = displayMenu(options, 3);

    if (action == 1 || action == 2) {
        int productIndex = getSafeInt("Selectionnez le produit par son numero dans la liste: ");
        
        if (productIndex < 1 || productIndex > *foundCount) {
            printf("Numero de produit invalide.\n\n");
        } else {
            Product selectedProduct = filteredProducts[productIndex - 1];
            if (action == 1) {
                printf("-----Modification du produit numero %d :-----\n", productIndex);
                getProductDetails(&selectedProduct);
                updateProduct(selectedProduct.id, selectedProduct, inventory, count);
            } else if (action == 2) {
                deleteProduct(selectedProduct.id, inventory, count);
            }
        }
    } else if (action == 3) {
        printf("Aucune action effectuee.\n\n");
    } else {
        printf("Action invalide.\n\n");
    }
}

// Recherche d'un produit
void productSearching(Product **inventory, int *count) {
    if (*count == 0) {
        printf("L'inventaire est vide.\n\n");
        return;
    }

    printf("Effectuez une recherche par: \n");
    int choice = displayMenu((char *[]) {"Nom", "Categorie"}, 2);

    char filter[MAX_CHAR_LENGTH];
    printf("Saisissez le %s : ", choice == 1 ? "nom" : "categorie");

    fgets(filter, sizeof(filter), stdin);
    filter[strcspn(filter, "\n")] = '\0';

    int foundCount;
    Product *filteredProducts = searchProducts(*inventory, *count, choice, filter, &foundCount);

    if (!foundCount) {
        printf("Aucun produit trouve.\n");
        printf("--------------------------------------------------------------\n\n");
        free(filteredProducts);
        return;
    }

    listAllProducts(filteredProducts, foundCount);
    productActions(inventory, filteredProducts, &foundCount, count);
    
    free(filteredProducts);
}

// Menu principal
void menu(Product **inventory, int *count) {
    int isRunning = 1;
    
    while (isRunning) {
        printf("----- Menu -----\n");
        char *options[] = {"Rechercher un produit", "Ajouter un produit", "Lister tous les produits", "Quitter"};
        int choice = displayMenu(options, 4);

        switch (choice) {
            case 1:
                productSearching(inventory, count);
                break;
            case 2: {
                Product newProduct;
                getProductDetails(&newProduct);
                addProduct(newProduct, inventory, count);
                break;
            }
            case 3:
                listAllProducts(*inventory, *count);
                productActions(inventory, *inventory, count, count);
                break;
            case 4:
                printf("Merci d'avoir utilise le programme !\n");
                isRunning = 0;
                break;
            default:
                printf("Choix invalide, veuillez reessayer.\n\n");
        }
    }
}

int main() {
    Product *inventory = malloc(sizeof(Product));
    int count = 0;

    if (load_data(INVENTORY_FILE, &inventory, &count) == -1) {
        free(inventory);
        return 1;
    }

    menu(&inventory, &count);

    save_data(INVENTORY_FILE, inventory, count);
    free(inventory);
    return 0;
}
