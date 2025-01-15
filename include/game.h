#ifndef GAME_H
#define GAME_H

#define MAX_NAME_LENGTH 150
#define MAX_USERS 100
#define SCORE_FILE "../../data/game.txt"
#define TEMP_FILE "../../data/temp.txt"
#define BUFFER_SIZE 256

typedef struct {
    char name[MAX_NAME_LENGTH];
    int end;
    int tries;
} Levels;

typedef struct {
    char name[MAX_NAME_LENGTH];
    int bestScores[4];
    int worstScores[4];
} User;

// Liste des niveaux de jeu
extern Levels gamesLevels[];

// Fonction utilitaire pour nettoyer une chaîne
void cleanString(char *str);
// Génère un nombre aléatoire dans un intervalle donnée
int generate_random_number(int min, int max);
// Récupère la liste des utilisateur
User *loadUsers(int *userCount);
// Sauvegarde les utilisateurs
void saveUsers(User users[], int count);
// Charge les statistiques de différents utilisateurs
void loadScores(User *user);
// Sauvegarde les scores des utilisateurs
void saveScores(User *user);
// Ajoute un nouveau utilisateur
int addUser(User **users, int *userCount, char *name);
// Met à jour le score d'un utilisateur
void updateScores(User *user, int level, int score);

#endif
