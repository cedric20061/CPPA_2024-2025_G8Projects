#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "game.h"
#include <time.h>

void test_generate_random_number() {
    int min = 1, max = 100;
    for (int i = 0; i < 1000; i++) {
        int num = generate_random_number(min, max);
        assert(num >= min && num <= max);
    }
    printf("test_generate_random_number passed\n");
}

void test_cleanString() {
    char str1[] = "  Hello  ";
    cleanString(str1);
    assert(strcmp(str1, "Hello") == 0);

    char str2[] = "\nWorld\n";
    cleanString(str2);
    assert(strcmp(str2, "World") == 0);

    printf("test_cleanString passed\n");
}
// Fonction pour generer un nom aleatoire de 5 caracteres
void generateRandomName(char *name, int length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (int i = 0; i < length; i++) {
        int randomIndex = rand() % (sizeof(charset) - 1);
        name[i] = charset[randomIndex];
    }
    name[length] = '\0'; // Terminer la chaîne par un caractere nul
}

void test_addUser() {
    User *users = NULL; // Liste d'utilisateurs initialement vide
    int userCount = 0;

    // Initialiser le generateur de nombres aleatoires
    srand(time(NULL));

    // Test 1 : Ajouter un nouvel utilisateur avec un nom aleatoire
    char name1[5 + 1];
    generateRandomName(name1, 5);
    int result = addUser(&users, &userCount, name1);
    assert(result == 1); // L'utilisateur doit être ajoute avec succes
    assert(strcmp(users[userCount-1].name, name1) == 0); // Le nom doit correspondre à name1

    // Test 2 : Ajouter un utilisateur avec un nom aleatoire dejà existant
    result = addUser(&users, &userCount, name1);
    assert(result == 0); // L'ajout doit echouer car le nom existe dejà

    // Test 3 : Ajouter un nouvel utilisateur avec un autre nom aleatoire
    char name2[5 + 1];
    generateRandomName(name2, 5);
    result = addUser(&users, &userCount, name2);
    assert(result == 1); // L'utilisateur doit être ajoute avec succes
    assert(strcmp(users[userCount-1].name, name2) == 0); // Le nom doit correspondre à name2

    // Nettoyer la memoire allouee
    free(users);
    printf("Tous les tests pour addUser ont reussi.\n");
}


void test_updateScores() {
    User user = {.name = "Test3", .bestScores = {0, 0, 0, 0}, .worstScores = {0, 0, 0, 0}};
    updateScores(&user, 1, 5);
    assert(user.bestScores[0] == 5);
    assert(user.worstScores[0] == 5);

    updateScores(&user, 1, 3);
    assert(user.bestScores[0] == 3);
    assert(user.worstScores[0] == 5);

    updateScores(&user, 1, 10);
    assert(user.bestScores[0] == 3);
    assert(user.worstScores[0] == 10);

    printf("test_updateScores passed\n");
}

void test_loadUsers() {
    int userCount = 0;
    User *users = loadUsers(&userCount);


    free(users);
    printf("test_loadUsers passed\n");
}

void test_saveScores() {
    User user = {.name = "Charlie", .bestScores = {2, 0, 0, 0}, .worstScores = {15, 0, 0, 0}};
    saveScores(&user);

    FILE *file = fopen(SCORE_FILE, "r");
    char buffer[BUFFER_SIZE];
    fgets(buffer, BUFFER_SIZE, file);

    fclose(file);
    printf("test_saveScores passed\n");
}

int main() {
    printf("Running tests...\n");

    test_generate_random_number();
    test_cleanString();
    test_updateScores();
    test_addUser();
    test_loadUsers();
    test_saveScores();

    printf("Tests completed.\n");
    return 0;
}