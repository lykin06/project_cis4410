#include <gtk/gtk.h>
#include <webkit/webkit.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

#include "values.h"
#include "client.h"
#include "parsing.h"
#include "cards.h"

Address addr;
socklen_t len;
int cs;

// Data needed for the threads
Data data_thread;

// Mutex
pthread_mutex_t mutex;

// Name of the client
char *n;

// Flag to stop the threads
int loop;

// Stores the name of the selected card
char *selected_card;

// List of connected users
GtkWidget *list;
char *users;

// Message wiew
GtkWidget *view;

// Number of connected users
int number_users;

// Number of cards
int cards;

// Player number
int player_number;

// Flag indicating the turn
int play;

// Labels to print the number of users
GtkWidget *label_users;
GtkWidget *label_player_one;
GtkWidget *label_player_two;
GtkWidget *label_player_three;

// Pointer to the next char to read
int *next_char;

/* Enum for the list of cards */
enum
{
	LIST_ITEM = 0,
	N_COLUMNS
};

// State of the game
int state;

/* 
 * Displays a plain text message into the text view
 */
static void display_message(char *message, GtkWidget *view, char *tag) {
    GtkTextBuffer *buffer;
    GtkTextIter iter;

   	printf("display_message \"%s\"\n", message);

    // Gets the buffer and the iter from the view
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);

    // Insert the message at the beginning of the text field
   	gtk_text_buffer_insert_with_tags_by_name(buffer, &iter, message, -1, "lmarg", tag, NULL);

   	printf("end display_message\n");
}

/*
 * Sends the card to play to the user
 */
static void button_clicked(GtkButton *button, gpointer user_data)
{
	int suit, set;
	int *nc;
	char buf[BUFSIZE];

	if(play == WAIT) {
		sprintf(buf, "It is not your turn, please wait.\n");
		printf("%s", buf);
		display_message(buf, view, "red_fg");
	} else {
		if(state == EXCHANGE) {
			if(strcmp(selected_card, "No cards") != 0) {
				nc = malloc(sizeof(int));
				nc[0] = 0;
				parse_name(selected_card, buf, nc);
				suit = suit_int(buf);
				parse_name(selected_card, buf, nc);
				set = set_int(buf);
				sprintf(buf, "%d %d %d %d ", GAME, player_number, suit, set);
				send_message(buf);
				sprintf(selected_card, "No cards");
			} else {
				display_message("No card selected\n", view, "red_fg");
			}
		}
	}
}

/* Called when the window is closed */
void destroy(GtkWidget *widget, gpointer data)
{
	gtk_main_quit ();
}

/*
 * Initializes the list of users
 * From: http://zetcode.com/tutorials/gtktutorial/gtktreeview/
 */
static void init_list(GtkWidget *list)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkListStore *store;

	/* Create a tree with one column and append it into a tree */
	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("List Items", renderer, "text", LIST_ITEM, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

	/* Create a gtk list store model and set it to the tree */
	store = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(list), GTK_TREE_MODEL(store));

	/* The model will be destroyed automatically with the view */
	g_object_unref(store);
}

/*
 * Adds an item to the list
 * From: http://zetcode.com/tutorials/gtktutorial/gtktreeview/
 */
static void add_to_list(const gchar *str)
{
	GtkListStore *store;
	GtkTreeIter iter;

	/* Get the tree from the view */
	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(list)));

	/* Append the new user (str) to the tree */
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, LIST_ITEM, str, -1);
}

/*
 * Changes the selected card
 * From: http://zetcode.com/tutorials/gtktutorial/gtktreeview/
 */
static void on_changed(GtkWidget *widget, gpointer label) 
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	char text[BUFSIZE];

	if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(widget), &model, &iter)) {
		gtk_tree_model_get(model, &iter, LIST_ITEM, &selected_card,  -1);
		gtk_label_set_text(GTK_LABEL(label), selected_card);
		sprintf(text, "New card selected: %s\n", selected_card);
		printf("%s", text);
		display_message(text, view, "red_fg");
	}
}

