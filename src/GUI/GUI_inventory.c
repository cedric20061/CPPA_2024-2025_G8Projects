#include <gtk/gtk.h>
#include "inventory.h"
#include <stdlib.h>

typedef struct {
    GtkWidget *window;
    Product **inventory;
    GtkWidget *product_list;
    Product *product_to_edit;
    int *count;
} CallbackData;

typedef struct {
    CallbackData *data;
    GtkWidget *name_entry;
    GtkWidget *category_entry;
    Product *product_to_edit;
    GtkWidget *price_entry;
    GtkWidget *stock_entry;
} CallbackProductUnion;

typedef struct {
    CallbackData *data;
    GtkWidget *search_entry;
    int type;
    GtkWidget *product_list;
} SearchCallBack;

void create_main_window(GtkWidget *window, Product **inventory, int *count);

void cleanWindow(GtkWidget *window){
    // Nettoyage de la fenêtre existante
    GList *children, *iter;
    children = gtk_container_get_children(GTK_CONTAINER(window));
    for (iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);

}

Product create_product_from_entries(GtkEntry *name_entry, GtkEntry *category_entry, GtkEntry *price_entry, GtkEntry *stock_entry) {
    Product new_product;

    const gchar *name = gtk_entry_get_text(name_entry);
    const gchar *category = gtk_entry_get_text(category_entry);
    const gchar *price_text = gtk_entry_get_text(price_entry);
    const gchar *stock_text = gtk_entry_get_text(stock_entry);

    strncpy(new_product.name, name, MAX_CHAR_LENGTH);
    strncpy(new_product.category, category, MAX_CHAR_LENGTH);

    int price = atoi(price_text);
    new_product.price = abs(price);

    int stock = atoi(stock_text);
    new_product.stock = abs(stock);

    return new_product;
}


void update_product_table(GtkWidget *product_list, Product *filteredProducts, int foundCount) {
    // Récupération du modèle existant
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(product_list));
    GtkListStore *store = GTK_LIST_STORE(model);

    // Effacer le contenu existant du GtkListStore
    gtk_list_store_clear(store);

    // Ajouter les nouveaux produits au GtkListStore
    GtkTreeIter iter;
    for (int i = 0; i < foundCount; i++) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
            0, filteredProducts[i].name,       // Nom
            1, filteredProducts[i].category,   // Catégorie
            2, filteredProducts[i].price,      // Prix
            3, filteredProducts[i].stock,      // Stock
            4, &filteredProducts[i],           // Référence produit
            5, &filteredProducts[i],           // Pointeur pour actions spécifiques
            -1);
    }
}

void on_save_clicked(GtkWidget *button, gpointer data) {
    CallbackProductUnion *union_data = (CallbackProductUnion *)data;

    // Récupérer les champs d'entrée depuis l'union
    GtkEntry *name_entry = GTK_ENTRY(union_data->name_entry);
    GtkEntry *category_entry = GTK_ENTRY(union_data->category_entry);
    GtkEntry *price_entry = GTK_ENTRY(union_data->price_entry);
    GtkEntry *stock_entry = GTK_ENTRY(union_data->stock_entry);
    GtkWidget *window = GTK_WIDGET(union_data->data->window);

    // Créer un produit à partir des champs d'entrée
    Product updated_product = create_product_from_entries(name_entry, category_entry, price_entry, stock_entry);

    // Vérifier si un produit est en cours de modification
    if (union_data->product_to_edit != NULL) {
        // Mise à jour du produit existant
        int result = updateProduct(union_data->product_to_edit->id, updated_product, union_data->data->inventory, union_data->data->count);
        if (result == 1) {
            g_print("Produit mis à jour : %s, %s, %d, %d\n", updated_product.name, updated_product.category, updated_product.price, updated_product.stock);
        } else {
            g_print("Erreur : mise à jour du produit échouée.\n");
        }
    } else {
        // Ajout d'un nouveau produit
        addProduct(updated_product, union_data->data->inventory, union_data->data->count);
        g_print("Produit ajouté : %s, %s, %d, %d\n", updated_product.name, updated_product.category, updated_product.price, updated_product.stock);
    }

    // Nettoyer et libérer les ressources
    free(union_data);
    cleanWindow(window);
    create_main_window(window, union_data->data->inventory, union_data->data->count);
}


