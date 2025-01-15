#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    GtkTextBuffer *buffer;
    char *current_file;
    GtkWidget *window;
    GtkWidget *headerbar;
    GtkWidget *status_label;
    GtkWidget *search_entry; // Champ ajouté
} FileData;


// Fonction pour charger le CSS
static void load_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();

    gtk_css_provider_load_from_path(provider,"../../styles/blocNote.css", NULL);

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(provider);
}

static void update_status_bar(FileData *data) {
    char *status_text;
    if (data->current_file) {
        status_text = g_strdup_printf("Fichier actuel: %s", data->current_file);
    } else {
        status_text = g_strdup("Aucun fichier ouvert");
    }
    gtk_label_set_text(GTK_LABEL(data->status_label), status_text);
    g_free(status_text);
}

void on_open_file(GtkWidget *button, FileData *file_data) {
    GtkWidget *dialog;
    GtkFileChooser *chooser;
    char *filename;

    dialog = gtk_file_chooser_dialog_new(
        "Ouvrir un fichier",
        GTK_WINDOW(file_data->window),
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "Annuler", GTK_RESPONSE_CANCEL,
        "Ouvrir", GTK_RESPONSE_ACCEPT,
        NULL
    );

    // Ajouter des filtres de fichiers
    GtkFileFilter *filter_text = gtk_file_filter_new();
    gtk_file_filter_set_name(filter_text, "Fichiers texte");
    gtk_file_filter_add_mime_type(filter_text, "text/plain");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter_text);

    chooser = GTK_FILE_CHOOSER(dialog);
    gtk_file_chooser_set_current_folder(chooser, "../../data/blocNote");

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        filename = gtk_file_chooser_get_filename(chooser);
        FILE *file = fopen(filename, "r");
        
        if (file) {
            char buffer_line[1024];
            GtkTextIter start, end;
            
            gtk_text_buffer_get_bounds(file_data->buffer, &start, &end);
            gtk_text_buffer_delete(file_data->buffer, &start, &end);

            while (fgets(buffer_line, sizeof(buffer_line), file)) {
                gtk_text_buffer_insert_at_cursor(file_data->buffer, buffer_line, -1);
            }

            fclose(file);
            
            // Mettre à jour le fichier courant
            g_free(file_data->current_file);
            file_data->current_file = g_strdup(filename);
            
            // Mettre à jour le titre de la fenêtre
            char *title = g_strdup_printf("Éditeur - %s", g_path_get_basename(filename));
            gtk_header_bar_set_title(GTK_HEADER_BAR(file_data->headerbar), title);
            g_free(title);
            
            update_status_bar(file_data);
        }
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

void on_save_file(GtkWidget *button, FileData *file_data) {
    GtkWidget *dialog;
    char *filename = file_data->current_file;

    if (!filename) {
        dialog = gtk_file_chooser_dialog_new(
            "Sauvegarder le fichier",
            GTK_WINDOW(file_data->window),
            GTK_FILE_CHOOSER_ACTION_SAVE,
            "Annuler", GTK_RESPONSE_CANCEL,
            "Sauvegarder", GTK_RESPONSE_ACCEPT,
            NULL
        );

        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), "../data/bibliotheque");
        gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
            filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
            g_free(file_data->current_file);
            file_data->current_file = g_strdup(filename);
            
            // Mettre à jour le titre de la fenêtre
            char *title = g_strdup_printf("Éditeur - %s", g_path_get_basename(filename));
            gtk_header_bar_set_title(GTK_HEADER_BAR(file_data->headerbar), title);
            g_free(title);
        }
        
        gtk_widget_destroy(dialog);
    }

    if (filename) {
        FILE *file = fopen(filename, "w");
        if (file) {
            GtkTextIter start, end;
            gchar *content;

            gtk_text_buffer_get_bounds(file_data->buffer, &start, &end);
            content = gtk_text_buffer_get_text(file_data->buffer, &start, &end, FALSE);
            fprintf(file, "%s", content);
            g_free(content);
            fclose(file);
            
            update_status_bar(file_data);
            
            // Afficher un message de confirmation
            GtkWidget *message = gtk_message_dialog_new(
                GTK_WINDOW(file_data->window),
                GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_INFO,
                GTK_BUTTONS_OK,
                "Le fichier a été sauvegardé avec succès!"
            );
            gtk_dialog_run(GTK_DIALOG(message));
            gtk_widget_destroy(message);
        }
    }
}

void on_new_file(GtkWidget *button, FileData *file_data) {
    GtkTextIter start, end;
    
    gtk_text_buffer_get_bounds(file_data->buffer, &start, &end);
    gtk_text_buffer_delete(file_data->buffer, &start, &end);
    
    g_free(file_data->current_file);
    file_data->current_file = NULL;
    
    gtk_header_bar_set_title(GTK_HEADER_BAR(file_data->headerbar), "Éditeur - Nouveau document");
    update_status_bar(file_data);
}

