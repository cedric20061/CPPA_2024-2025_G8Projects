#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>
#include "game.h"

typedef struct {
    GtkWidget *entry;
    GtkWidget *feedback_label;
    User *user;
    int level;
} GameWidgets;

// Structure pour stocker les données de classement par niveau
typedef struct {
    char *name;
    int score;
    int rank;
} RankingEntry;


// Globals
GtkWidget *window;
GtkWidget *content_area;
int numberToGuess;
int userNumber;
User *allUsers = NULL;
int maxTries;
int tries;
User current_user;

// Function prototypes
void show_login_menu();
void show_statistics_menu();
void show_create_user_menu();
void show_difficulty_menu(GtkWidget *widget, gpointer data);
void start_game(GtkWidget *widget, gpointer data);
void update_game_interface(GtkWidget *entry, gpointer data);

// Utility functions
void clear_window() {
    GList *children = gtk_container_get_children(GTK_CONTAINER(content_area));
    for (GList *child = children; child != NULL; child = child->next) {
        gtk_widget_destroy(GTK_WIDGET(child->data));
    }
    g_list_free(children);
}

// Fonction pour créer un CSS provider global
void load_css() {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider,"../../styles/game.css", NULL);
    
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(provider);
}

void on_connect_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *user_list = GTK_WIDGET(data);
    const gchar *selected_user_name = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(user_list));

    if (selected_user_name != NULL && selected_user_name[0] != '\0') {
        // Chercher l'utilisateur sélectionné dans la liste des utilisateurs
        User *selected_user = NULL;
        for (int i = 0; i < userNumber; i++) {
            if (g_strcmp0(allUsers[i].name, selected_user_name) == 0) {
                selected_user = &allUsers[i];
                current_user = *selected_user;
                break;
            }
        }

        if (selected_user != NULL) {
            // Passer l'utilisateur sélectionné à `show_difficulty_menu`
            show_difficulty_menu(widget, selected_user);
        }
    } else {
        // Si aucun utilisateur n'est sélectionné, afficher un message d'erreur
        GtkWidget *error_label = gtk_label_new("Veuillez sélectionner un utilisateur.");
        gtk_container_add(GTK_CONTAINER(content_area), error_label);
        gtk_widget_show_all(content_area);
    }
}

void on_create_user_button_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *entry = GTK_WIDGET(data);
    const char *new_user_name = gtk_entry_get_text(GTK_ENTRY(entry));

    if (new_user_name != NULL && new_user_name[0] != '\0') {
        // Supprimer les espaces superflus et valider le nom
        char trimmed_name[MAX_NAME_LENGTH];
        strncpy(trimmed_name, new_user_name, MAX_NAME_LENGTH - 1);
        trimmed_name[MAX_NAME_LENGTH - 1] = '\0';

        // Appeler la fonction addUser
        int add_result = addUser(&allUsers, &userNumber, trimmed_name);

        if (add_result == 1) {
            // Si l'utilisateur est ajouté avec succès, passer au menu de difficulté
            show_difficulty_menu(widget, &allUsers[userNumber - 1]);
        } else if (add_result == 0) {
            // Si le nom est déjà utilisé, afficher un message d'erreur
            GtkWidget *error_label = gtk_label_new("Nom déjà utilisé. Veuillez en essayer un autre.");
            gtk_container_add(GTK_CONTAINER(content_area), error_label);
            gtk_widget_show_all(content_area);
        } else {
            // En cas d'erreur de mémoire
            GtkWidget *error_label = gtk_label_new("Erreur d'allocation mémoire.");
            gtk_container_add(GTK_CONTAINER(content_area), error_label);
            gtk_widget_show_all(content_area);
        }
    } else {
        // Si le champ est vide, afficher un message d'erreur
        GtkWidget *error_label = gtk_label_new("Veuillez entrer un nom valide.");
        gtk_container_add(GTK_CONTAINER(content_area), error_label);
        gtk_widget_show_all(content_area);
    }
}


