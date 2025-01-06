#ifndef INVENTORY_H
#define INVENTORY_H

#define MAX_CHAR_LENGTH 50
#define INVENTORY_FILE "../../data/inventory.txt"

typedef struct {
    int id;
    char name[MAX_CHAR_LENGTH];
    char category[MAX_CHAR_LENGTH];
    int price;
    int stock;
} Product;

// Fonction pour charger les donnees depuis un fichier
int load_data(const char *filename, Product **inventory, int *count);
// Fonction pour sauvegarder les donnees dans un fichier
void save_data(const char *filename, Product *inventory, int count);
// Fonction pour ajouter un produit
int addProduct(Product productToAdd, Product **inventory, int *count);
// Fonction pour trouver l'indice d'un produit par ID
int findIndex(Product *inventory, int count, int id);
// Fonction pour supprimer un produit
int deleteProduct(int id, Product **inventory, int *count);
// Fonction pour mettre à jour un produit
int updateProduct(int id, Product updatedProduct, Product **inventory, int *count);
Product *searchProducts(Product *inventory, int count, int choice, const char *filter, int *foundCount);


#endif
