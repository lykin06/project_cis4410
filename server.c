#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

#include "values.h"
#include "server.h"
#include "parsing.h"
#include "cards.h"
#include "functions.h"

// Address of the server
Address myaddr;

// Socket of the server
int server_socket;

// Lenght of an address
socklen_t addrlen;

// Integer holding the position of the next char to read
int *next_char;

// List of connected users, cannot have more than MAXUSERS users
User users[MAXUSERS];

// Number of connected users
int number_users;

// user's action
int action;

// Deck of cards
Card deck[NUMBER_OF_CARDS];

// Player buffers
int pone;
Card one[3];
int ptwo;
Card two[3];
int pthree;
Card three[3];
int pfour;
Card four[3];

// Buffer including card played
Card played[4];
int pcard;
int round_left;

// Pointer to functions
funcptr *functions;

// State of the server
int state;

// Card exchange
int exchange;

// Player's turn
int turn;

/*
 * Sends a message to a player
 */
void send_message(char *message, Address address) {
	printf("Server: Sends message \"%s\"\n", message);
	if (sendto(server_socket, message, strlen(message), 0, (struct sockaddr *)&address, addrlen) < 0) {
		perror("ERROR - sendto failed");
		exit(-1);
	}
}

/*
 * Initialize the server
 * Sets the deck of cards to be a 52 French Card Deck
 */
void init_server() {
	// Set number of players
	number_users = 0;

	// Set the function pointer
	functions = malloc(sizeof(funcptr) * ACTIONS);
	functions[0] = &play_game;
	functions[1] = &broadcast;
	functions[2] = &add_user;
	functions[3] = &disconnect;

	// Initialize the deck of cards
	reset_cards();

	// Initialize the points
	reset_points();

	// Sets the state of the server
	state = WAITING;
	exchange = LEFT;

	turn = 0;
	pone = 0;
	ptwo = 0;
	pthree = 0;
	pfour = 0;
}

/*
 * Returns the id of the given username
 * Returns -1 if the user is not in the list
 */
int get_userid(char *name) {
	printf("get_userid, name %s\n", name);
	int i;
	
	for(i = 0; i < number_users; ++i) {
		printf("loop %d, test %s\n", i, users[i].name);
		if(strcmp(users[i].name, name) == 0) {
			return i;
		}
	}
	
	// Not in the list
	return -1;
}

/*
 * Adds the user to the list of users if there is less than MAXUSERS users
 * Notifies the other players.
 */
void add_user(char *message, Address remaddr) {
	int i;
	char names[BUFSIZE];
	char *buf = malloc(sizeof(char) * BUFSIZE);

	if(number_users < MAXUSERS) {
		// Sets the list of players
		if(number_users > 0) {
			names[0] = '\0';
			sprintf(names, "%d ", number_users);
			for(i = 0; i < number_users; ++i) {
				// Adds the name to the list
				sprintf(buf, "%s ", users[i].name);
				strcat(names, buf);

				// Notifies the player
				send_message(message, users[i].addr);
			}
		} else {
			sprintf(names, "%d ", 0);
		}

		// Sends the list of players to the new client
		send_message(names, remaddr);
		
		// Gets the name of the user
		buf = consume(message, next_char);
		i = strlen(buf);
		printf("Adding user %s size %d\n", buf, i);
		
		users[number_users].name = buf;
		users[number_users].addr = remaddr;
		users[number_users].points = 0;
		printf("Added %s\n", users[number_users].name);
		
		// Increases number of users
		++number_users;

		// Changes the state if there are enough players
		if(number_users == MAXUSERS) {
			state = GIVE_CARDS;
		}
	} else {
		// Sends a "server is full" message to the player
		sprintf(buf, "Server is full");
		send_message(buf, remaddr);
	}
}

/*
 * Removes the users from the list
 */
void disconnect(char *message, Address remaddr) {
	int i;
	
	// Flag indicating we removes the user
	int removed = 0;

	// Sends the message to all the others
	broadcast(message, remaddr);
	
	// Gets the name of the user
	char *buf = consume(message, next_char);
	
	// Notifies the user
	sprintf(message, "You are disconnected");
	send_message(message, remaddr);
	
	// Removes the user from the list
	if(number_users > 1) {
		for(i = 0; i < number_users; ++i) {
			if(removed > 0) {
				// Shifts the user
				users[i - 1] = users[i];
			} else {
				// We doing nothing until we found the user
				if(strcmp(buf, users[i].name) == 0) {
					++removed;
				}
			}
		}
	}
	
	--number_users;
	printf("User removed, %d users left\n", number_users);

	// Change the state
	state = WAITING;
}