/*
 * Sends the message to the server
 */
void send_message(char *message) {
	printf("Sending: %s\n", message);
	if (sendto(cs, message, strlen(message), 0, (struct sockaddr *)&addr, len) < 0) {
		perror("ERROR - sendto failed");
		exit(-1);
	}
}

/*
 * Hooks the entry with the view
 * Used tutorial to create this from: https://developer.gnome.org/
 */
static void entry_activate_cb (GtkEntry *entry, GtkWidget *view)
{
    char text[BUFSIZE];
    char message[BUFSIZE];
    
    sprintf(text, "You: %s\n", gtk_entry_get_text(entry));
    sprintf(message, "%d %s %s", BROADCAST, n, gtk_entry_get_text(entry));
	send_message(message);
	display_message(text, view, "blue_fg");
    gtk_entry_set_text (entry, "");
}

/*
 * Creates the gui window
 * Used tutorials to create the window from: https://developer.gnome.org/
 * Used this tutorial to create the list: http://zetcode.com/tutorials/gtktutorial/gtktreeview/
 */
static GtkWidget* create_window(void)
{
	/* Declare variables */
	GtkWidget *window;
	GtkWidget *scroll;
	GtkWidget *entry;
	GtkWidget *label_list;
	GtkWidget *button;
	GtkWidget *box_window;
	GtkWidget *box_game;
	GtkWidget *box_list;
	GtkWidget *box_players;
	GtkWidget *box_message;
	
	GtkTreeSelection *selection;
	GtkTextIter iter;
	GtkTextBuffer *buffer;
	buffer = gtk_text_buffer_new(NULL);

	/* Create a window with a title, and a default size */
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW (window), "Hearts");
	gtk_window_set_default_size(GTK_WINDOW (window), 660, 600);
	g_signal_connect(window, "delete-event", G_CALLBACK (gtk_main_quit), NULL);
	
	/* Create the boxes */
	box_window = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	box_game = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	box_list = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	box_players = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
	box_message = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	
	/* Create the list */
	list = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(list), FALSE);
	
	/*Create a label*/
	label_list = gtk_label_new("No cards");
	label_users = gtk_label_new(n);
	label_player_one = gtk_label_new("Player");
	label_player_two = gtk_label_new("Player");
	label_player_three = gtk_label_new("Player");
	gtk_label_set_justify(GTK_LABEL(label_list), GTK_JUSTIFY_CENTER);
	gtk_label_set_justify(GTK_LABEL(label_users), GTK_JUSTIFY_CENTER);
	gtk_label_set_justify(GTK_LABEL(label_player_one), GTK_JUSTIFY_CENTER);
	gtk_label_set_justify(GTK_LABEL(label_player_two), GTK_JUSTIFY_CENTER);
	gtk_label_set_justify(GTK_LABEL(label_player_three), GTK_JUSTIFY_CENTER);
	
	/* Create a new entry */
	entry = gtk_entry_new();

	/* Create a scrolled window */
	scroll = gtk_scrolled_window_new (NULL, NULL);
	g_object_set (scroll, "shadow-type", GTK_SHADOW_IN, NULL);

	/* Create the view to print the messages */
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	gtk_container_add(GTK_CONTAINER (scroll), view);
	gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);

	/* Create tags to apply polices to the texts */
	gtk_text_buffer_create_tag(buffer, "lmarg", "left_margin", 5, NULL);
	gtk_text_buffer_create_tag(buffer, "bold", "weight", PANGO_WEIGHT_BOLD, NULL);
	gtk_text_buffer_create_tag(buffer, "normal", "weight", PANGO_WEIGHT_NORMAL, NULL);
	gtk_text_buffer_create_tag(buffer, "blue_fg", "foreground", "blue", NULL);
	gtk_text_buffer_create_tag(buffer, "red_fg", "foreground", "red", NULL);

	/* Add a new message */
	gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, "You are connected\n", -1, "bold", "lmarg",  NULL);

	/* Create a button */
	button = gtk_button_new_with_label ("Play a card");

	/* Add the widgets to the boxes */
	gtk_box_pack_start(GTK_BOX (box_players), label_player_one, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX (box_players), label_player_two, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX (box_players), label_player_three, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX (box_game), box_players, FALSE, FALSE, 5);	
	gtk_box_pack_start(GTK_BOX (box_message), scroll, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX (box_message), entry, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX (box_game), box_message, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX (box_window), box_game, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX (box_list), label_users, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX (box_list), list, TRUE, TRUE, 5);
	gtk_box_pack_start(GTK_BOX (box_list), label_list, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX (box_list), button, FALSE, FALSE, 5);
	gtk_box_pack_start(GTK_BOX (box_window), box_list, FALSE, FALSE, 5);
	
	/* Add the grid to the window */
	gtk_container_add(GTK_CONTAINER (window), box_window);
	
	/* Initialize the list */
	init_list(list);
	add_to_list("No cards");

	/* Initialize the players */
	set_players();
	
	/* Create the tree */
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(list));
	
	/* Hook the selection of en item of the list */
	g_signal_connect(selection, "changed", G_CALLBACK(on_changed), label_list);

	/* Hook the entry to the view */
	g_signal_connect (entry, "activate", G_CALLBACK (entry_activate_cb), view);

	/*Connecting the clicked signal to the callback function*/
	g_signal_connect (GTK_BUTTON (button), "clicked", G_CALLBACK (button_clicked), G_OBJECT (window));

	/* Show the window */
	gtk_widget_show_all(GTK_WIDGET (box_window));
	
	return window;
}

