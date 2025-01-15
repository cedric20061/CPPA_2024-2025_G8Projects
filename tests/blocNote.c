#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "blocNote.h"

#define TEST_FILENAME "../../data/blocNote/test.txt"
#define TEST_FILE_TO_DELETE "../../data/blocNote/test2.txt"
void test_open_file() {
    // Test 1 : Ouvrir un fichier valide
    FILE *file = open_file(TEST_FILENAME, "w");
    assert(file != NULL);
    fclose(file);

    // Test 2 : Ouvrir un fichier invalide
    file = open_file("non_existent_file.txt", "r");
    assert(file == NULL);
    printf("test_open_file passed\n");
}

void test_display_file_content() {
    FILE *file = fopen(TEST_FILENAME, "w");
    fprintf(file, "Hello World\n");
    fprintf(file, "This is a test file.\n");
    fclose(file);

    char **lines = NULL;
    size_t num_lines = 0;
    file = open_file(TEST_FILENAME, "r");
    display_file_content(file, &lines, &num_lines);
    fclose(file);

    // Vérifier que le nombre de lignes est correct
    assert(num_lines == 2);
    assert(strcmp(lines[0], "Hello World") == 0);
    assert(strcmp(lines[1], "This is a test file.") == 0);

    // Libérer la mémoire
    for (size_t i = 0; i < num_lines; i++) {
        free(lines[i]);
    }
    free(lines);
    printf("test_display_file_content passed\n");
}

void test_delete_file() {
    // Test 1 : Créer et supprimer un fichier
    FILE *file = fopen(TEST_FILE_TO_DELETE, "w");
    fprintf(file, "This file will be deleted.\n");
    fclose(file);

    delete_file(TEST_FILE_TO_DELETE);

    // Vérifier que le fichier a été supprimé
    file = fopen(TEST_FILE_TO_DELETE, "r");
    assert(file == NULL);
    if (file != NULL) fclose(file);

    printf("test_delete_file passed\n");
}

void test_searchWord() {
    // Créer un fichier de test
    FILE *file = fopen(TEST_FILENAME, "w");
    fprintf(file, "This is a test file with the word test in it.\n");
    fprintf(file, "The word test appears twice in this file.\n");
    fclose(file);

    // Test 1 : Chercher un mot présent dans le fichier
    int count = searchWord(TEST_FILENAME, "test");
    assert(count == 3); // Le mot 'test' apparaît 3 fois

    // Test 2 : Chercher un mot non présent dans le fichier
    count = searchWord(TEST_FILENAME, "notfound");
    assert(count == 0); // Le mot 'notfound' n'est pas dans le fichier

    printf("test_searchWord passed\n");
}

int main() {
    printf("Running tests...\n");

    test_open_file();
    test_display_file_content();
    test_delete_file();
    test_searchWord();

    printf("Tests completed.\n");
    return 0;
}
