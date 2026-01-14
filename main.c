#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define LONESHA256_STATIC
#include "lonesha256.h"

void print_hash(uint8_t *hash)
{
    for (size_t i = 0; i < 32; i++)
        printf("%02X", hash[i]);
    printf("\n");
}

bool verify(uint8_t hash[32], int difficulty)
{
    int leading_zero_nibbles = 0;

    for (size_t i = 0; i <= difficulty / 2; i++)
    {
        // Check upper nibble
        if (hash[i] & 0b11110000)
            break;
        if (++leading_zero_nibbles == difficulty)
            return true;

        // Check lower nibble
        if (hash[i] & 0b00001111)
            break;
        if (++leading_zero_nibbles == difficulty)
            return true;
    }

    return false;
}

int32_t main(int32_t argc, uint8_t *argv[])
{
    if (argc != 3)
    {
        printf("usage: %s <challenge string> <difficulty>", argv[0]);
        return;
    }

    uint8_t *challenge = argv[1];

    if (strlen(challenge) == 0)
    {
        printf("Challenge string must not be an empty.\n");
        return;
    }

    uint32_t difficulty = (uint32_t)strtol(argv[2], NULL, 10);

    if (difficulty <= 0)
    {
        printf("Difficulty must be greater than 0.\n");
        return;
    }

    uint32_t nonce = -1;

    uint8_t hash[32];
    uint8_t candidate[64];

    do
    {
        nonce++;

        size_t len = snprintf(candidate, 64, "%s%d", challenge, nonce);
        lonesha256(hash, candidate, len);

#ifdef DEBUG
        print_hash(hash);
#endif
    } while (!verify(hash, difficulty));

    printf("\nfinal nonce: %d\n", nonce);
}