/*
 * Sends the message to all the users
 */
void broadcast(char *message, Address remaddr) {
	printf("Broadcast\n");
	int i;
	int id;
	char *n = malloc(sizeof(char) * BUFSIZE);
	char buf[BUFSIZE];
	
	// Get user name
	parse_name(message, n, next_char);
	id = get_userid(n);
	printf("Sender: %s, id: %d, next_char = %d\n", n, id, *next_char);

	// Parse the rest of the message
	sprintf(buf, "%s", consume(message, next_char));

	// Waiting for all the users
	if(state == PAUSE) {
		if(strcmp(buf, "ready") == 0) {
			++pcard;
			if(pcard == MAXUSERS) {
				state = GIVE_CARDS;
			}
		}
	}
	
	if(id >= 0) {
		printf("Known user\n");
		if(number_users > 1) {
			printf("Enough users\n");
			for(i = 0; i < number_users; ++i) {
				printf("loop %d, test %d\n", i, id);
				if(i != id) {
					send_message(message, users[i].addr);
				} else {
					printf("Sender %s\n", users[id].name);
				}
			}
		} else {
			printf("Not enough users\n");
		}
	} else {
		printf("Unknown user\n"); 
	}
	
	free(n);
}

/*
 * Sends the message to one user
 */
void play_game(char *message, Address remaddr) {
	int p, suit, set, i, moon, loser, points;
	char buf[BUFSIZE];

	printf("state %d\n", state);

	if(state == EXCHANGE) {
		p = parse_int(message, next_char);
		suit = parse_int(message, next_char);
		set = parse_int(message, next_char);
		switch(p) {
			case 0:
				one[pone].suit = suit;
				one[pone].set = set;
				++pone;
				i = 3 - pone;
				break;
			case 1:
				two[ptwo].suit = suit;
				two[ptwo].set = set;
				++ptwo;
				i = 3 - ptwo;
				break;
			case 2:
				three[pthree].suit = suit;
				three[pthree].set = set;
				++pthree;
				i = 3 - pthree;
				break;
			case 3:
				four[pfour].suit = suit;
				four[pfour].set = set;
				++pfour;
				i = 3 - pfour;
				break;
		}

		// Notifies the sender
		sprintf(buf, "%d %d %d ", REMOVE_CARD, suit, set);
		send_message(buf, users[p].addr);

		if((pone + ptwo + pthree + pfour) == 12) {
			printf("exchange\n");
			exchange_cards();

			// Notifies the first player
			sprintf(buf, "%d %d Please, selected the two of spades", GAME, HAND);
			printf("%s starts the game\n", users[turn].name);
			send_message(buf, users[turn].addr);

			state = HAND;
		} else {
			if(i > 0) {
				sprintf(buf, "%d %d Please, selected %d more cards", GAME, EXCHANGE, i);
			} else {
				sprintf(buf, "%d %d Waiting for other players...", GAME, PLAYERS);
			}
			send_message(buf, users[p].addr);
		}

		return;
	}

	if(state == HAND) {
		// Receives a card played
		p = parse_int(message, next_char);
		suit = parse_int(message, next_char);
		set = parse_int(message, next_char);
		played[pcard].suit = suit;
		played[pcard].set = set;
		played[pcard].player = p;

		// Sends the card to the players
		for(i = 0; i < MAXUSERS; ++i) {
			if(i == p) {
				sprintf(buf, "%d %d %d ", REMOVE_CARD, suit, set);
			} else {
				sprintf(buf, "%d %s %d %d ", DISPLAY, users[p].name, suit, set);
			}
			send_message(buf, users[i].addr);
		}

		// Increases the flag
		++pcard;
		printf("Cards played %d\n", pcard);

		// Checks if all the players has played
		if(pcard == MAXUSERS) {
			printf("End of round\n");

			// Calculates the points
			turn = play_round();

			// Notifies players
			sprintf(buf, "%d %d %s won the round", GAME, PLAYERS, users[turn].name);
			for(i = 0; i < MAXUSERS; ++i) {
				send_message(buf, users[i].addr);
			}

			// Restarts a round
			pcard = 0;
			--round_left;
			printf("%d rounds left\n", round_left);
		}

		// Checks if all the rounds have been played
		if(round_left == 0) {
			// Compares the points
			moon = compare_points();
			if(moon > -1) {
				for(i = 0; i < MAXUSERS; ++i) {
					if(i != moon) {
						users[i].points = MOON;
					} else {
						users[i].points = 0;
					}
				}
			}

			// Changes state to PAUSE
			state = PAUSE;

			// Adds the points to the score
			sprintf(buf, "%d %d %s", POINTS, PAUSE, add_points());

			// Notifies the players
			loser = -1;
			points = 100;
			for(i = 0; i < MAXUSERS; ++i) {
				send_message(buf, users[i].addr);
				if(users[i].score > points) {
					loser = i;
					points = users[i].score;
				}
			}

			// End game
			if(loser > -1) {
				sprintf(buf, "%d %s", END, users[loser].name);
				for(i = 0; i < MAXUSERS; ++i) {
					send_message(buf, users[i].addr);
				}
			}
		} else {
			// Notifies the next player
			if(pcard != 0) {
				turn = (turn + 1) % 4;
			}
			sprintf(buf, "%d %d Please, selected a card", GAME, HAND);
			send_message(buf, users[turn].addr);
		}
	}
}