/*
 * Receives the message from the server
 */
void *receive_thread(void *data_thread) {
	int recvlen, action;
	
	// Pointer to functions
	funcptr *functions = malloc(sizeof(funcptr) * ACTIONS);
	functions[0] = &play_card;
	functions[1] = &broadcast;
	functions[2] = &add_user;
	functions[3] = &remove_user;

	// Buffer to receive messages
	char message[BUFSIZE];

	// Set the data values
	Data *data = (Data *) data_thread;
	int cs = data->client_socket;
	Address addr = data->address;
	socklen_t len = data->addrlen;
	char *n = data->name;
	
	while(loop == 0) {
		// Empty buffer
		message[0] = '\0';
		recvlen = recvfrom(cs, message, BUFSIZE, 0, (struct sockaddr *)&addr, &len);

		if (recvlen > 0) {
			// Add End caracter to the message
			message[recvlen] = '\0';
			next_char[0] = 0;
			printf("Received: \"%s\"\n", message);
			if(strcmp(message, "You are disconnected") != 0) {
				// Parse the action wanted by the client
				action = parse_int(message, next_char);
				printf("Will perform action %d, next_char = %d\n", action, *next_char);
			
				// Check if the action is correct
				if((action >= ACTIONS) || (action < 0)) {
					printf("Client: unknown request\n");
				} else {
					// Process the request
					functions[action](message);
				}
			}
		}
	}
	
	free(functions);
	pthread_exit((void *) 0);
}

/*
 * Adds the connected players
 */
void set_players() {
	int i;
	
	// Set all the values
	next_char[0] = 0;
	player_number = parse_int(users, next_char);

	// Set the labels
	for(i = 0	; i < player_number; ++i) {
		add_user(users);
	}
}

/*
 * Sets the player name
 */
void add_user(char *message) {
	char value[BUFSIZE];
	char text[BUFSIZE];

	// Increase number of users
	++number_users;

	// Parses the name
	parse_name(message, value, next_char);
	printf("New player %d: %s\n", number_users, value);

	// Print a message
	if(number_users < MAXUSERS) {
		sprintf(text, "%s connected, waiting for more players\n", value);
	} else {
		sprintf(text, "%s connected, enough players connected\nThe game can start!\n", value);
	}
	display_message(text, view, "bold");

	// Add the new user
	switch(number_users) {
		case 2:
			gtk_label_set_text(GTK_LABEL(label_player_one), value);
			break;
		case 3:
			gtk_label_set_text(GTK_LABEL(label_player_two), value);			
			break;
		case 4:
			gtk_label_set_text(GTK_LABEL(label_player_three), value);
			break;
	}

	// Change the state if all the players are connected
	if(number_users == MAXUSERS) {
		state = GIVE_CARDS;
	}
}

