#ifndef GLOBAL_H
#define GLOBAL_H

#define MAX_CHAR_LENGTH 50

// Affiche le menu avec un tableau de chaînes et vérifie que le choix est valide
int displayMenu(char *menu[], int count);
// Récupère de facon sécurisé une chaine de caractère
int getSafeInt(char *prompt);

#endif