/*
 * Free the global variables
 */
void free_vars() {
	free(next_char);
	close(server_socket);
}

int check_action(int action) {
	if(action >= ACTIONS) {
		return -1;
	}

	if(action < 0) {
		return -1;
	}

	if(state == WAITING) {
		if(action == GAME) {
			return -1;
		}
	}

	if(state > WAITING) {
		// Players can't disconnect
		if(action == QUIT) {
			return -1;
		}
	}

	return 0;
}

void reset_points() {
	int i;

	for (i = 0; i < MAXUSERS; ++i) {
		users[i].points = 0;
	}
}

char *add_points() {
	char *buf = malloc(sizeof(char*));
	char c[BUFSIZE];
	int i;

	for(i = 0; i < MAXUSERS; ++i) {
		users[i].score += users[i].points;
		users[i].points = 0;
		sprintf(c, "%s %d ", users[i].name, users[i].score);
		strcat(buf, c);
	}

	return buf;
}

int compare_points() {
	int i, moon = -1;

	for(i = 0; i < MAXUSERS; ++i) {
		if(users[i].points == MOON) {
			moon = i;
		}
	}

	return moon;
}

/*
 * Resets the cards position
 */
void reset_cards() {
	int suit, set, card = 0;
	
	for(suit = 0; suit < MAX_SUIT; ++suit) {
		for(set = 0; set < MAX_SET; ++set) {
			deck[card].set = set;
			deck[card].suit = suit;
			deck[card].player = -1;
			++card;
		}
	}
}

/*
 * Shuffles the deck of cards
 * Iterates over the deck and swift cards
 */
void shuffle_cards() {
	Card tmp;
	int i, j;

	for(i = 0; i < NUMBER_OF_CARDS; ++i) {
		// Reset the player
		deck[i].player = -1;
		tmp.suit = deck[i].suit;
		tmp.set = deck[i].set;
		j = my_rand(0, NUMBER_OF_CARDS);
		deck[i].suit = deck[j].suit;
		deck[i].set = deck[j].set;
		deck[j].suit = tmp.suit;
		deck[j].set = tmp.set;
	}
}

/*
 * Gives the cards to all the players
 */
void give_cards() {
	int i,j;
	int card = 0;
	char message[BUFSIZE];

	for(i = 0; i < MAX_SET; ++i) {
		for(j = 0; j < MAXUSERS; ++j) {
			deck[card].player = i;
			sprintf(message, "%d %d %d ", ADD_CARD, deck[card].suit, deck[card].set);
			send_message(message, users[j].addr);

			// Sets the first player
			if(card_value(deck[card].suit, deck[card].set) == TWO) {
				turn = j;
				printf("%s has the two of spades\n", users[turn].name);
			}

			++card;
		}
	}
}