/*
 * Removes the user from the list
 * From: http://zetcode.com/tutorials/gtktutorial/gtktreeview/
 */
void remove_user(char *message) {
	char name[BUFSIZE];
	char p[SIZENAME];
	char text[BUFSIZE];

	// Get the user name
	parse_name(message, name, next_char);
	printf("Remove user: %s\n", name);
	// Print a message
	sprintf(text, "%s disconnected\n", name);
	display_message(text, view, "bold");

	// Get the users
	sprintf(p, "%s", gtk_label_get_text(GTK_LABEL(label_player_one)));
	if(strcmp(p, name) == 0) {
		gtk_label_set_text(GTK_LABEL(label_player_one), gtk_label_get_text(GTK_LABEL(label_player_two)));
		gtk_label_set_text(GTK_LABEL(label_player_two), gtk_label_get_text(GTK_LABEL(label_player_three)));
		return;
	}
	sprintf(p, "%s", gtk_label_get_text(GTK_LABEL(label_player_two)));
	if(strcmp(p, name) == 0) {
		gtk_label_set_text(GTK_LABEL(label_player_two), gtk_label_get_text(GTK_LABEL(label_player_three)));
		return;
	}
	sprintf(p, "%s", gtk_label_get_text(GTK_LABEL(label_player_three)));
	if(strcmp(p, name) == 0) {
		gtk_label_set_text(GTK_LABEL(label_player_three), "Player");
		return;
	}

	// Decreases the number of users
	--number_users;
}

/*
 * Receives a message in broadcast mode
 */
void broadcast(char *message) {
    char text[BUFSIZE];
   	char name[SIZENAME];
   	
   	parse_name(message, name, next_char);
   	sprintf(text, "%s: %s\n", name, consume(message, next_char));
   	
   	display_message(text, view, "blue_fg");
}

void remove_card(char *c) {
	GtkListStore *store;
	GtkTreeModel *model;
	GtkTreeIter iter;
	char *value;
	gboolean isvalid;

	// Get the list
	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW (list)));
	model = gtk_tree_view_get_model (GTK_TREE_VIEW (list));

	// Get the first element
	isvalid = gtk_tree_model_get_iter_first(model, &iter);
		
	// Searching the card
	while(isvalid) {
		// Get the value of the card
		gtk_tree_model_get(model, &iter, LIST_ITEM, &value,  -1);
		
		if(strcmp(c, value) == 0) {
			// Remove the card
			gtk_list_store_remove(store, &iter);
			return;
		}
		
		// Get the next card
		isvalid = gtk_tree_model_iter_next(model, &iter);
	}
}

/*
 *
 */
void play_card(char *message) {
	int suit, set;
	int player;
	char buf[BUFSIZE];
	char c[BUFSIZE];
		
	if(state == GIVE_CARDS) {
		suit = parse_int(message, next_char);
		set = parse_int(message, next_char);
		sprintf(c, "%s %s", suit_string(suit), set_string(set));
		add_to_list(c);
		++cards;
		if(cards == MAX_SET) {
			state = EXCHANGE;
			play = PLAY;
			//sprintf(buf, "Choose 3 cards to send to an opponent\n");
			//display_message(buf, view, "red_fg");
			cards = 0;
		}
		return;
	}

	if(state == EXCHANGE) {
		if(play == PLAY) {
			player = parse_int(message, next_char);
			suit = parse_int(message, next_char);
			set = parse_int(message, next_char);
			sprintf(c, "%s %s", suit_string(suit), set_string(set));
			sprintf(buf, "You choosed: %s\n", c);
			remove_card(c);
			display_message(buf, view, "normal");
			++cards;
			if(cards == 3) {
				play = WAIT;
				cards = 0;
			}
		} else {
			suit = parse_int(message, next_char);
			set = parse_int(message, next_char);
			sprintf(c, "%s %s", suit_string(suit), set_string(set));
			sprintf(buf, "You received: %s\n", c);
			add_to_list(c);
			display_message(buf, view, "normal");
		}
		return;
	}
}