void on_cancel_clicked(GtkWidget *button, gpointer data) {
    CallbackProductUnion *union_data = (CallbackProductUnion *)data;
    GtkWidget *window = GTK_WIDGET(union_data->data->window);

    cleanWindow(window);
    create_main_window(window, union_data->data->inventory, union_data->data->count);
}

GtkWidget* create_entry_with_label(const char *label_text, const char *default_text, GtkWidget *container) {
    GtkWidget *label = gtk_label_new(label_text);
    GtkWidget *entry = gtk_entry_new();

    if (default_text != NULL) {
        gtk_entry_set_text(GTK_ENTRY(entry), default_text);
    }

    gtk_box_pack_start(GTK_BOX(container), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(container), entry, FALSE, FALSE, 0);

    return entry;
}

void load_shop_css() {
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(provider,"../../styles/inventory.css", NULL);

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    g_object_unref(provider);
}
void on_addOrEdit_product(GtkWidget *button, gpointer data) {
    // Récupération des données nécessaires
    CallbackData *appData = (CallbackData *)data;
    GtkWidget *window = GTK_WIDGET(appData->window);

    // Vérifiez si un produit à modifier est passé dans `gpointer`
    Product *product_to_edit = appData->product_to_edit;

    cleanWindow(appData->window);

    // Création du formulaire
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_style_context_add_class(gtk_widget_get_style_context(vbox), "form-container");
    gtk_container_add(GTK_CONTAINER(window), vbox);
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *header = gtk_label_new("My Shop");
    gtk_style_context_add_class(gtk_widget_get_style_context(header), "header");
    gtk_box_pack_start(GTK_BOX(header_box), header, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), header_box, FALSE, FALSE, 0);

    // Champs d'entrée
    GtkWidget *name_entry = create_entry_with_label(
        "Nom:", 
        product_to_edit ? product_to_edit->name : NULL, 
        vbox
    );
    gtk_style_context_add_class(gtk_widget_get_style_context(name_entry), "input");

    GtkWidget *category_entry = create_entry_with_label(
        "Catégorie:", 
        product_to_edit ? product_to_edit->category : NULL, 
        vbox
    );
    gtk_style_context_add_class(gtk_widget_get_style_context(category_entry), "input");

    GtkWidget *price_entry = create_entry_with_label(
        "Prix:", 
        product_to_edit ? g_strdup_printf("%d", product_to_edit->price) : NULL, 
        vbox
    );
    gtk_style_context_add_class(gtk_widget_get_style_context(price_entry), "input");

    GtkWidget *stock_entry = create_entry_with_label(
        "Stock:", 
        product_to_edit ? g_strdup_printf("%d", product_to_edit->stock) : NULL, 
        vbox
    );
    gtk_style_context_add_class(gtk_widget_get_style_context(stock_entry), "input");

    // Boutons
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_style_context_add_class(gtk_widget_get_style_context(button_box), "button-box");
    GtkWidget *save_button = gtk_button_new_with_label(product_to_edit ? "Modifier" : "Enregistrer");
    gtk_style_context_add_class(gtk_widget_get_style_context(save_button), "save-button");
    GtkWidget *cancel_button = gtk_button_new_with_label("Annuler");
    gtk_style_context_add_class(gtk_widget_get_style_context(cancel_button), "cancel-button");
    gtk_box_pack_start(GTK_BOX(button_box), save_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), cancel_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), button_box, FALSE, FALSE, 0);

    gtk_widget_show_all(window);

    // Création d'une union pour contenir toutes les données nécessaires
    CallbackProductUnion *union_data = malloc(sizeof(CallbackProductUnion));
    union_data->data = appData;
    union_data->name_entry = name_entry;
    union_data->category_entry = category_entry;
    union_data->price_entry = price_entry;
    union_data->stock_entry = stock_entry;
    union_data->product_to_edit = product_to_edit;

    // Connecter les signaux aux callbacks appropriés
    g_signal_connect(save_button, "clicked", G_CALLBACK(on_save_clicked), union_data);
    g_signal_connect(cancel_button, "clicked", G_CALLBACK(on_cancel_clicked), union_data);
}

