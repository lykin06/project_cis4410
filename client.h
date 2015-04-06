#ifndef CLIENT_H_
#define CLIENT_H_

#include <netinet/in.h>

// Number of possible actions
#define ACTIONS 7

// Address
typedef struct sockaddr_in Address;

// Struct to store all the data threads
typedef struct thread_data_t Data;

// Function pointer
typedef void (*funcptr)(char *message);

struct thread_data_t {
	// Socket
	int client_socket;

	// Address
	Address address;

	// Lenght of an address
	socklen_t addrlen;

	// Name of the client;
	char *name;
};

/*
 * Adds the connected players
 */
void set_players();

/*
 * Sets the player name
 */
void add_user(char *message);

/*
 * Removes the user from the list
 */
void remove_user(char *message);

/*
 * Receives a message in broadcast mode
 */
void broadcast(char *message);

/*
 * A user play a card
 */
void play_card(char *message);

/*
 * Sends the message to the server
 */
void send_message(char *message);

/*
 * Adds the card tot he list
 */
void add_card(char *message);

/*
 * Removes the card from the list
 */
void remove_card(char *message);

/*
 * Displays a card played by another player
 */
void display_card(char *message);

#endif // CLIENT_H_