/*
 * Main function of the program
 */
int main(int argc, char **argv) {
	GtkWidget *window;
	int recvlen, rc;
	pthread_attr_t attr;
	void *status;
	pthread_t receiver;

	n = malloc(sizeof(char) * SIZENAME);
	char *buf = malloc(sizeof(char) * BUFSIZE);
	next_char = malloc(sizeof(int));
	selected_card = malloc(sizeof(char *));

	// Set client's name
	if(argc > 0) {
		n = argv[1];
	} else {
		printf("Enter your name: ");
		scanf("%s", n);
	}

	while(strlen(n) > SIZENAME - 1) {
		printf("WARNING - Your name must be under %d characters\nPlease, enter a new one: ", (SIZENAME - 1));
		scanf("%s", n);
	}

	// Set lenght of an address
    len = sizeof(Address);
	
	// Set the address to the server
	memset((char *)&addr, 0, len);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(PORT);

	// create a UDP socket
	if ((cs = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket\n");
		return -1;
	}

	// Sends our name to the server
	sprintf(buf, "%d %s", CONNECT, n);
	printf("Sending: %s\n", buf);
	if (sendto(cs, buf, strlen(buf), 0, (struct sockaddr *)&addr, len) < 0) {
		perror("ERROR - sendto failed");
		free(n);
		free(buf);
		free(next_char);
		return -1;
	}

	// Get the list of connected users
	users = malloc(sizeof(char) * BUFSIZE);
	recvlen = recvfrom(cs, users, BUFSIZE, 0, (struct sockaddr *)&addr, &len);

	if (recvlen > 0) {
		// Add End caracter to the message
		users[recvlen] = '\0';
		printf("Received: \"%s\"\n", users);
		// Stops the execution if the server is full.
		if(strcmp(users, "Server is full") == 0) {
			printf("You cannot play at the moment, the server is full.\n");
			return 0;
		}
	}
		
	// Set the flags
	loop = 0;
	play = WAIT;
	state = WAITING;
	number_users = 1;
	cards = 0;
	sprintf(selected_card, "No cards");

	// Initialize Mutex
	pthread_mutex_init(&mutex, NULL);

	// Initialise GTK
	gtk_init(&argc, &argv);

	// Create the window
	view = gtk_text_view_new();
	window = create_window();
	
	gtk_widget_show(window);

	// Initialize and set thread detached attribute
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	
	// Set the data
	data_thread.client_socket = cs;
	data_thread.address = addr;
	data_thread.addrlen = len;
	data_thread.name = n;

	// Set the threads
	rc = pthread_create(&receiver, &attr, receive_thread, (void *) &data_thread);
	if (rc){
		printf("ERROR %d - Cannot create receiver thread\n", rc); 
		return -1;
	}

	gtk_main();

	// User as closed the window we indicating we want to finish the execution
	loop = 1;
	
	// Notifies the server
	sprintf(buf, "%d %s", QUIT, n);
	printf("Sending: %s\n", buf);
	if (sendto(cs, buf, strlen(buf), 0, (struct sockaddr *)&addr, len) < 0) {
		perror("ERROR - sendto failed");
		return -1;
	}
	
	// Free attribute and wait for threads
	pthread_attr_destroy(&attr);
	
	rc = pthread_join(receiver, &status);
	if (rc) {
		printf("ERROR %d - Cannot join receiver thread\n", rc); 
		return -1;
	}

	pthread_mutex_destroy(&mutex);

	close(cs);
	free(buf);
	free(next_char);
	return 0;
}
