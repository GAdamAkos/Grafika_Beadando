#ifndef COLLISION_H
#define COLLISION_H

#include <stdbool.h>

typedef struct AABB {
    float min_x, min_y, min_z;
    float max_x, max_y, max_z;
} AABB;

static inline bool aabb_intersects(const AABB* a, const AABB* b) {
    return (a->min_x <= b->max_x && a->max_x >= b->min_x) &&
           (a->min_y <= b->max_y && a->max_y >= b->min_y) &&
           (a->min_z <= b->max_z && a->max_z >= b->min_z);
}

#endif