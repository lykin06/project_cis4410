#ifndef SERVER_H_
#define SERVER_H_

// Number of possible actions
#define ACTIONS 4

#include <netinet/in.h>

// Holds the users information
typedef struct User_t User;

// Address
typedef struct sockaddr_in Address;

struct User_t {
	// User's name
	char *name;
	
	// User's address
	Address addr;
	
	// Number of points
	int points;
};

// Function pointer
typedef void (*funcptr)(char *message, Address remaddr);

/*
 * Sends a message to a player
 */
void send_message(char *message, Address address);

/*
 * Initialize the server
 * Sets the deck of cards to be a 52 French Card Deck
 */
void init_server();

/*
 * Waiting for players loop
 * Stops when there are 4 players
 * Previous players can leave
 */
void wait_for_players();

/*
 * Returns the id of the given username
 * Returns -1 if the user is not in the list
 */
int get_userid(char *name);

/*
 * Adds the user to the list of users if there is less than MAXUSERS users
 * Notifies the other players.
 */
void add_user(char *message, Address remaddr);

/*
 * Removes the users from the list
 */
void disconnect(char *message, Address remaddr);

/*
 * Sends the message to all the users
 */
void broadcast(char *message, Address remaddr);

/*
 * Sends the message to one user
 */
void play_game(char *message, Address remaddr);

/*
 * Holds the main loop of the program
 * Receives and processes the messages from the client
 */
void server();

/*
 * Free the global variables
 */
void free_vars();

/*
 * Resets the cards position
 */
void reset_cards();

/*
 * Shuffles the deck of cards
 * Iterates over the deck and swift cards
 */
 void shuffle_cards();

 /*
 * Gives the three cards from a player to another
 */
void exchange_cards();

#endif // SERVER_H_