// Fonction utilitaire pour créer un conteneur stylisé
GtkWidget* create_styled_box() {
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_widget_set_margin_start(box, 40);
    gtk_widget_set_margin_end(box, 40);
    gtk_widget_set_margin_top(box, 30);
    gtk_widget_set_margin_bottom(box, 30);
    return box;
}

// Fonction pour styliser un bouton
void style_button(GtkWidget *button) {
    gtk_widget_set_size_request(button, 200, 40);
    GtkStyleContext *context = gtk_widget_get_style_context(button);
    gtk_style_context_add_class(context, "suggested-action");
}

// Fonction pour styliser un label titre
void style_title_label(GtkWidget *label) {
    char *markup = g_markup_printf_escaped(
        "<span font='20' weight='bold'>%s</span>",
        gtk_label_get_text(GTK_LABEL(label))
    );
    gtk_label_set_markup(GTK_LABEL(label), markup);
    g_free(markup);
}


void show_login_menu() {
    clear_window();

    GtkWidget *main_box = create_styled_box();
    GtkWidget *label = gtk_label_new("Guess the Number");
    style_title_label(label);

    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_style_context_add_class(gtk_widget_get_style_context(card), "stats-view");

    GtkWidget *user_list = gtk_combo_box_text_new();
    gtk_widget_set_size_request(user_list, 200, 40);
    
    for (int i = 0; i < userNumber; i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(user_list), allUsers[i].name);
    }

    GtkWidget *buttons_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(buttons_box, GTK_ALIGN_CENTER);

    GtkWidget *connect_button = gtk_button_new_with_label("Se connecter");
    GtkWidget *create_user_button = gtk_button_new_with_label("Créer un utilisateur");
    GtkWidget *view_stats_button = gtk_button_new_with_label("Voir les statistiques");

    style_button(connect_button);
    style_button(create_user_button);
    style_button(view_stats_button);

    gtk_container_add(GTK_CONTAINER(main_box), label);
    gtk_container_add(GTK_CONTAINER(card), user_list);
    gtk_container_add(GTK_CONTAINER(buttons_box), connect_button);
    gtk_container_add(GTK_CONTAINER(buttons_box), create_user_button);
    gtk_container_add(GTK_CONTAINER(buttons_box), view_stats_button);
    gtk_container_add(GTK_CONTAINER(card), buttons_box);
    gtk_container_add(GTK_CONTAINER(main_box), card);
    gtk_container_add(GTK_CONTAINER(content_area), main_box);

    g_signal_connect(connect_button, "clicked", G_CALLBACK(on_connect_button_clicked), user_list);
    g_signal_connect(create_user_button, "clicked", G_CALLBACK(show_create_user_menu), NULL);
    g_signal_connect(view_stats_button, "clicked", G_CALLBACK(show_statistics_menu), NULL);

    gtk_widget_show_all(content_area);
}


