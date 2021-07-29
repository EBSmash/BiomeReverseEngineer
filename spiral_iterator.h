#ifndef SPIRAL_ITERATOR_H_
#define SPIRAL_ITERATOR_H_

#include <assert.h>
#include <math.h>

// This function converts a flat index (a non-negative number)
// into x and z coordinates, so that when `flat` goes from 0, 1, 2, ...
// the x and z are going through all locations, starting from 0,0
// and going outwards in a spiral fashion.
//
// Example up to `flat` = 24, that is first 25 positions,
// data presented as x,z (flat):
//  -2,-2 (9)    -1,-2 (10)   0,-2 (11)   1,-2 (12)   2,-2 (13)
//  -2,-1 (24)   -1,-1 (1)    0,-1 (2)    1,-1 (3)    2,-1 (14)
//  -2, 0 (23)   -1, 0 (8)    0, 0 (0)    1, 0 (4)    2, 0 (15)
//  -2, 1 (22)   -1, 1 (7)    0, 1 (6)    1, 1 (5)    2, 1 (16)
//  -2, 2 (21)   -1, 2 (20)   0, 2 (19)   1, 2 (18)   2, 2 (17)
static void flat_to_xz_spiral(int64_t flat, int64_t* x, int64_t* z) {
    // Preconditions.
    assert(flat >= 0);
    assert(x != NULL);
    assert(z != NULL);

    // The first position is kind of special, it messes up some formulas.
    // Just handle it explicitly
    if (flat == 0) {
        *x = 0;
        *z = 0;
        return;
    }
    // Now we have `flat > 0`, and thus for example `distance > 0` too.

    // How far are we from 0,0 in Manhattan distance?
    const double distance_tmp = sqrt(((double)flat) + 1e-3);
    int64_t distance = floor(distance_tmp);
    if (distance % 2 == 1) {
        distance += 1;
    }
    distance /= 2;

    // What's the linear offset at this distance?
    int64_t linear_offset = flat - (2*distance - 1)*(2*distance - 1);

    // Now we need to find out which part of the spiral are we at (up, right, down or left).
    // The below value is always 0, 1, 2 or 3.
    int64_t part = linear_offset / (2*distance);
    // And what's the offset in this part.
    int64_t part_offset = linear_offset - (part * 2 * distance);
    switch(part) {
        case 0:
            // We're at the top of the spiral.
            *z = -distance;
            *x = (-distance) + part_offset;
            break;
        case 1:
            // We're at the right side of the spiral.
            *x = distance;
            *z = -distance + part_offset;
            break;
        case 2:
            // We're at the bottom of the spiral.
            *z = distance;
            *x = distance - part_offset;
            break;
        case 3:
            // We're on the left side of the spiral.
            *x = -distance;
            *z = distance - part_offset;
            break;
        default:
            // Internal error!
            assert(0);
    }
}

#endif  // SPIRAL_ITERATOR_H_
