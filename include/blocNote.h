#ifndef BLOCNOTE_H
#define BLOCNOTE_H

#define MAX_LINE_LENGTH 256

// Affiche le contenue d'un fichier
void display_file_content(FILE *file, char ***lines, size_t *num_lines);
// Supprime une ligne dans le fichier
void delete_file(char *filename);
// Recherche le nombre d'occurence d'un mot dans le fichier
int searchWord(char *filename, char *word);
// Liste les fichiers présent dans un répèrtoire
void list_files_in_directory(const char *path);
// Fonction pour ouvrir un fichier
FILE *open_file(const char *filename, const char *mode);

#endif