void show_create_user_menu() {
    clear_window();

    // Conteneur principal avec marges
    GtkWidget *main_box = create_styled_box();

    // Titre
    GtkWidget *title = gtk_label_new("Créer un nouveau compte");
    style_title_label(title);

    // Carte pour le formulaire
    GtkWidget *form_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_style_context_add_class(gtk_widget_get_style_context(form_card), "stats-view");
    gtk_widget_set_margin_top(form_card, 20);

    // Label d'instruction
    GtkWidget *instruction_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(instruction_label), 
        "<span font='14'>Entrez votre nom de joueur :</span>");

    // Zone de saisie
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Votre nom");
    gtk_widget_set_size_request(entry, 250, 40);
    gtk_widget_set_halign(entry, GTK_ALIGN_CENTER);

    // Conteneur pour les boutons
    GtkWidget *buttons_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(buttons_box, GTK_ALIGN_CENTER);
    gtk_widget_set_margin_top(buttons_box, 20);

    // Bouton de création
    GtkWidget *create_button = gtk_button_new_with_label("Créer l'utilisateur");
    style_button(create_button);
    
    // Bouton retour
    GtkWidget *back_button = gtk_button_new_with_label("Retour");
    style_button(back_button);

    // Message d'erreur (invisible par défaut)
    GtkWidget *error_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(error_label), 
        "<span color='#FF0000' font='12'></span>");
    
    // Assemblage des boutons
    gtk_container_add(GTK_CONTAINER(buttons_box), back_button);
    gtk_container_add(GTK_CONTAINER(buttons_box), create_button);

    // Assemblage du formulaire
    gtk_container_add(GTK_CONTAINER(form_card), instruction_label);
    gtk_container_add(GTK_CONTAINER(form_card), entry);
    gtk_container_add(GTK_CONTAINER(form_card), error_label);
    gtk_container_add(GTK_CONTAINER(form_card), buttons_box);

    // Assemblage final
    gtk_container_add(GTK_CONTAINER(main_box), title);
    gtk_container_add(GTK_CONTAINER(main_box), form_card);
    
    gtk_container_add(GTK_CONTAINER(content_area), main_box);

    // Connexion des signaux
    g_signal_connect(create_button, "clicked", G_CALLBACK(on_create_user_button_clicked), entry);
    g_signal_connect(back_button, "clicked", G_CALLBACK(show_login_menu), NULL);
    g_signal_connect(entry, "activate", G_CALLBACK(on_create_user_button_clicked), entry);

    // Mise à jour du CSS pour ajouter des styles spécifiques au formulaire
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "entry { font-size: 14px; padding: 8px 12px; }"
        "entry:focus { border-color: #2196F3; }"
        , -1, NULL);
    
    GtkStyleContext *entry_context = gtk_widget_get_style_context(entry);
    gtk_style_context_add_provider(entry_context,
                                 GTK_STYLE_PROVIDER(provider),
                                 GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    g_object_unref(provider);

    gtk_widget_show_all(content_area);
}



void create_level_stats_card(GtkWidget *container, const char *level_name, RankingEntry *entries, int count) {
    // Création de la carte pour le niveau
    GtkWidget *level_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_style_context_add_class(gtk_widget_get_style_context(level_card), "stats-view");
    gtk_widget_set_margin_start(level_card, 10);
    gtk_widget_set_margin_end(level_card, 10);
    gtk_widget_set_margin_bottom(level_card, 10);

    // Titre du niveau
    char *level_title = g_markup_printf_escaped("<span font='16' weight='bold'>Niveau %s</span>", level_name);
    GtkWidget *level_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(level_label), level_title);
    g_free(level_title);

    // Création de la grille pour les entrées
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 15);
    gtk_widget_set_margin_top(grid, 10);
    gtk_widget_set_margin_bottom(grid, 10);
    gtk_widget_set_margin_start(grid, 15);
    gtk_widget_set_margin_end(grid, 15);

    // En-têtes de colonnes
    GtkWidget *rank_header = gtk_label_new(NULL);
    GtkWidget *name_header = gtk_label_new(NULL);
    GtkWidget *score_header = gtk_label_new(NULL);
    
    gtk_label_set_markup(GTK_LABEL(rank_header), "<span weight='bold'>Rang</span>");
    gtk_label_set_markup(GTK_LABEL(name_header), "<span weight='bold'>Joueur</span>");
    gtk_label_set_markup(GTK_LABEL(score_header), "<span weight='bold'>Score</span>");

    gtk_grid_attach(GTK_GRID(grid), rank_header, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), name_header, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), score_header, 2, 0, 1, 1);

    // Ajout des entrées dans la grille
    for (int i = 0; i < count; i++) {
        // Rang avec médaille pour le top 3
        char *rank_text;
        if (i == 0) rank_text = g_strdup("🥇");
        else if (i == 1) rank_text = g_strdup("🥈");
        else if (i == 2) rank_text = g_strdup("🥉");
        else rank_text = g_strdup_printf("%d", i + 1);

        GtkWidget *rank_label = gtk_label_new(rank_text);
        GtkWidget *name_label = gtk_label_new(entries[i].name);
        GtkWidget *score_label = gtk_label_new(g_strdup_printf("%d", entries[i].score));

        // Alignement
        gtk_widget_set_halign(rank_label, GTK_ALIGN_CENTER);
        gtk_widget_set_halign(name_label, GTK_ALIGN_START);
        gtk_widget_set_halign(score_label, GTK_ALIGN_END);

        // Style pour le top 3
        if (i < 3) {
            char *markup = g_markup_printf_escaped("<span weight='bold'>%s</span>", entries[i].name);
            gtk_label_set_markup(GTK_LABEL(name_label), markup);
            g_free(markup);
            
            markup = g_markup_printf_escaped("<span weight='bold'>%d</span>", entries[i].score);
            gtk_label_set_markup(GTK_LABEL(score_label), markup);
            g_free(markup);
        }

        gtk_grid_attach(GTK_GRID(grid), rank_label, 0, i + 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), name_label, 1, i + 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), score_label, 2, i + 1, 1, 1);
        
        g_free(rank_text);
    }

    gtk_container_add(GTK_CONTAINER(level_card), level_label);
    gtk_container_add(GTK_CONTAINER(level_card), grid);
    gtk_container_add(GTK_CONTAINER(container), level_card);
}

