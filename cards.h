#ifndef CARDS_H_
#define CARDS_H_

typedef struct card_t
{
	enum {SPADES, HEARTS, DIAMONDS, CLUBS} suit;
	enum {ACE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN, JACK, QUEEN, KING} set;
	int player;
} Card;

#endif // CARDS_H_