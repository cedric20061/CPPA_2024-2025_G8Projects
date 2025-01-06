#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "game.h"
#include "global.h"

void numberGuessing(User *user, int numberToGuess, int maxTries, int level) {
    int tries = 0;
    int userInput = -1;

    while (1) {
        userInput = getSafeInt("Entrez votre proposition : ");
        tries++;
        printf(userInput < numberToGuess ? "Plus grand\n" : userInput > numberToGuess? "Plus petit\n" : "Gagne !\n");
        if(userInput == numberToGuess) {
            printf("Bravo %s, vous avez gagne en %d essais !\n", user->name, tries);
            updateScores(user, level, tries);
            saveScores(user);
            break;
        }

        if (maxTries > 0 && tries >= maxTries) {
            printf("Desole, vous avez perdu.\nLe nombre a trouver etait : %d\n", numberToGuess);
            break;
        }

        if (maxTries > 0) {
            printf("Il vous reste %d tentatives.\n", maxTries - tries);
        }
    }
}

// Lance le jeu pour un utilisateur
void playGame(User *currentUser) {
    char *levelOptions[4];
    for (int i=0; i<4; i++){
        levelOptions[i] = gamesLevels[i].name;
    }

    printf("\n=======Choisissez votre niveau de difficulte :=======\n");
    int level = displayMenu(levelOptions, 4);

    int numberToGuess = generate_random_number(0, gamesLevels[level - 1].end);
    int maxTries = gamesLevels[level - 1].tries;

    printf("Le jeu commence ! Bonne chance !\n");
    numberGuessing(currentUser, numberToGuess, maxTries, level);
}


// Affiche les statistiques des utilisateurs
void displayStatistics(User *users, int userCount) {
    if (userCount == 0) {
        printf("Aucun utilisateur pour afficher des statistiques.\n");
        return;
    }
    printf("\n========== Statistiques des joueurs ==========\n");
    const char *levels[] = {"Facile", "Moyen", "Difficile", "Expert"};

    for (int level = 0; level < 4; level++) {
        printf("\n%s:\n", levels[level]);

        // Creer un tableau temporaire pour le classement
        User *sortedUsers = malloc(userCount * sizeof(User));
        int sortedCount = 0;

        // Ajouter uniquement les utilisateurs ayant un score non nul pour ce niveau
        for (int i = 0; i < userCount; i++) {
            if (users[i].bestScores[level] > 0) {
                sortedUsers[sortedCount++] = users[i];
            }
        }

        // Trier les utilisateurs par score croissant
        for (int i = 0; i < sortedCount - 1; i++) {
            for (int j = i + 1; j < sortedCount; j++) {
                if (sortedUsers[i].bestScores[level] > sortedUsers[j].bestScores[level]) {
                    User temp = sortedUsers[i];
                    sortedUsers[i] = sortedUsers[j];
                    sortedUsers[j] = temp;
                }
            }
        }

        // Afficher les informations triees
        if (sortedCount == 0) {
            printf("  Aucun joueur n'a de score pour ce niveau.\n");
        } else {
            for (int i = 0; i < sortedCount; i++) {
                printf("  %d. %s (%d points)\n", i + 1, sortedUsers[i].name, sortedUsers[i].bestScores[level]);
            }
        }

        free(sortedUsers); // Liberer la memoire
    }
    printf("===============================================\n");
}

int main() {
    printf("======= Bienvenue dans Guess the Number =======\n");
    printf("Le principe est simple : Trouvez le nombre aleatoire.\n");

    srand(time(NULL)); // Initialiser la graine pour les nombres aleatoires.

    int userCount = 0;
    User *users = loadUsers(&userCount);
    if (users == NULL) {
        printf("Aucun utilisateur trouve.\n");
        userCount = 0;
    }

    while (1) {
        printf("\nMenu principal :\n");

        // Creer le menu principal avec des options fixes.
        char *mainMenuOptions[] = {
            "Se connecter",
            "Creer un nouvel utilisateur",
            "Voir les statistiques",
            "Quitter"
        };

        int mainChoice = displayMenu(mainMenuOptions, 4); // Afficher le menu principal.

        if (mainChoice == 1) { // Se connecter.
            if (userCount == 0) {
                printf("Aucun utilisateur n'est disponible. Veuillez en creer un d'abord.\n");
                continue;
            }

            // Afficher la liste des utilisateurs disponibles pour la connexion.
            printf("\nListe des utilisateurs disponibles :\n");

            char **userMenuOptions = malloc((userCount + 1) * sizeof(char *));
            if (!userMenuOptions) {
                fprintf(stderr, "Erreur d'allocation memoire pour les utilisateurs.\n");
                exit(EXIT_FAILURE);
            }

            for (int i = 0; i < userCount; i++) {
                userMenuOptions[i] = strdup(users[i].name); // Copier le nom de l'utilisateur.
            }
            userMenuOptions[userCount] = strdup("Retour au menu principal");

            int userChoice = displayMenu(userMenuOptions, userCount + 1);

            if (userChoice >= 0 && userChoice < userCount) {
                User *currentUser = &users[userChoice];
                loadScores(currentUser);
                playGame(currentUser);
            }

            // Liberer la memoire allouee pour le menu des utilisateurs.
            for (int i = 0; i < userCount + 1; i++) {
                free(userMenuOptions[i]);
            }
            free(userMenuOptions);

        } else if (mainChoice == 2) { // Creer un nouvel utilisateur.
            char name[MAX_NAME_LENGTH];
            int addUserResult = 0;

            do {
                printf("Entrez le nom du nouvel utilisateur : ");
                fgets(name, sizeof(name), stdin);

                // Supprimer le caractere de nouvelle ligne '\n' si present.
                name[strcspn(name, "\n")] = '\0';

                addUserResult = addUser(&users, &userCount, name);

                if (addUserResult == 0) {
                    printf("Le nom est invalide ou dejà utilise. Veuillez reessayer.\n");
                }
            } while (addUserResult == 0);

            User *currentUser = &users[userCount - 1]; // Selectionner le nouvel utilisateur.
            loadScores(currentUser);
            playGame(currentUser);
            users = loadUsers(&userCount);

        } else if (mainChoice == 3) { // Voir les statistiques.
            displayStatistics(users, userCount);

        } else if (mainChoice == 4) { // Quitter.
            printf("Merci d'avoir joue ! À bientôt.\n");
            break;
        }

        // Liberer la memoire des utilisateurs.
    }

    free(users);
    return 0;
}
