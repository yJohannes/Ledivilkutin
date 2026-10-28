#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


int main()
{
    uint8_t a = 110u;
    uint8_t b = 140u;

    int8_t diff = (int8_t)(a - b);

    printf("%d", diff); // -30

    return EXIT_SUCCESS;
}