void add_column_to_tree_view(GtkTreeView *tree_view, const char *title, int col_id) {
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *col = gtk_tree_view_column_new_with_attributes(title, renderer, "text", col_id, NULL);
    gtk_tree_view_append_column(tree_view, col);
}

void on_modify_product(Product *product, Product **inventory, int *count, GtkWidget *window) {
    g_print("Modifier produit: %s\n", product->name);
    CallbackData *appData = malloc(sizeof(CallbackData));
    appData->product_to_edit = product;
    appData->count = count;
    appData->inventory = inventory;
    appData->window = window;
    // Ouvrir le formulaire
    on_addOrEdit_product(NULL, appData);
}

void on_delete_product(Product *product, Product **inventory, int *count, GtkWidget *product_list) {
    // Créer une fenêtre de confirmation
    GtkWidget *dialog = gtk_message_dialog_new(
                            NULL, 
                            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                            GTK_MESSAGE_QUESTION, 
                            GTK_BUTTONS_YES_NO,
                            "Êtes-vous sûr de vouloir supprimer le produit %s ?",
                            product->name
                        );
    
    // Exécuter la boîte de dialogue et récupérer la réponse de l'utilisateur
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    
    if (response == GTK_RESPONSE_YES) {
        deleteProduct(product->id, inventory, count);
        g_print("Produit supprimé : %s\n", product->name);
        update_product_table(product_list, *inventory, *count);
        // Supprimer le produit du store ou de la base de données ici
    } else {
        g_print("Suppression annulée\n");
    }

    // Fermer et détruire la boîte de dialogue après la réponse de l'utilisateur
    gtk_widget_destroy(dialog);
}

void on_button_clicked(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    CallbackData *appData = (CallbackData *)user_data;
    GtkTreeView *tree_view = GTK_TREE_VIEW(widget);
    GtkTreeModel *model = gtk_tree_view_get_model(tree_view);
    GtkTreePath *path;
    GtkTreeIter iter;
    GtkTreeViewColumn *column;  // Déclaration correcte pour 'column'
    gpointer data;

    // Récupérer la position du clic et déterminer si elle correspond à une cellule
    if (gtk_tree_view_get_path_at_pos(tree_view, event->x, event->y, &path, &column, NULL, NULL)) {
        // Récupérer le titre de la colonne sur laquelle l'utilisateur a cliqué
        const gchar *column_title = gtk_tree_view_column_get_title(column);
        
        // Vérifier si le titre de la colonne est "Modifier"
        if (g_strcmp0(column_title, "Modifier") == 0) {
            // Récupérer l'élément de données (produit) lié à la ligne cliquée
            if (gtk_tree_model_get_iter(model, &iter, path)) {
                gtk_tree_model_get(model, &iter, 4, &data, -1); // obtenir le produit
                Product *product = (Product *)data;
                on_modify_product(product, appData->inventory, appData->count, appData->window); // Appel de la fonction de modification du produit
            }
        }
        // Vérifier si le titre de la colonne est "Supprimer"
        else if (g_strcmp0(column_title, "Supprimer") == 0) {
            // Récupérer l'élément de données (produit) lié à la ligne cliquée
            if (gtk_tree_model_get_iter(model, &iter, path)) {
                gtk_tree_model_get(model, &iter, 5, &data, -1); // obtenir le produit
                Product *product = (Product *)data;
                on_delete_product(product, appData->inventory, appData->count, appData->product_list); // Appel de la fonction de suppression du produit
            }
        }

        // Libérer la mémoire allouée pour le GtkTreePath
        gtk_tree_path_free(path);
    }
}


