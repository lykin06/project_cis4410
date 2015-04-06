#ifndef VALUES_H_
#define VALUES_H_

// Flags for the different actions
#define GAME		0
#define BROADCAST 	1
#define CONNECT 	2
#define QUIT 		3
#define ADD_CARD	4
#define REMOVE_CARD 5
#define DISPLAY		6
#define POINTS		7

// Different important values
#define BUFSIZE 	512
#define MESSAGE 	257
#define SIZENAME 	65
#define PORT 		2222
#define MAXUSERS 	4

// Flags for tehh turn
#define PLAY 		0
#define WAIT 		1

// Different server states
#define WAITING			0
#define GIVE_CARDS		1
#define EXCHANGE		2
#define HAND			3
#define ROUND			4
#define PAUSE			5
#define PLAYERS			7

// Different exchange values
#define LEFT	1
#define FRONT	2
#define RIGHT	3

// IP address of the server
#define IP "131.104.232.117"

#endif // VALUES_H_
