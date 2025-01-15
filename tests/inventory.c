#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "inventory.h"

void test_addProduct() {
    Product *inventory = NULL;
    int count = 0;

    // Chargement des données existantes (si disponibles)
    load_data(INVENTORY_FILE, &inventory, &count);

    // Ajouter des produits réalistes à l'inventaire
    Product products[] = {
        {0, "iPhone 15", "Smartphone", 350000, 50},
        {0, "Samsung Galaxy S23", "Smartphone", 320000, 30},
        {0, "Huawei P50", "Smartphone", 250000, 40},
        {0, "MacBook Pro 16", "Ordinateur", 2200000, 15},
        {0, "Dell XPS 13", "Ordinateur", 1300000, 20},
        {0, "HP Spectre x360", "Ordinateur", 1500000, 10},
        {0, "LG OLED TV 55\"", "Télévision", 850000, 25},
        {0, "Samsung QLED 55\"", "Télévision", 950000, 30},
        {0, "Sony Bravia 65\"", "Télévision", 1200000, 12},
        {0, "Sony PlayStation 5", "Console", 300000, 20},
        {0, "Microsoft Xbox Series X", "Console", 270000, 25},
        {0, "Nintendo Switch", "Console", 150000, 40},
        {0, "Apple AirPods Pro 2", "Accessoire", 150000, 60},
        {0, "Bose QuietComfort 35", "Accessoire", 180000, 50},
        {0, "JBL Charge 5", "Accessoire", 120000, 70},
        {0, "Canon EOS 80D", "Appareil photo", 950000, 15},
        {0, "Nikon D7500", "Appareil photo", 850000, 20},
        {0, "GoPro Hero 10", "Caméra", 400000, 25},
        {0, "Samsung Galaxy Tab S8", "Tablette", 600000, 18},
        {0, "Apple iPad Pro 11\"", "Tablette", 800000, 22}
    };

    // Ajouter chaque produit à l'inventaire
    for (int i = 0; i < 20; i++) {
        int result = addProduct(products[i], &inventory, &count);

        assert(result == 1);
        assert(strcmp(inventory[count - 1].name, products[i].name) == 0);
        assert(strcmp(inventory[count - 1].category, products[i].category) == 0);
        assert(inventory[count - 1].price == products[i].price);
        assert(inventory[count - 1].stock == products[i].stock);
    }

    // Vérifier le dernier produit ajouté
    Product newProduct = {0, "ProduitTest", "CategorieTest", 100, 10};
    int result = addProduct(newProduct, &inventory, &count);

    assert(result == 1);
    assert(strcmp(inventory[count - 1].name, "ProduitTest") == 0);
    assert(strcmp(inventory[count - 1].category, "CategorieTest") == 0);
    assert(inventory[count - 1].price == 100);
    assert(inventory[count - 1].stock == 10);

    // Libération de la mémoire
    free(inventory);
}


void test_deleteProduct() {
    Product *inventory=NULL;
    int count;
    load_data(INVENTORY_FILE, &inventory, &count);

    int result = deleteProduct(20, &inventory, &count);

    assert(result == 1);

    free(inventory);
}

void test_updateProduct() {
    Product *inventory = NULL;
    int count;
    load_data(INVENTORY_FILE, &inventory, &count);
    Product updatedProduct = {0, "ProduitUpdated", "CategorieUpdated", 300, 15};

    int result = updateProduct(2, updatedProduct, &inventory, &count);
    
    int index = findIndex(inventory, count, 2);

    assert(result == 1);
    assert(strcmp(inventory[index].name, "ProduitUpdated") == 0);
    assert(strcmp(inventory[index].category, "CategorieUpdated") == 0);
    assert(inventory[index].price == 300);
    assert(inventory[index].stock == 15);

    free(inventory);
}

void test_searchProducts() {
    Product *inventory = malloc(sizeof(Product) * 3);
    int count = 3;

    inventory[0] = (Product){1, "ProduitA", "Categorie1", 100, 10};
    inventory[1] = (Product){2, "ProduitB", "Categorie1", 200, 5};
    inventory[2] = (Product){3, "ProduitC", "Categorie2", 300, 15};

    int foundCount = 0;
    Product *results = searchProducts(inventory, count, 1, "ProduitA", &foundCount);

    assert(foundCount == 1);
    assert(strcmp(results[0].name, "ProduitA") == 0);
    assert(results[0].price == 100);
    assert(results[0].stock == 10);

    free(results);
    free(inventory);
}

void test_save_and_load_data() {
    Product *loadedInventory = NULL;
    int loadedCount = 0;

    int result = load_data(INVENTORY_FILE, &loadedInventory, &loadedCount);

    assert(!result);

    free(loadedInventory);
}

int main() {
    printf("Running tests...\n");

    test_addProduct();
    test_deleteProduct();
    test_updateProduct();
    test_searchProducts();
    test_save_and_load_data();

    printf("All tests passed!\n");
    return 0;
}