GtkWidget *create_product_table(Product **inventory, int *count, GtkWidget *window) {
    GtkWidget *product_list = gtk_tree_view_new();
    GtkListStore *store = gtk_list_store_new(6, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT, G_TYPE_POINTER, G_TYPE_POINTER);
    GtkTreeIter iter;

    // Ajouter les données au GtkListStore
    for (int i = 0; i < *count; i++) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
            0, (*inventory)[i].name,       // Nom
            1, (*inventory)[i].category,   // Catégorie
            2, (*inventory)[i].price,      // Prix
            3, (*inventory)[i].stock,      // Stock
            4, &(*inventory)[i],           // Référence produit
            5, &(*inventory)[i],
            -1);
    }

    // Ajouter les colonnes au GtkTreeView
    add_column_to_tree_view(GTK_TREE_VIEW(product_list), "Nom", 0);
    add_column_to_tree_view(GTK_TREE_VIEW(product_list), "Catégorie", 1);
    add_column_to_tree_view(GTK_TREE_VIEW(product_list), "Prix", 2);
    add_column_to_tree_view(GTK_TREE_VIEW(product_list), "Stock", 3);

    // Ajouter la colonne "Modifier"
    GtkTreeViewColumn *modify_column = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(modify_column, "Modifier");
    // Ajouter la colonne "Supprimer"
    GtkTreeViewColumn *delete_column = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(delete_column, "Supprimer");

    // Cellule pour "Modifier" (text cell renderer)
    GtkCellRenderer *modify_renderer = gtk_cell_renderer_text_new();
    g_object_set(modify_renderer, "text", "Modifier", "foreground", "blue", NULL);
    // Cellule pour "Supprimer" (text cell renderer)
    GtkCellRenderer *delete_renderer = gtk_cell_renderer_text_new();
    g_object_set(delete_renderer, "text", "Supprimer", "foreground", "red", NULL);

    // Allocation de mémoire pour userData et initialisation
    CallbackData *userData = g_new(CallbackData, 1);
    userData->inventory = inventory;
    userData->count = count;
    userData->product_list = product_list;
    userData->window = window;
    // Connecter le signal "button-press-event" pour "Modifier"
    // Connecter le signal "button-press-event" pour les colonnes Modifier et Supprimer
    g_signal_connect(product_list, "button-press-event", G_CALLBACK(on_button_clicked), userData);

    gtk_tree_view_column_pack_start(modify_column, modify_renderer, FALSE);
    gtk_tree_view_column_pack_start(delete_column, delete_renderer, FALSE);

    // Pack les colonnes "Modifier" et "Supprimer" dans le tree view
    gtk_tree_view_append_column(GTK_TREE_VIEW(product_list), modify_column);
    gtk_tree_view_append_column(GTK_TREE_VIEW(product_list), delete_column);

    gtk_tree_view_set_model(GTK_TREE_VIEW(product_list), GTK_TREE_MODEL(store));

    return product_list;
}

void search_product(GtkWidget *button, gpointer data) {
    // Récupération des données nécessaires
    SearchCallBack *appData = (SearchCallBack *)data;
    const gchar *search = gtk_entry_get_text(GTK_ENTRY(appData->search_entry));

    if (search == NULL || strlen(search) == 0) {
        update_product_table(appData->product_list, *(appData->data->inventory), *(appData->data->count));
        return;
    }

    // Recherche des produits
    int foundCount = 0;
    Product *filteredProducts = searchProducts(
        *(appData->data->inventory),
        *(appData->data->count),
        appData->type,
        search,
        &foundCount
    );

    if (foundCount == 0) {
        g_print("Aucun produit trouvé pour la recherche : %s\n", search);
        gtk_list_store_clear(GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(appData->product_list))));
    } else {
        // Mise à jour du tableau avec les produits trouvés
        update_product_table(appData->product_list, filteredProducts, foundCount);
    }

    // Libération de la mémoire allouée par searchProducts
    free(filteredProducts);
}


