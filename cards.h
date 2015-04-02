#ifndef CARDS_H_
#define CARDS_H_

// Defines the values for a French Card Deck

#define NUMBER_OF_CARDS 52
#define MAX_SET 13
#define MAX_SUIT 4

typedef struct card_t
{
	enum {SPADES, HEARTS, DIAMONDS, CLUBS} suit;
	enum {ACE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING} set;
	int player;
} Card;

#endif // CARDS_H_