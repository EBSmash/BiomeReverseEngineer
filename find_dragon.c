#include <time.h>
#include <stdio.h>
#include <pthread.h>
#include <inttypes.h>

#include "spiral_iterator.h"
#include "cubiomes/generator.h"
#include "cubiomes/util.h"

// This program searches "windows", that is areas with
// dimensions windowSize by windowSize blocks in a spiral
// pattern, starting at (0, 0).
const uint64_t seed = -4172144997902289642LL;
// How many windows should I use?
// If you want to get up to N blocks out, the formula is:
//    (2*N/windowSize)**2
// where `**2` is the square (second power).
const int numWindows = 10000;
const int windowSize = 1024;
const int marginSize = 128;
const int areaSize = windowSize + marginSize;

// The program is multithreaded, the number of threads
// should be proportional to the number of cores.
// But not necessarily equal! See the stats below for a
// 96-core Amazon virtual machine.
//
// Some stats (as of 2021-07-29) for an Amazon EC2 instance
// of the c5a.24xlarge type, with 96 vCPUs:
//    numThreads==1   --->   150 Mblocks/sec
//    numThreads==2   --->   300 Mblocks/sec
//    numThreads==4   --->   580 Mblocks/sec
//    numThreads==8   --->  1200 Mblocks/sec
//    numThreads==16  --->  2200 Mblocks/sec
//    numThreads==32  --->  3900 Mblocks/sec  <--- best
//    numThreads==48  --->  3500 Mblocks/sec
//    numThreads==64  --->  3200 Mblock/sec
//    numThreads==80  --->  3100 Mblock/sec
//    numThreads==96  --->  3100 Mblocks/sec
//    numThreads==112 --->  3000 Mblocks/sec
const int numThreads = 4;

int accessBiome(const int* biomeIds, const int x, const int z) {
    return biomeIds[(z*areaSize) + x];
}

int isDesert(int id) {
    switch(id) {
    case desert:
    case desert_hills:
        return 1;
    default:
        return 0;
    }
}

/*
   here's the pattern:
   0     1       2       3       4       5     6     7       8      9        10    11    12
   mesa, desert, desert, desert, desert, mesa, mesa, desert, desert
   mesa, mesa,   desert, mesa,   mesa,   mesa, mesa, desert, desert
   mesa, mesa,   mesa,   mesa,   mesa,   mesa, mesa, desert, desert, desert, mesa, mesa, desert


   the widest part of the pattern has 13 blocks
   the height of the pattern is 3 blocks
*/
int old_has_dragon_pattern_at(const int* biomeIds, const int x, const int z) {
    // first row
    if (!isMesa(accessBiome(biomeIds, x, z))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 1, z))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 2, z))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 3, z))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 4, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 5, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 6, z))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 7, z))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 8, z))){
        return 0;
    }

    // second row
    if (!isMesa(accessBiome(biomeIds, x, z + 1))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 1, z + 1))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 2, z + 1))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 3, z + 1))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 4, z + 1))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 5, z + 1))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 6, z + 1))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 7, z + 1))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 8, z + 1))){
        return 0;
    }

    // third row
    if (!isMesa(accessBiome(biomeIds, x, z + 2))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 1, z + 2))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 2, z + 2))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 3, z + 2))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 4, z + 2))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 5, z + 2))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 6, z + 2))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 7, z + 2))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 8, z + 2))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 9, z + 2))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 10, z + 2))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 11, z + 2))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 12, z + 2))){
        return 0;
    }


    // Found it!
    return 1;
}

/*
   here's the pattern:
   0     1       2       3       4       5       6       7       8      9        10      11
   mesa, desert, desert, desert, desert, desert, desert, desert, desert, desert, desert, mesa 
   mesa, mesa,   desert, desert, desert, mesa,   mesa,   mesa,   desert, desert, mesa,   mesa
   mesa, mesa,   desert, desert, mesa,   mesa,   mesa,   mesa,   desert, mesa,   mesa,   mesa
   mesa, mesa,   mesa,   mesa,   mesa,   mesa,   mesa,   mesa,   mesa,   mesa,   mesa,   mesa


   the widest part of the pattern has 12 blocks
   the height of the pattern is 4 blocks
*/
int has_dragon_pattern_at(const int* biomeIds, const int x, const int z) {
    // first row
    if (!isMesa(accessBiome(biomeIds, x, z))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 1, z))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 2, z))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 3, z))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 4, z))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 5, z))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 6, z))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 7, z))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 8, z))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 9, z))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 10, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 11, z))){
        return 0;
    }

    // second row
    if (!isMesa(accessBiome(biomeIds, x, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 1, z))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 2, z))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 3, z))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 4, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 5, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 6, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 7, z))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 8, z))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 9, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 10, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 11, z))){
        return 0;
    }

    // third row
    if (!isMesa(accessBiome(biomeIds, x, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 1, z))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 2, z))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 3, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 4, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 5, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 6, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 7, z))){
        return 0;
    }
    if (!isDesert(accessBiome(biomeIds, x + 8, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 9, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 10, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 11, z))){
        return 0;
    }

    // fourth row
    if (!isMesa(accessBiome(biomeIds, x, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 1, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 2, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 3, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 4, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 5, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 6, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 7, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 8, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 9, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 10, z))){
        return 0;
    }
    if (!isMesa(accessBiome(biomeIds, x + 11, z))){
        return 0;
    }

    // Found it!
    return 1;
}