void create_main_window(GtkWidget *window, Product **inventory, int *count) {
    // Chargement des styles CSS
    load_shop_css();

    // Conteneur principal avec marges
    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_start(main_box, 20);
    gtk_widget_set_margin_end(main_box, 20);
    gtk_widget_set_margin_top(main_box, 20);
    gtk_widget_set_margin_bottom(main_box, 20);
    gtk_container_add(GTK_CONTAINER(window), main_box);

    // En-tête stylisé
    GtkWidget *header_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *header = gtk_label_new("My Shop");
    gtk_style_context_add_class(gtk_widget_get_style_context(header), "header");
    gtk_box_pack_start(GTK_BOX(header_box), header, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), header_box, FALSE, FALSE, 0);

    // Zone de recherche stylisée
    GtkWidget *search_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_style_context_add_class(gtk_widget_get_style_context(search_card), "search-bar");

    // Barre de recherche avec icône
    GtkWidget *search_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *search_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Rechercher un produit...");
    gtk_style_context_add_class(gtk_widget_get_style_context(search_entry), "search-entry");

    // Boutons de recherche
    GtkWidget *buttons_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget *searchByNameButton = gtk_button_new_with_label("Rechercher par nom");
    GtkWidget *searchByCategoryButton = gtk_button_new_with_label("Rechercher par catégorie");

    gtk_box_pack_start(GTK_BOX(search_box), search_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(buttons_box), searchByNameButton, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(buttons_box), searchByCategoryButton, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(search_card), search_box);
    gtk_container_add(GTK_CONTAINER(search_card), buttons_box);
    gtk_box_pack_start(GTK_BOX(main_box), search_card, FALSE, FALSE, 0);

    // Zone des produits avec scroll
    GtkWidget *scroll_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_window),
                                 GTK_POLICY_AUTOMATIC,
                                 GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scroll_window, -1, 400);
    
    // Tableau des produits
    GtkWidget *product_list = create_product_table(inventory, count, window);
    gtk_style_context_add_class(gtk_widget_get_style_context(product_list), "product-list");
    
    gtk_container_add(GTK_CONTAINER(scroll_window), product_list);
    gtk_box_pack_start(GTK_BOX(main_box), scroll_window, TRUE, TRUE, 10);

    // Barre d'actions en bas
    GtkWidget *action_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_top(action_bar, 10);
    
    // Bouton d'ajout stylisé
    GtkWidget *add_button = gtk_button_new_with_label("+ Ajouter un produit");
    gtk_style_context_add_class(gtk_widget_get_style_context(add_button), "add-button");
    gtk_widget_set_halign(add_button, GTK_ALIGN_END);
    gtk_box_pack_end(GTK_BOX(action_bar), add_button, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(main_box), action_bar, FALSE, FALSE, 0);

    // Configuration des données pour la recherche
    CallbackData *appData = g_new(CallbackData, 1);
    appData->inventory = inventory;
    appData->count = count;
    appData->window = window;

    SearchCallBack *search_data1 = g_new(SearchCallBack, 1);
    SearchCallBack *search_data2 = g_new(SearchCallBack, 1);
    
    search_data1->search_entry = search_entry;
    search_data1->product_list = product_list;
    search_data1->type = 1;
    search_data1->data = appData;

    search_data2->search_entry = search_entry;
    search_data2->product_list = product_list;
    search_data2->type = 2;
    search_data2->data = appData;

    // Connexion des signaux
    g_signal_connect(searchByNameButton, "clicked", G_CALLBACK(search_product), search_data1);
    g_signal_connect(searchByCategoryButton, "clicked", G_CALLBACK(search_product), search_data2);
    g_signal_connect(search_entry, "activate", G_CALLBACK(search_product), search_data1);

    // Configuration du bouton d'ajout
    CallbackData *data = g_new(CallbackData, 1);
    data->window = window;
    data->inventory = inventory;
    data->count = count;
    data->product_to_edit = NULL;
    g_signal_connect(add_button, "clicked", G_CALLBACK(on_addOrEdit_product), data);

    gtk_widget_show_all(window);
}

// Fonction principale
int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    Product *inventory = malloc(sizeof(Product));
    int count = 0;

    if (load_data(INVENTORY_FILE, &inventory, &count) == -1) {
        free(inventory);
        return 1;
    }
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Gestionnaire d'Inventaire");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    create_main_window(window, &inventory, &count);

    gtk_main();

    return 0;
}