void on_search_word(GtkWidget *button, gpointer user_data) {
    FileData *file_data = (FileData *)user_data;
    GtkTextIter start, match_start, match_end;
    const gchar *search_word;
    gchar *content;
    int count = 0;

    GtkWidget *search_entry = g_object_get_data(G_OBJECT(button), "search_entry");
    search_word = gtk_entry_get_text(GTK_ENTRY(search_entry));
    if (!search_word || g_strcmp0(search_word, "") == 0) {
        GtkWidget *message = gtk_message_dialog_new(
            GTK_WINDOW(file_data->window),
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_WARNING,
            GTK_BUTTONS_OK,
            "Veuillez entrer un mot à rechercher."
        );
        gtk_dialog_run(GTK_DIALOG(message));
        gtk_widget_destroy(message);
        return;
    }

    gtk_text_buffer_get_start_iter(file_data->buffer, &start);
    while (gtk_text_iter_forward_search(
        &start, search_word, GTK_TEXT_SEARCH_TEXT_ONLY, &match_start, &match_end, NULL)) {
        count++;
        start = match_end;
    }

    char *message_text = g_strdup_printf("Le mot '%s' apparaît %d fois dans le texte.", search_word, count);
    GtkWidget *message = gtk_message_dialog_new(
        GTK_WINDOW(file_data->window),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO,
        GTK_BUTTONS_OK,
        "%s", message_text
    );
    g_free(message_text);
    gtk_dialog_run(GTK_DIALOG(message));
    gtk_widget_destroy(message);
}

int main(int argc, char *argv[]) {
    GtkWidget *window, *scrolled_window, *text_view;
    GtkWidget *headerbar, *new_button, *open_button, *save_button;
    GtkWidget *status_bar, *status_label;
    GtkTextBuffer *buffer;
    FileData *file_data;

    gtk_init(&argc, &argv);
    load_css();

    // Création de la fenêtre principale
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    // Configuration de la headerbar
    headerbar = gtk_header_bar_new();
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(headerbar), TRUE);
    gtk_header_bar_set_title(GTK_HEADER_BAR(headerbar), "Éditeur - Nouveau document");
    gtk_window_set_titlebar(GTK_WINDOW(window), headerbar);
    GtkWidget *search_entry = gtk_search_entry_new();
    gtk_widget_set_tooltip_text(search_entry, "Rechercher un mot");
    gtk_header_bar_pack_end(GTK_HEADER_BAR(headerbar), search_entry);

    GtkWidget *search_button = gtk_button_new_from_icon_name("edit-find", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_tooltip_text(search_button, "Rechercher");
    gtk_header_bar_pack_end(GTK_HEADER_BAR(headerbar), search_button);
    g_object_set_data(G_OBJECT(search_button), "search_entry", search_entry);

    // Création des boutons avec icônes
    new_button = gtk_button_new_from_icon_name("document-new", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_tooltip_text(new_button, "Nouveau fichier");
    gtk_header_bar_pack_start(GTK_HEADER_BAR(headerbar), new_button);

    open_button = gtk_button_new_from_icon_name("document-open", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_tooltip_text(open_button, "Ouvrir un fichier");
    gtk_header_bar_pack_start(GTK_HEADER_BAR(headerbar), open_button);

    save_button = gtk_button_new_from_icon_name("document-save", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_tooltip_text(save_button, "Sauvegarder");
    gtk_header_bar_pack_start(GTK_HEADER_BAR(headerbar), save_button);

    // Création de la zone de texte avec scroll
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                 GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
    text_view = gtk_text_view_new();
    gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);

    // Création de la barre de status
    status_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_style_context_add_class(gtk_widget_get_style_context(status_bar), "status-bar");
    status_label = gtk_label_new("Aucun fichier ouvert");
    gtk_box_pack_start(GTK_BOX(status_bar), status_label, FALSE, FALSE, 5);

    // Configuration du conteneur principal
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), main_box);
    gtk_box_pack_start(GTK_BOX(main_box), scrolled_window, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), status_bar, FALSE, FALSE, 0);

    // Initialisation des données
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    file_data = g_new0(FileData, 1);
    file_data->buffer = buffer;
    file_data->current_file = NULL;
    file_data->window = window;
    file_data->headerbar = headerbar;
    file_data->status_label = status_label;
    file_data->search_entry = search_entry;

    // Connexion des signaux
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(new_button, "clicked", G_CALLBACK(on_new_file), file_data);
    g_signal_connect(open_button, "clicked", G_CALLBACK(on_open_file), file_data);
    g_signal_connect(save_button, "clicked", G_CALLBACK(on_save_file), file_data);
    g_signal_connect(search_button, "clicked", G_CALLBACK(on_search_word), file_data);

    gtk_widget_show_all(window);
    gtk_main();

    // Nettoyage
    g_free(file_data->current_file);
    g_free(file_data);

    return 0;
}