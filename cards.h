#ifndef CARDS_H_
#define CARDS_H_

// Defines the values for a French Card Deck

#define NUMBER_OF_CARDS 52
#define MAX_SET 13
#define MAX_SUIT 4
#define SPADES_QUEEN 11

typedef struct card_t
{
	enum {SPADES, HEARTS, DIAMONDS, CLUBS} suit;
	enum {ACE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING} set;
	int player;
} Card;

/*
 * Returns the name of the given suit value
 */
char *suit_string(int suit);

/*
 * Returns the value of the given suit name
 */
int suit_int(char *name);

/*
 * Returns the name of the given set value
 */
char *set_string(int set);

/*
 * Returns the value of the given set name
 */
int set_int(char *name);

/*
 * Returns the value of the card
 */
int card_value(int suit, int set);

#endif // CARDS_H_