void show_statistics_menu() {
    clear_window();
    
    // Conteneur principal
    GtkWidget *main_box = create_styled_box();
    
    // Titre principal
    GtkWidget *title = gtk_label_new("Tableau des scores");
    style_title_label(title);
    gtk_widget_set_margin_bottom(title, 20);

    // Création du conteneur pour les cartes de niveau
    GtkWidget *levels_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    
    const char *levels[] = {"Facile", "Moyen", "Difficile", "Expert"};

    // Pour chaque niveau
    for (int level = 0; level < 4; level++) {
        // Création du tableau de classement
        RankingEntry *entries = NULL;
        int entryCount = 0;

        // Collecte des scores non nuls
        for (int i = 0; i < userNumber; i++) {
            if (allUsers[i].bestScores[level] > 0) {
                entries = realloc(entries, (entryCount + 1) * sizeof(RankingEntry));
                entries[entryCount].name = allUsers[i].name;
                entries[entryCount].score = allUsers[i].bestScores[level];
                entries[entryCount].rank = entryCount + 1;
                entryCount++;
            }
        }

        // Tri des scores
        for (int i = 0; i < entryCount - 1; i++) {
            for (int j = i + 1; j < entryCount; j++) {
                if (entries[i].score > entries[j].score) {
                    RankingEntry temp = entries[i];
                    entries[i] = entries[j];
                    entries[j] = temp;
                }
            }
        }

        // Création de la carte de statistiques pour ce niveau
        create_level_stats_card(levels_container, levels[level], entries, entryCount);

        // Libération de la mémoire
        free(entries);
    }

    // Ajout d'un scroll window si nécessaire
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll), 
                                 GTK_POLICY_NEVER, 
                                 GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), levels_container);

    // Bouton de retour
    GtkWidget *back_button = gtk_button_new_with_label("Retour au menu principal");
    style_button(back_button);
    gtk_widget_set_margin_top(back_button, 20);

    // Assemblage de l'interface
    gtk_container_add(GTK_CONTAINER(main_box), title);
    gtk_container_add(GTK_CONTAINER(main_box), scroll);
    gtk_container_add(GTK_CONTAINER(main_box), back_button);
    gtk_container_add(GTK_CONTAINER(content_area), main_box);

    g_signal_connect(back_button, "clicked", G_CALLBACK(show_login_menu), NULL);

    gtk_widget_show_all(content_area);
}