int has_dragon_pattern(const int* biomeIds, const int areaX, const int areaZ) {
    for (int z = 0; z < areaSize - 4; ++z) {
        for (int x = 0; x < areaSize - 12; ++x) {
            if (has_dragon_pattern_at(biomeIds, x, z)) {
                // Found it!
                printf("Found it - x:%d z:%d\n", areaX+x, areaZ+z);
                return 1;
            }
        }
    }
    // Did not find it.
    return 0;
}

int has_mesa(const int* biomeIds) {
    int mesa_found = 0;
    for (int z = 0; z < areaSize; ++z) {
        for (int x = 0; x < areaSize; ++x) {
            const int biome = accessBiome(biomeIds, x, z);
            if (isMesa(biome)) {
                mesa_found = 1;
            }
            if (mesa_found) {
                break;
            }
        }
        if (mesa_found) {
            break;
        }
    }
    return mesa_found;
}

int64_t distance(int64_t a, int64_t b) {
    a = (a>0) ? a : (-a);
    b = (b>0) ? b : (-b);
    return (b>a) ? b : a;
}

struct ThreadData {
    // Inputs
    int thread_id;

    // Outputs
    int64_t windows_processed;
    int64_t found_matches;
};

void* thread_func(void* ptr) {
    struct ThreadData* data = (struct ThreadData*)ptr;
    int thread_id = data->thread_id;

    // Initialize a stack of biome layers.
    LayerStack g;
    setupGenerator(&g, MC_1_12);
    // Extract the desired layer.
    Layer *layer = &g.layers[L_VORONOI_1];

    // Allocate a sufficient buffer for the biomes and for the image pixels.
    int *biomeIds = allocCache(layer, areaSize, areaSize);

    // Apply the seed only for the required layers
    setLayerSeed(layer, seed);

    // Each thread processes their own modulus of the windows
    int64_t found_matches = 0;
    int64_t windows_processed = 0;
    for (int64_t window_i = thread_id; window_i < numWindows; window_i += numThreads) {
        // Compute the coordinates of the window.
        int64_t x = 0;
        int64_t z = 0;
        flat_to_xz_spiral(window_i, &x, &z);
        x *= windowSize;
        z *= windowSize;

        // Generate the area.
        genArea(layer, biomeIds, x, z, areaSize, areaSize);

        // Does it have mesa?
        if (has_mesa(biomeIds)) {
            // Does it have the dragon?
            if (has_dragon_pattern(biomeIds, x, z)) {
                found_matches++;
            }
        }
        windows_processed++;

        // Every now and then print progress information
        if (windows_processed % 100 == 0) {
            const float blocks_processed =
                (float)windows_processed * (float)windowSize * (float)windowSize;
            printf(
                "Thread %2d has processed %" PRId64 " windows (%.1f billion blocks), "
                "and is currently at distance %" PRId64 " from origin\n",
                thread_id, windows_processed,
                blocks_processed * 1e-9, distance(x, z));
        }
    }


    // Clean up.
    free(biomeIds);

    // Write outputs.
    data->windows_processed = windows_processed;
    data->found_matches = found_matches;

    return ptr;
}

int main()
{
    // Time measurement, for performance stats only.
    const time_t start_time = time(NULL);

    // Initialize thread datas.
    struct ThreadData thread_datas[numThreads];
    for (int i = 0; i < numThreads; ++i) {
        thread_datas[i].thread_id = i;
    }

    // Kick off the threads.
    pthread_t threads[numThreads];
    for (int i = 0; i < numThreads; ++i) {
        pthread_create(&(threads[i]), NULL, thread_func, (void*) &(thread_datas[i]));
    }

    // Join the threads after they're done with computation.
    for (int i = 0; i < numThreads; ++i) {
        pthread_join(threads[i], NULL);
    }

    // Time measurement, for performance stats only.
    const clock_t end_time = time(NULL);
    const float time_seconds = difftime(end_time, start_time);

    // Aggregate statistics.
    int64_t windows_processed = 0;
    int64_t found_matches = 0;
    for (int i = 0; i < numThreads; ++i) {
        windows_processed += thread_datas[i].windows_processed;
        found_matches += thread_datas[i].found_matches;
    }
    const float blocks_processed =
        (float)windows_processed * (float)windowSize * (float)windowSize;
    const float blocks_per_sec = blocks_processed / time_seconds;

    // Print stats.
    printf("\n");
    printf("All done.\n");
    printf(
        "Processed %" PRId64 " windows, %.1f billions of blocks\n",
        windows_processed, blocks_processed * 1e-9f);
    printf("Processing time: %.1f sec.\n", time_seconds);
    printf("Processing speed: %.1f millions of blocks per second\n",
           blocks_per_sec * 1e-6);
    printf("Found %" PRId64 " matches\n", found_matches);

    return 0;
}
