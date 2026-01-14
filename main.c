#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <threads.h>

#define LONESHA256_STATIC
#include "lonesha256.h"

#define MAX_THREADS 8

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

char *challenge;
int difficulty;
int num_threads;

atomic_bool found;
atomic_int result;

typedef struct
{
    uint32_t thread_id;
} work_ctx_t;

uint32_t work(work_ctx_t *ctx)
{
    uint32_t nonce = ctx->thread_id - 1;

    uint8_t hash[32];
    uint8_t candidate[64];

    do
    {
        // Check if other threads finished
        if (atomic_load_explicit(&found, memory_order_relaxed))
            return -1;

        nonce += num_threads;

        size_t len = snprintf(candidate, 64, "%s%d", challenge, nonce);
        lonesha256(hash, candidate, len);

#ifdef DEBUG
        print_hash(hash);
#endif
    } while (!verify(hash, difficulty));

    atomic_store(&result, nonce);
    atomic_store_explicit(&found, true, memory_order_relaxed);

    return nonce;
}

int32_t main(int32_t argc, uint8_t *argv[])
{
    if (argc < 3)
    {
        printf("usage: %s <challenge string> <difficulty> [thread count]", argv[0]);
        return;
    }

    challenge = argv[1];

    if (strlen(challenge) == 0)
    {
        printf("Challenge string must not be an empty.\n");
        return;
    }

    difficulty = (uint32_t)strtol(argv[2], NULL, 10);

    if (difficulty <= 0)
    {
        printf("Difficulty must be greater than 0.\n");
        return;
    }

    num_threads = (argc == 4) ? (uint32_t)strtol(argv[3], NULL, 10) : 1;

    if (num_threads <= 0 || num_threads > MAX_THREADS)
    {
        printf("Thread count must be between 1 and %d.\n", MAX_THREADS);
        return;
    }

    thrd_t ts[MAX_THREADS];
    work_ctx_t *args[MAX_THREADS];
    uint32_t ns[MAX_THREADS];

    for (int i = 0; i < num_threads; i++)
    {
        args[i] = malloc(sizeof(work_ctx_t));
        args[i]->thread_id = i;

        if (thrd_create(ts + i, work, args[i]) != thrd_success)
        {
            printf("Failed to create thread %d\n", i);
            return;
        }
    }

    for (int i = 0; i < num_threads; i++)
    {
        thrd_join(ts[i], ns + i);
        free(args[i]);

        if (ns[i] != -1)
            printf("%d\n", ns[i]);
    }
}