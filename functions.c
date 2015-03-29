#include <time.h>
#include <stdlib.h>

/*
 * Returns a random number between min and max
 * uses rand() function from sdtio.h and srand() to initialize the seed
 */
int my_rand(int min, int max) {
    time_t t;
    srand((unsigned) time(&t) * rand());
    return (rand() % (max - min + 1)) + min;
}