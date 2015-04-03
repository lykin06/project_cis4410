#include <string.h>

#include "cards.h"

/*
 * Returns the name of the given suit value
 */
char *suit_string(int suit) {
	switch(suit) {
		case SPADES:
			return "Spades";
		case HEARTS:
			return "Hearts";
		case DIAMONDS:
			return "Diamonds";
		case CLUBS:
			return "Clubs";
	}

	return "";
}

/*
 * Returns the value of the given suit name
 */
int suit_int(char *name) {
	if(strcmp(name, "Spades") == 0) {
		return SPADES;
	}
	if(strcmp(name, "Hearts") == 0) {
		return HEARTS;
	}
	if(strcmp(name, "Diamonds") == 0) {
		return DIAMONDS;
	}
	if(strcmp(name, "Clubs") == 0) {
		return CLUBS;
	}

	return -1;
}

/*
 * Returns the name of the given set value
 */
char *set_string(int set) {
	switch(set) {
		case ACE:
			return "Ace";
		case TWO:
			return "Two";
		case THREE:
			return "Three";
		case FOUR:
			return "Four";
		case FIVE:
			return "Five";
		case SIX:
			return "Six";
		case SEVEN:
			return "Seven";
		case EIGHT:
			return "Eight";
		case NINE:
			return "Nine";
		case TEN:
			return "Ten";
		case JACK:
			return "Jack";
		case QUEEN:
			return "Queen";
		case KING:
			return "King";
	}

	return "";
}

/*
 * Returns the value of the given set name
 */
int set_int(char *name) {
	if(strcmp(name, "Ace") == 0) {
		return ACE;
	}
	if(strcmp(name, "Two") == 0) {
		return TWO;
	}
	if(strcmp(name, "Three") == 0) {
		return THREE;
	}
	if(strcmp(name, "Four") == 0) {
		return FOUR;
	}
	if(strcmp(name, "Five") == 0) {
		return FIVE;
	}
	if(strcmp(name, "Six") == 0) {
		return SIX;
	}
	if(strcmp(name, "Seven") == 0) {
		return SEVEN;
	}
	if(strcmp(name, "Eight") == 0) {
		return EIGHT;
	}
	if(strcmp(name, "Nine") == 0) {
		return NINE;
	}
	if(strcmp(name, "Ten") == 0) {
		return TEN;
	}
	if(strcmp(name, "Jack") == 0) {
		return JACK;
	}
	if(strcmp(name, "Queen") == 0) {
		return QUEEN;
	}
	if(strcmp(name, "King") == 0) {
		return KING;
	}

	return -1;
}

/*
 * Returns the value of the card
 */
int card_value(int suit, int set) {
	return ((suit * MAX_SET) + set);
}