void show_difficulty_menu(GtkWidget *widget, gpointer data) {
    clear_window();
    User *selected_user = (User *)data;
    current_user = *selected_user;

    // Création du conteneur principal avec marges
    GtkWidget *main_box = create_styled_box();

    // Titre stylisé
    GtkWidget *title = gtk_label_new("Choisissez votre niveau");
    style_title_label(title);
    
    // Carte contenant les boutons
    GtkWidget *card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_style_context_add_class(gtk_widget_get_style_context(card), "stats-view");
    
    // Conteneur pour les boutons de difficulté
    GtkWidget *buttons_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_halign(buttons_box, GTK_ALIGN_CENTER);

    // Message de bienvenue
    char welcome_text[256];
    snprintf(welcome_text, sizeof(welcome_text), 
             "<span font='14'>Bienvenue %s !</span>", 
             current_user.name);
    GtkWidget *welcome_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(welcome_label), welcome_text);

    // Création des boutons de difficulté avec style
    const char *difficulty_descriptions[] = {
        "Nombres entre 1 et 50 - Essais illimités",
        "Nombres entre 1 et 100 - 10 essais",
        "Nombres entre 1 et 500 - 10 essais",
        "Nombres entre 1 et 1000 - 15 essais"
    };

    for (int i = 0; i < 4; i++) {
        // Création d'une boîte pour chaque niveau
        GtkWidget *level_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
        
        // Bouton de niveau
        GtkWidget *button = gtk_button_new_with_label(gamesLevels[i].name);
        style_button(button);
        
        // Description du niveau
        GtkWidget *desc_label = gtk_label_new(difficulty_descriptions[i]);
        gtk_label_set_markup(GTK_LABEL(desc_label), 
                           g_markup_printf_escaped("<span font='12' color='#666666'>%s</span>", 
                           difficulty_descriptions[i]));

        // Ajout du bouton et de sa description dans la boîte de niveau
        gtk_container_add(GTK_CONTAINER(level_box), button);
        gtk_container_add(GTK_CONTAINER(level_box), desc_label);
        gtk_container_add(GTK_CONTAINER(buttons_box), level_box);

        // Connexion du signal
        g_signal_connect(button, "clicked", G_CALLBACK(start_game), GINT_TO_POINTER(i+1));
    }

    // Bouton de retour
    GtkWidget *back_button = gtk_button_new_with_label("Retour au menu principal");
    gtk_widget_set_margin_top(back_button, 20);
    style_button(back_button);
    
    // Assemblage de l'interface
    gtk_container_add(GTK_CONTAINER(main_box), title);
    gtk_container_add(GTK_CONTAINER(main_box), welcome_label);
    gtk_container_add(GTK_CONTAINER(card), buttons_box);
    gtk_container_add(GTK_CONTAINER(main_box), card);
    gtk_container_add(GTK_CONTAINER(main_box), back_button);
    gtk_container_add(GTK_CONTAINER(content_area), main_box);

    g_signal_connect(back_button, "clicked", G_CALLBACK(show_login_menu), NULL);

    gtk_widget_show_all(content_area);
}

void start_game(GtkWidget *widget, gpointer data) {
    int level = GPOINTER_TO_INT(data);
    clear_window();
    numberToGuess = generate_random_number(0, gamesLevels[level-1].end);
    maxTries = gamesLevels[level-1].tries;
    tries = 0;

    GtkWidget *main_box = create_styled_box();
    GtkWidget *game_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 15);
    gtk_style_context_add_class(gtk_widget_get_style_context(game_card), "stats-view");

    GtkWidget *title = gtk_label_new("Trouvez le nombre mystère");
    style_title_label(title);

    GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(input_box, GTK_ALIGN_CENTER);

    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(entry), 15);
    gtk_widget_set_halign(entry, GTK_ALIGN_CENTER);

    GtkWidget *submit_button = gtk_button_new_with_label("Valider");
    style_button(submit_button);

    GtkWidget *feedback_label = gtk_label_new(NULL);
    gtk_style_context_add_class(gtk_widget_get_style_context(feedback_label), "game-feedback");

    gtk_container_add(GTK_CONTAINER(main_box), title);
    gtk_container_add(GTK_CONTAINER(input_box), entry);
    gtk_container_add(GTK_CONTAINER(input_box), submit_button);
    gtk_container_add(GTK_CONTAINER(game_card), input_box);
    gtk_container_add(GTK_CONTAINER(game_card), feedback_label);
    gtk_container_add(GTK_CONTAINER(main_box), game_card);
    gtk_container_add(GTK_CONTAINER(content_area), main_box);

    GameWidgets *widgets = g_new(GameWidgets, 1);
    widgets->entry = entry;
    widgets->feedback_label = feedback_label;
    widgets->level = level;

    g_signal_connect(submit_button, "clicked", G_CALLBACK(update_game_interface), widgets);
    g_signal_connect(entry, "activate", G_CALLBACK(update_game_interface), widgets);

    gtk_widget_show_all(content_area);
}

