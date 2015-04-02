/*
 * Creates the gui window
 * Used tutorials to create the window from: https://developer.gnome.org/
 * Used this tutorial to create the list: http://zetcode.com/tutorials/gtktutorial/gtktreeview/
 */
static GtkWidget* create_popup(void)
{
	/* Declare variables */
	GtkWidget *window;
	GtkWidget *button;
	GtkWidget *box;
	char *label_message = malloc(sizeof(char *));

	/* Create a window with a title and a default size */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (window), "Waiting For Players");
	gtk_window_set_default_size (GTK_WINDOW (window), 250, 50);

	/* Create the box */
	box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	gtk_container_add(GTK_CONTAINER (window), box);

	/* Create the label */
	sprintf(label_message, "%d connected players.\nTo leave teh game click on the button.\n", number_users);
	label_popup = gtk_label_new(label_message);
	gtk_label_set_justify(GTK_LABEL(label_popup), GTK_JUSTIFY_CENTER);
	gtk_box_pack_start(GTK_BOX (box), label_popup, TRUE, TRUE, 5);

	/* Create the button */
	button = gtk_button_new_with_label ("Leave");
	gtk_box_pack_start(GTK_BOX (box), button, FALSE, FALSE, 5);

	/* Show the window */
	gtk_widget_show_all(GTK_WIDGET (box));

	return window;
}