void send_card(int suit, int set, int sender) {
	int receiver;
	char buf[BUFSIZE];

	// Computes the receiver
	receiver = (sender + exchange) % MAXUSERS;

	// Sets the message to send
	sprintf(buf, "%d %d %d ", ADD_CARD, suit, set);

	// Sends the message
	send_message(buf, users[receiver].addr);

	// Changes the value of the first player
	if(card_value(suit, set) == TWO) {
		turn = receiver;
		printf("%s has the two of spades\n", users[turn].name);
	}
}

/*
 * Gives the three cards from a player to another
 */
void exchange_cards() {
	int i;

	for(i = 0; i < 3; ++i) {
		send_card(one[i].suit, one[i].set, 0);
		send_card(two[i].suit, two[i].set, 1);
		send_card(three[i].suit, three[i].set, 2);
		send_card(four[i].suit, four[i].set, 3);
	}

	if(exchange == RIGHT) {
		exchange = LEFT;
	} else {
		++exchange;
	}

	// Changes state
	state = HAND;

	// Sets the pointer buffer
	pcard = 0;
	round_left = 13;
}

int play_round() {
	int i, win = 0, points = 0;

	for(i = 0; i < MAXUSERS; ++i) {
		if(played[i].suit == HEARTS) {
			++points;
		}

		if(card_value(played[i].suit, played[i].set) == SPADES_QUEEN) {
			points += 13;
		}

		if((played[win].suit == played[i].suit) && (played[i].set > played[win].set)) {
			win = i;
		}
	}

	users[win].points += points;

	return win;
}

/*
 * Sets up a new game
 */
void setup_game() {
	int i;
	char buf[BUFSIZE];

	shuffle_cards();
	give_cards();
	
	// Notifies the players to exchange the cards
	for(i = 0; i < MAXUSERS; ++i) {
		sprintf(buf, "%d %d Please, select 3 cards.", GAME, EXCHANGE);
		send_message(buf, users[i].addr);
	}

	state = EXCHANGE;
}

/*
 * Holds the main loop of the program
 * Receives and processes the messages from the client
 */
void server() {
	int recvlen;
	char *message;
	char *buf;
	Address remaddr;
	
	// Set local the variables
	message = malloc(sizeof(char) * BUFSIZE);
	buf = malloc(sizeof(char) * BUFSIZE);

	// Game Loop
	while(1) {
		// Waiting for players
		if(number_users < MAXUSERS) {
			printf("Server: Waiting on port %d for more players\n%d players connected\n", PORT, number_users);
		} else {
			printf("Server: Waiting on port %d\n", PORT);
		}

		if(state == GIVE_CARDS) {
			setup_game();
		}

		recvlen = recvfrom(server_socket, message, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
		
		// Check if the message is not empty before processing it
		if (recvlen > 0) {
			// Add End caracter to the message
			message[recvlen] = '\0';
			next_char[0] = 0;
			printf("Server: Received message: \"%s\"\n", message);
			
			// Parse the action wanted by the client
			action = parse_int(message, next_char);
			printf("Will perform action %d, next_char = %d\n", action, *next_char);
			
			// Check if the action is correct
			if(check_action(action) < 0) {
				sprintf(buf, "Not allowed request");
				send_message(buf, remaddr);
			} else {
				// Process the request
				functions[action](message, remaddr);
			}			
		} else {
			printf("Server: Cannot process request, Not enough data received.\n");
		}
	}
	
	free(functions);
}

/*
 * Main function
 * Set all the needed values and start the main loop
 */
int main(int argc, char **argv) {
	// Set the global variables
	addrlen = sizeof(struct sockaddr_in);
	next_char = malloc(sizeof(int));
	
	// Set server address and port
	memset((char *)&myaddr, 0, addrlen);
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(PORT);
	
	// create a UDP socket
	if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("ERROR - cannot create socket\n");
		free_vars();
		return -1;
	}

	// Bind the socket to the address
	if (bind(server_socket, (struct sockaddr *)&myaddr, addrlen) < 0) {
		perror("ERROR - bind failed\n");
		free_vars();
		close(server_socket);
		return -1;
	}
	
	init_server();
	server();

	return 0;
}