void update_game_interface(GtkWidget *widget, gpointer data) {
    GameWidgets *widgets = (GameWidgets *)data;
    const char *input_text = gtk_entry_get_text(GTK_ENTRY(widgets->entry));
    int userInput = atoi(input_text);
    tries++;

    if (userInput < numberToGuess) {
        gtk_label_set_text(GTK_LABEL(widgets->feedback_label), "Plus grand !");
    } else if (userInput > numberToGuess) {
        gtk_label_set_text(GTK_LABEL(widgets->feedback_label), "Plus petit !");
    } else {
        // Jeu gagné
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "Bravo %s ! Trouvé en %d essais.", current_user, tries);
        gtk_label_set_text(GTK_LABEL(widgets->feedback_label), buffer);
        updateScores(&current_user, widgets->level, tries);
        saveScores(&current_user);
        allUsers = loadUsers(&userNumber);

        // Ajouter le bouton de retour au menue principale
        GtkWidget *back_button = gtk_button_new_with_label("Retour au menu principal");
        gtk_container_add(GTK_CONTAINER(content_area), back_button);
        g_signal_connect(back_button, "clicked", G_CALLBACK(show_login_menu), NULL);

        // Ajouter le bouton de recommencement
        GtkWidget *restart = gtk_button_new_with_label("Rejouer");
        gtk_container_add(GTK_CONTAINER(content_area), restart);
        g_signal_connect(restart, "clicked", G_CALLBACK(start_game), GINT_TO_POINTER(widgets->level));

        // Ajouter le bouton de retour au menue de difficulté
        GtkWidget *difficulties = gtk_button_new_with_label("Retour au menu des difficultés");
        gtk_container_add(GTK_CONTAINER(content_area), difficulties);
        g_signal_connect(difficulties, "clicked", G_CALLBACK(show_difficulty_menu), &current_user);

        // Forcer l'affichage des widgets
        gtk_widget_show_all(content_area);
        return;
    }

    if (maxTries > 0 && tries >= maxTries) {
        // Jeu perdu
        char buffer[128];
        snprintf(buffer, sizeof(buffer), "Perdu %s ! Le nombre était %d.", current_user, numberToGuess);
        gtk_label_set_text(GTK_LABEL(widgets->feedback_label), buffer);

        // Ajouter le bouton de retour
        GtkWidget *back_button = gtk_button_new_with_label("Retour au menu principal");
        gtk_container_add(GTK_CONTAINER(content_area), back_button);
        g_signal_connect(back_button, "clicked", G_CALLBACK(show_login_menu), NULL);

        // Forcer l'affichage des widgets
        gtk_widget_show_all(content_area);
        return;
    }

    gtk_entry_set_text(GTK_ENTRY(widgets->entry), ""); // Réinitialiser l'entrée
}


// Main function
int main(int argc, char *argv[]) {
    srand(time(NULL));
    gtk_init(&argc, &argv);
    
    // Charger le CSS
    load_css();
    
    allUsers = loadUsers(&userNumber);
    // if (allUsers == NULL) {
    //     printf("Erreur lors du chargement des utilisateurs.\n");
    //     return 1;
    // }

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Guess the Number");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 600);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    content_area = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), content_area);

    show_login_menu();

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}